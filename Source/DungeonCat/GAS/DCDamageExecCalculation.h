// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DCDamageExecCalculation.generated.h"

/**
 *  The single entry point for all damage in the game (SystemsDesign.md §2.4). Reads a raw
 *  pre-mitigation damage magnitude off the triggering GameplayEffectSpec via the "Data.Damage"
 *  SetByCaller tag, rolls the source's CritChance/CritMultiplier, applies the target's Armor as a
 *  flat reduction (GameDevPlan.md §2.1's "flat-then-scale" beta decision), and writes the result to
 *  the target's Damage meta attribute - UDCAttributeSet::PostGameplayEffectExecute converts that into
 *  the real Health change. Nothing else should ever apply damage outside this class.
 */
UCLASS()
class DUNGEONCAT_API UDCDamageExecCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	UDCDamageExecCalculation();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
