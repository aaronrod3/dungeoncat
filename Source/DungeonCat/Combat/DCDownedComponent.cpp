// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCDownedComponent.h"
#include "GAS/DCAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UDCDownedComponent::UDCDownedComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UDCDownedComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDCDownedComponent, bIsDowned);
	DOREPLIFETIME(UDCDownedComponent, bIsDead);
}

void UDCDownedComponent::BindToAttributeSet(UDCAttributeSet* AttributeSetIn)
{
	if (!AttributeSetIn || BoundAttributeSet == AttributeSetIn)
	{
		// Already bound to this exact instance - idempotent, matches the PossessedBy/OnRep_PlayerState
		// double-call pattern this is invoked from (ADCPlayerCharacter).
		return;
	}

	BoundAttributeSet = AttributeSetIn;
	BoundAttributeSet->OnOutOfHealth.AddDynamic(this, &UDCDownedComponent::HandleOutOfHealth);
}

void UDCDownedComponent::HandleOutOfHealth()
{
	// OnOutOfHealth only ever broadcasts server-side (DCAttributeSet.h's own comment) - defensive guard.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (bIsDowned)
	{
		// Already downed and hit again while the revive timer was still running - a finishing blow.
		Die();
	}
	else
	{
		Server_EnterDowned();
	}
}

void UDCDownedComponent::Server_EnterDowned()
{
	if (bIsDowned || bIsDead)
	{
		return;
	}

	bIsDowned = true;
	// OnRep never fires on the authoring machine - call it directly here, matching the convention
	// this pattern was ported from (zombieshooter's UZSHealthComponent).
	OnRep_IsDowned();

	GetWorld()->GetTimerManager().SetTimer(DownedTimerHandle, this, &UDCDownedComponent::HandleDownedTimerExpired, DownedDurationSeconds, false);
}

void UDCDownedComponent::Server_ExitDowned()
{
	if (!bIsDowned)
	{
		return;
	}

	bIsDowned = false;
	OnRep_IsDowned();

	GetWorld()->GetTimerManager().ClearTimer(DownedTimerHandle);
}

void UDCDownedComponent::HandleDownedTimerExpired()
{
	// Nobody revived (or self-healed, once that path exists) in time.
	Die();
}

void UDCDownedComponent::Server_ReviveDowned()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !bIsDowned)
	{
		return;
	}

	if (BoundAttributeSet)
	{
		// Never down - a partial self-heal before a teammate arrives isn't overwritten, same
		// reasoning as the ported zombieshooter pattern.
		BoundAttributeSet->SetHealth(FMath::Max(BoundAttributeSet->GetHealth(), ReviveHealthAmount));
	}

	Server_ExitDowned();
}

void UDCDownedComponent::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	if (bIsDowned)
	{
		bIsDowned = false;
		GetWorld()->GetTimerManager().ClearTimer(DownedTimerHandle);
	}

	OnRep_IsDead();
	OnPlayerDeath.Broadcast();
}

void UDCDownedComponent::OnRep_IsDowned()
{
	OnDownedChanged.Broadcast(bIsDowned);

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	static const FGameplayTag Tag_State_Downed = UGameplayTagsManager::Get().RequestGameplayTag(FName("State.Downed"));

	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			if (bIsDowned)
			{
				ASC->AddLooseGameplayTag(Tag_State_Downed);
			}
			else
			{
				ASC->RemoveLooseGameplayTag(Tag_State_Downed);
			}
		}
	}

	// "Crawls slowly" (SystemsDesign.md §8) - capture the pre-downed speed on entry so exit restores
	// the exact original rather than dividing back through the multiplier (avoids float drift / a
	// double-application bug if this ever fired twice for the same transition).
	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			if (bIsDowned)
			{
				PreDownedMaxWalkSpeed = Movement->MaxWalkSpeed;
				Movement->MaxWalkSpeed = PreDownedMaxWalkSpeed * DownedMoveSpeedMultiplier;
			}
			else if (PreDownedMaxWalkSpeed > 0.f)
			{
				Movement->MaxWalkSpeed = PreDownedMaxWalkSpeed;
			}
		}
	}
}

void UDCDownedComponent::OnRep_IsDead()
{
	// Cosmetic/gameplay consequences of death (ragdoll, respawn flow) are P4/GameMode-level work that
	// doesn't exist yet - see this class's own header comment on the scope boundary. Nothing to do
	// here yet beyond what OnPlayerDeath's broadcast (Die(), server-only) already covers.
}
