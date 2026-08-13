// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DCGameplayEffect_TestDamage.generated.h"

/**
 *  Week-1 replication-proof GameplayEffect only (ProductionPlan.md P1). Instant, routes through
 *  UDCDamageExecCalculation like every real damage-dealing effect will. Authored in C++ rather than
 *  as a GameplayEffect Blueprint/data asset purely because no editor session was available to create
 *  one when this was written - the real Knight abilities should get proper GE_DC_* Blueprint assets
 *  once the editor is back in the loop, not more classes like this one.
 */
UCLASS()
class DUNGEONCAT_API UDCGameplayEffect_TestDamage : public UGameplayEffect
{
	GENERATED_BODY()

public:

	UDCGameplayEffect_TestDamage();
};
