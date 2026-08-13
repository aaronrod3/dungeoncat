// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCGameplayEffect_Damage.h"
#include "DCDamageExecCalculation.h"

UDCGameplayEffect_Damage::UDCGameplayEffect_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition ExecutionDef;
	ExecutionDef.CalculationClass = UDCDamageExecCalculation::StaticClass();
	Executions.Add(ExecutionDef);
}
