// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCGameplayEffect_AttackMoveSlow.h"
#include "DCAttributeSet.h"

UDCGameplayEffect_AttackMoveSlow::UDCGameplayEffect_AttackMoveSlow()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.5f));

	FGameplayModifierInfo MoveSpeedModifier;
	MoveSpeedModifier.Attribute = UDCAttributeSet::GetMoveSpeedAttribute();
	MoveSpeedModifier.ModifierOp = EGameplayModOp::MultiplyCompound;
	MoveSpeedModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.4f));
	Modifiers.Add(MoveSpeedModifier);

	// No "State.Attacking" tag grant here - UGameplayEffect::FindOrAddComponent<UTargetTagsGameplayEffectComponent>()
	// crashes when called from a GameplayEffect's own constructor in UE5.8 ("NewObject with empty
	// name can't be used to create default subobjects... Use ObjectInitializer.CreateDefaultSubobject<>
	// instead" - confirmed via a headless smoke-test crash, not a guess). Nothing currently consumes
	// this tag (no enemy AI/ability yet checks it), so it's deferred rather than worked around blind.
	// If a future system needs it, grant it via UAbilitySystemComponent::AddLooseGameplayTag from the
	// ability side instead (see UDCGameplayAbility_Dash's Invulnerable handling for the pattern) -
	// don't retry the GameplayEffectComponent route from a constructor.
}
