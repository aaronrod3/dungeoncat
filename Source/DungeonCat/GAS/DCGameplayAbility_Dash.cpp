// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCGameplayAbility_Dash.h"
#include "DCPlayerCharacter.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"

UDCGameplayAbility_Dash::UDCGameplayAbility_Dash()
{
	static const FGameplayTag Tag_Ability_Dash = UGameplayTagsManager::Get().RequestGameplayTag(FName("Ability.Knight.Dash"));
	SetAssetTags(FGameplayTagContainer(Tag_Ability_Dash));

	CooldownDurationSeconds = 3.0f;

	static const FGameplayTag Tag_Ability_BasicAttack = UGameplayTagsManager::Get().RequestGameplayTag(FName("Ability.Knight.BasicAttack"));
	CancelAbilitiesWithTag.AddTag(Tag_Ability_BasicAttack);
}

void UDCGameplayAbility_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthority(&ActivationInfo) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	StartCooldown();

	static const FGameplayTag Tag_State_Invulnerable = UGameplayTagsManager::Get().RequestGameplayTag(FName("State.Invulnerable"));
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(Tag_State_Invulnerable);
	}

	if (ADCPlayerCharacter* Character = GetDCPlayerCharacter())
	{
		Character->LaunchCharacter(Character->GetActorForwardVector() * LaunchSpeed, true, false);
	}

	UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, DashDurationSeconds);
	DelayTask->OnFinish.AddDynamic(this, &UDCGameplayAbility_Dash::HandleDashFinished);
	DelayTask->ReadyForActivation();
}

void UDCGameplayAbility_Dash::HandleDashFinished()
{
	static const FGameplayTag Tag_State_Invulnerable = UGameplayTagsManager::Get().RequestGameplayTag(FName("State.Invulnerable"));
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(Tag_State_Invulnerable);
	}

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
