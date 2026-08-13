// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCGameplayEffect_Cost_Headbutt.h"
#include "DCAttributeSet.h"

UDCGameplayEffect_Cost_Headbutt::UDCGameplayEffect_Cost_Headbutt()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo StaminaCostModifier;
	StaminaCostModifier.Attribute = UDCAttributeSet::GetStaminaAttribute();
	StaminaCostModifier.ModifierOp = EGameplayModOp::Additive;
	StaminaCostModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-15.0f));
	Modifiers.Add(StaminaCostModifier);
}
