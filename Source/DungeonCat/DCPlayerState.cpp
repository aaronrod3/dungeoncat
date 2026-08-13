// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCPlayerState.h"
#include "GAS/DCAbilitySystemComponent.h"
#include "GAS/DCAttributeSet.h"

ADCPlayerState::ADCPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UDCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed: attributes replicate to everyone (co-op teammates need to see each other's Health for a
	// shared HUD/revive prompts, SystemsDesign.md §7/§8) while gameplay effects/cues stay owner-only.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UDCAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ADCPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
