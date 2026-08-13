// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCGameplayAbility_ShieldBash.h"
#include "DCPlayerCharacter.h"
#include "DCGameplayEffect_AttackMoveSlow.h"
#include "DCGameplayEffect_Cost_Headbutt.h"
#include "GameplayTagsManager.h"

UDCGameplayAbility_ShieldBash::UDCGameplayAbility_ShieldBash()
{
	static const FGameplayTag Tag_Ability_ShieldBash = UGameplayTagsManager::Get().RequestGameplayTag(FName("Ability.Knight.ShieldBash"));
	SetAssetTags(FGameplayTagContainer(Tag_Ability_ShieldBash));

	CostGameplayEffectClass = UDCGameplayEffect_Cost_Headbutt::StaticClass();
	CooldownDurationSeconds = 4.0f;
}

void UDCGameplayAbility_ShieldBash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthority(&ActivationInfo) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	StartCooldown();
	ApplySelfEffect(UDCGameplayEffect_AttackMoveSlow::StaticClass());

	if (ADCPlayerCharacter* Character = GetDCPlayerCharacter())
	{
		// Shield leads the charge - off-hand socket, not the weapon hand.
		Character->LaunchCharacter(Character->GetActorForwardVector() * LaunchSpeed, true, false);
	}

	const TArray<FHitResult> Hits = SphereTraceFromSocket(FName("Socket.WeaponHandL"), TraceDistance, TraceRadius);
	ApplyDamageToTargets(Hits, Damage);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
