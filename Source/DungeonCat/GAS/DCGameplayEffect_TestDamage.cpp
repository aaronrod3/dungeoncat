// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCGameplayEffect_TestDamage.h"
#include "DCDamageExecCalculation.h"

UDCGameplayEffect_TestDamage::UDCGameplayEffect_TestDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition ExecutionDef;
	ExecutionDef.CalculationClass = UDCDamageExecCalculation::StaticClass();
	Executions.Add(ExecutionDef);
}
