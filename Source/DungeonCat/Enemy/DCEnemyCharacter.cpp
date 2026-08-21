// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCEnemyCharacter.h"
#include "DCEnemyConfig.h"
#include "GAS/DCAbilitySystemComponent.h"
#include "GAS/DCAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

ADCEnemyCharacter::ADCEnemyCharacter()
{
	AbilitySystemComponent = CreateDefaultSubobject<UDCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed, matching ADCPlayerState's reasoning (SystemsDesign.md §7/§8): co-op players need to see
	// an enemy's Health for shared telegraph/threat readability, not just the server.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UDCAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ADCEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ADCEnemyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (AbilitySystemComponent)
	{
		// No PlayerState indirection here (unlike ADCPlayerCharacter) - owner and avatar are the same
		// actor, so there's no OnRep_PlayerState race to guard against.
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	if (AttributeSet)
	{
		AttributeSet->OnOutOfHealth.AddDynamic(this, &ADCEnemyCharacter::HandleOutOfHealth);
	}

	if (HasAuthority())
	{
		InitializeAttributesFromConfig();
	}

	if (Config)
	{
		// Base movement speed, set independently on every machine from the same static config data -
		// not a replicated value, so no authority gate needed. Left as a plain MaxWalkSpeed set rather
		// than routed through AttributeSet::MoveSpeed (that's a multiplier for buffs/slows on top of a
		// base speed, per its own header comment - this IS the base speed for a given archetype).
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->MaxWalkSpeed = Config->MoveSpeed;
		}
	}
}

void ADCEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADCEnemyCharacter, bIsDead);
}

void ADCEnemyCharacter::InitializeAttributesFromConfig()
{
	if (!Config)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no DCEnemyConfig assigned - spawning with AttributeSet defaults, not archetype stats. Assign one on the Blueprint child."), *GetNameSafe(this));
		return;
	}

	AttributeSet->InitMaxHealth(Config->MaxHealth);
	AttributeSet->InitHealth(Config->MaxHealth);
}

void ADCEnemyCharacter::HandleOutOfHealth()
{
	// OnOutOfHealth only ever broadcasts from the server-only PostGameplayEffectExecute path
	// (DCAttributeSet.h's own comment) - this guard is defensive, not load-bearing.
	if (!HasAuthority())
	{
		return;
	}

	Die();
}

void ADCEnemyCharacter::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	SetActorEnableCollision(false);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
	}

	// Despawn timing / loot-drop-on-death is deliberately not wired here - stays decoupled from the
	// itemization system (Docs/Items.md) until that integration is actually designed, rather than
	// guessing a lifespan now that later work would have to work around.
	OnEnemyDeath.Broadcast(this);
}

void ADCEnemyCharacter::OnRep_IsDead()
{
	// Mirrors Die()'s server-side collision disable for clients - bIsDead itself replicates, but
	// SetActorEnableCollision doesn't automatically follow a replicated bool, so each client applies
	// the same visible effect locally on the rep event instead of a second replicated flag.
	if (bIsDead)
	{
		SetActorEnableCollision(false);
	}
}
