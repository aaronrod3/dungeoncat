// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DCGameplayEffect_AttackMoveSlow.generated.h"

/**
 *  Applied to self by every attack ability for its active-swing duration (P1 design decision,
 *  ProductionPlan.md: attacks slow the player, they don't root - a real movement penalty via a
 *  MoveSpeed modifier, not a hard movement block/tag-based input gate). HasDuration, grants
 *  "State.Attacking" and multiplies MoveSpeed, then auto-expires.
 */
UCLASS()
class DUNGEONCAT_API UDCGameplayEffect_AttackMoveSlow : public UGameplayEffect
{
	GENERATED_BODY()

public:

	UDCGameplayEffect_AttackMoveSlow();
};
