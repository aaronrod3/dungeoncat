// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DCGameplayEffect_Cost_Headbutt.generated.h"

/** Headbutt's Stamina cost (SystemsDesign.md §2.3) - assigned to UDCGameplayAbility_ShieldBash::CostGameplayEffectClass. Instant, subtracts a flat Stamina amount. */
UCLASS()
class DUNGEONCAT_API UDCGameplayEffect_Cost_Headbutt : public UGameplayEffect
{
	GENERATED_BODY()

public:

	UDCGameplayEffect_Cost_Headbutt();
};
