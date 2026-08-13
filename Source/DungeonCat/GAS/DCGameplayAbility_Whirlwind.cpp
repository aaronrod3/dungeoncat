// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCGameplayAbility_Whirlwind.h"
#include "DCGameplayEffect_AttackMoveSlow.h"
#include "GameplayTagsManager.h"

UDCGameplayAbility_Whirlwind::UDCGameplayAbility_Whirlwind()
{
	static const FGameplayTag Tag_Ability_Whirlwind = UGameplayTagsManager::Get().RequestGameplayTag(FName("Ability.Knight.Whirlwind"));
	SetAssetTags(FGameplayTagContainer(Tag_Ability_Whirlwind));

	CooldownDurationSeconds = 8.0f;
}

void UDCGameplayAbility_Whirlwind::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthority(&ActivationInfo) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	StartCooldown();
	ApplySelfEffect(UDCGameplayEffect_AttackMoveSlow::StaticClass());

	const TArray<FHitResult> Hits = SphereOverlapAroundSelf(Radius);
	ApplyDamageToTargets(Hits, Damage);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
