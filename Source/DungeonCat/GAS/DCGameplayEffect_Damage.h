// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DCGameplayEffect_Damage.generated.h"

/**
 *  The one real damage-dealing GameplayEffect (SystemsDesign.md §2.4's single entry point) - Instant,
 *  routes through UDCDamageExecCalculation. Every ability that deals damage reuses this exact class,
 *  varying only the "Data.Damage" SetByCaller magnitude they set on the outgoing spec (see
 *  UDCGameplayAbility::ApplyDamageToTargets) - there is deliberately no per-ability damage GE
 *  subclass to keep in sync.
 */
UCLASS()
class DUNGEONCAT_API UDCGameplayEffect_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:

	UDCGameplayEffect_Damage();
};
