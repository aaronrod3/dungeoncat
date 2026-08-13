// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCDamageExecCalculation.h"
#include "DCAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"

struct FDCDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritMultiplier);

	FDCDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDCAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDCAttributeSet, CritChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDCAttributeSet, CritMultiplier, Source, false);
	}
};

static const FDCDamageStatics& DCDamageStatics()
{
	static FDCDamageStatics Statics;
	return Statics;
}

UDCDamageExecCalculation::UDCDamageExecCalculation()
{
	RelevantAttributesToCapture.Add(DCDamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DCDamageStatics().CritChanceDef);
	RelevantAttributesToCapture.Add(DCDamageStatics().CritMultiplierDef);
}

void UDCDamageExecCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float Armor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DCDamageStatics().ArmorDef, EvaluationParameters, Armor);

	float CritChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DCDamageStatics().CritChanceDef, EvaluationParameters, CritChance);

	float CritMultiplier = 1.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DCDamageStatics().CritMultiplierDef, EvaluationParameters, CritMultiplier);

	// Raw pre-mitigation magnitude comes from the activating ability via SetSetByCallerMagnitude(Data.Damage, ...).
	// 0 if the caller forgot to set it - not an error, just deals no damage.
	static const FGameplayTag Tag_Data_Damage = UGameplayTagsManager::Get().RequestGameplayTag(FName("Data.Damage"));
	const float BaseDamage = Spec.GetSetByCallerMagnitude(Tag_Data_Damage, false, 0.f);

	// CritChance is 0 by default for the beta (SystemsDesign.md §10 open items) - harmless no-op math
	// at CritChance == 0, so this is already correct once a later itemization pass makes it live.
	const bool bIsCrit = FMath::FRand() < FMath::Clamp(CritChance, 0.f, 1.f);
	const float PostCritDamage = bIsCrit ? BaseDamage * CritMultiplier : BaseDamage;

	// Flat armor reduction (GameDevPlan.md §2.1 beta decision), floored at 0 - armor can't heal.
	const float FinalDamage = FMath::Max(PostCritDamage - Armor, 0.f);

	if (FinalDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UDCAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, FinalDamage));
	}
}
