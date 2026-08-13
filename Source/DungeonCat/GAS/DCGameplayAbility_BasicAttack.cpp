// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCGameplayAbility_BasicAttack.h"
#include "DCGameplayEffect_AttackMoveSlow.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameplayTagsManager.h"

UDCGameplayAbility_BasicAttack::UDCGameplayAbility_BasicAttack()
{
	static const FGameplayTag Tag_Ability_BasicAttack = UGameplayTagsManager::Get().RequestGameplayTag(FName("Ability.Knight.BasicAttack"));
	SetAssetTags(FGameplayTagContainer(Tag_Ability_BasicAttack));
}

void UDCGameplayAbility_BasicAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bIsCharging = false;

	UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);
	ReleaseTask->OnRelease.AddDynamic(this, &UDCGameplayAbility_BasicAttack::HandleInputReleased);
	ReleaseTask->ReadyForActivation();

	UAbilityTask_WaitDelay* ThresholdTask = UAbilityTask_WaitDelay::WaitDelay(this, HoldThresholdSeconds);
	ThresholdTask->OnFinish.AddDynamic(this, &UDCGameplayAbility_BasicAttack::HandleHoldThresholdReached);
	ThresholdTask->ReadyForActivation();
}

void UDCGameplayAbility_BasicAttack::HandleHoldThresholdReached()
{
	// Still holding past the threshold (if input had already been released, HandleInputReleased
	// already ended the ability and this sibling task would have been cancelled with it - standard
	// GAS task-lifecycle behavior, not something this needs to guard against separately).
	bIsCharging = true;

	// Grey-box: no charge-loop visual yet. Once a real Pounce charge-loop montage exists
	// (AssetPipeline.md §4), play/loop it here until release.
}

void UDCGameplayAbility_BasicAttack::HandleInputReleased(float TimeHeld)
{
	if (bIsCharging)
	{
		PerformPounce();
	}
	else
	{
		PerformClawFlurryHit();
	}

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UDCGameplayAbility_BasicAttack::PerformClawFlurryHit()
{
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastHitTime > ComboWindowSeconds)
	{
		ComboIndex = 0;
	}
	LastHitTime = Now;

	ApplySelfEffect(UDCGameplayEffect_AttackMoveSlow::StaticClass());

	const int32 SafeIndex = ClawFlurryHitDamage.IsValidIndex(ComboIndex) ? ComboIndex : 0;
	const TArray<FHitResult> Hits = SphereTraceFromSocket(FName("Socket.WeaponHandR"), ClawFlurryTraceDistance, ClawFlurryTraceRadius);
	ApplyDamageToTargets(Hits, ClawFlurryHitDamage[SafeIndex]);

	ComboIndex = (ComboIndex + 1) % FMath::Max(ClawFlurryHitDamage.Num(), 1);
}

void UDCGameplayAbility_BasicAttack::PerformPounce()
{
	ApplySelfEffect(UDCGameplayEffect_AttackMoveSlow::StaticClass());

	const TArray<FHitResult> Hits = SphereTraceFromSocket(FName("Socket.WeaponHandR"), PounceTraceDistance, PounceTraceRadius);
	ApplyDamageToTargets(Hits, PounceDamage);

	// A heavy hit isn't part of the tap-chain - always resets the combo.
	ComboIndex = 0;
}
