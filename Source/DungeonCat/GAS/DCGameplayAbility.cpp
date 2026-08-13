// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCGameplayAbility.h"
#include "DCPlayerCharacter.h"
#include "DCAttributeSet.h"
#include "DCGameplayEffect_Damage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayTagsManager.h"

UDCGameplayAbility::UDCGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

bool UDCGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (CooldownDurationSeconds > 0.f && GetWorld() && GetWorld()->GetTimeSeconds() < CooldownEndTimeSeconds)
	{
		return false;
	}
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

float UDCGameplayAbility::GetDCCooldownTimeRemaining() const
{
	if (!GetWorld())
	{
		return 0.f;
	}
	return FMath::Max(CooldownEndTimeSeconds - GetWorld()->GetTimeSeconds(), 0.f);
}

void UDCGameplayAbility::StartCooldown()
{
	if (GetWorld())
	{
		CooldownEndTimeSeconds = GetWorld()->GetTimeSeconds() + CooldownDurationSeconds;
	}
}

ADCPlayerCharacter* UDCGameplayAbility::GetDCPlayerCharacter() const
{
	return Cast<ADCPlayerCharacter>(GetAvatarActorFromActorInfo());
}

const UDCAttributeSet* UDCGameplayAbility::GetDCAttributeSet() const
{
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		return ASC->GetSet<UDCAttributeSet>();
	}
	return nullptr;
}

TArray<FHitResult> UDCGameplayAbility::SphereTraceFromSocket(FName SocketName, float TraceDistance, float TraceRadius) const
{
	TArray<FHitResult> Hits;

	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	ADCPlayerCharacter* Character = GetDCPlayerCharacter();
	if (!Character || !HasAuthority(&ActivationInfo))
	{
		return Hits;
	}

	// Falls back to the actor's own transform if SocketName doesn't exist yet (no skeleton authored) -
	// GetSocketLocation on a missing socket returns the component transform, non-fatal.
	const FVector TraceStart = Character->GetMesh() ? Character->GetMesh()->GetSocketLocation(SocketName) : Character->GetActorLocation();
	const FVector TraceEnd = TraceStart + (Character->GetActorForwardVector() * TraceDistance);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionShape Shape;
	Shape.SetSphere(TraceRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	GetWorld()->SweepMultiByObjectType(Hits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, Shape, QueryParams);
	return Hits;
}

TArray<FHitResult> UDCGameplayAbility::SphereOverlapAroundSelf(float Radius) const
{
	TArray<FHitResult> Hits;

	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	ADCPlayerCharacter* Character = GetDCPlayerCharacter();
	if (!Character || !HasAuthority(&ActivationInfo))
	{
		return Hits;
	}

	// Zero-length sweep at the caster's own location - functionally an overlap check, but keeps the
	// same FHitResult return type as SphereTraceFromSocket so callers (ApplyDamageToTargets/
	// ApplyEffectToTargets) don't need two separate hit-result types.
	const FVector Center = Character->GetActorLocation();

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionShape Shape;
	Shape.SetSphere(Radius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	GetWorld()->SweepMultiByObjectType(Hits, Center, Center, FQuat::Identity, ObjectParams, Shape, QueryParams);
	return Hits;
}

TArray<AActor*> UDCGameplayAbility::GetUniqueGASTargets(const TArray<FHitResult>& Hits)
{
	TArray<AActor*> Targets;
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<UAbilitySystemInterface>())
		{
			Targets.AddUnique(HitActor);
		}
	}
	return Targets;
}

void UDCGameplayAbility::ApplyDamageToTargets(const TArray<FHitResult>& Hits, float DamageAmount) const
{
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC || !HasAuthority(&ActivationInfo) || DamageAmount <= 0.f)
	{
		return;
	}

	static const FGameplayTag Tag_Data_Damage = UGameplayTagsManager::Get().RequestGameplayTag(FName("Data.Damage"));

	for (AActor* Target : GetUniqueGASTargets(Hits))
	{
		IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(Target);
		UAbilitySystemComponent* TargetASC = TargetInterface ? TargetInterface->GetAbilitySystemComponent() : nullptr;
		if (!TargetASC)
		{
			continue;
		}

		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
		ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(UDCGameplayEffect_Damage::StaticClass(), GetAbilityLevel(), ContextHandle);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(Tag_Data_Damage, DamageAmount);
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
	}
}

void UDCGameplayAbility::ApplyEffectToTargets(const TArray<FHitResult>& Hits, TSubclassOf<UGameplayEffect> EffectClass) const
{
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC || !HasAuthority(&ActivationInfo) || !EffectClass)
	{
		return;
	}

	for (AActor* Target : GetUniqueGASTargets(Hits))
	{
		IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(Target);
		UAbilitySystemComponent* TargetASC = TargetInterface ? TargetInterface->GetAbilitySystemComponent() : nullptr;
		if (!TargetASC)
		{
			continue;
		}

		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
		ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), ContextHandle);
		if (SpecHandle.IsValid())
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
	}
}

void UDCGameplayAbility::ApplySelfEffect(TSubclassOf<UGameplayEffect> EffectClass) const
{
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !HasAuthority(&ActivationInfo) || !EffectClass)
	{
		return;
	}

	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), ContextHandle);
	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
