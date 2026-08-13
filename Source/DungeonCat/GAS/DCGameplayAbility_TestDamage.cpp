// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCGameplayAbility_TestDamage.h"
#include "DCGameplayEffect_TestDamage.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"

UDCGameplayAbility_TestDamage::UDCGameplayAbility_TestDamage()
{
	TestDamageEffectClass = UDCGameplayEffect_TestDamage::StaticClass();
}

void UDCGameplayAbility_TestDamage::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthority(&ActivationInfo) || !TestDamageEffectClass || !ActorInfo)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(TestDamageEffectClass, GetAbilityLevel(), ContextHandle);
	if (SpecHandle.IsValid())
	{
		static const FGameplayTag Tag_Data_Damage = UGameplayTagsManager::Get().RequestGameplayTag(FName("Data.Damage"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag_Data_Damage, TestDamageAmount);

		// Self-targeted on purpose - this is a replication proof, not a real attack, so it doesn't need
		// a target actor/trace at all.
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
