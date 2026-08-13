// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCGameplayAbility.h"
#include "DCPlayerCharacter.h"
#include "DCAttributeSet.h"
#include "AbilitySystemComponent.h"

UDCGameplayAbility::UDCGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
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
