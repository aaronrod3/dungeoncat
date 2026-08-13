// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DungeonCatGameMode.h"
#include "DCGameMode.generated.h"

/**
 *  Concrete GameMode for actually testing the real game in PIE (ProductionPlan.md P1) - sets
 *  DefaultPawnClass/PlayerStateClass to the DC-prefixed classes. Non-abstract, unlike its parent,
 *  specifically so it can be picked directly in Project Settings > Maps & Modes without needing a
 *  Blueprint child (no editor session available to make one this pass).
 */
UCLASS()
class DUNGEONCAT_API ADCGameMode : public ADungeonCatGameMode
{
	GENERATED_BODY()

public:

	ADCGameMode();
};
