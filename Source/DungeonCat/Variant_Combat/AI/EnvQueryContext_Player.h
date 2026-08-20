// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_Player.generated.h"

/**
 *  UEnvQueryContext_Player
 *  EnvQuery Context that returns every connected player's pawn (via GameState->PlayerArray, not just
 *  the querying process's own local player - see the .cpp for why the naive local-player-index approach
 *  breaks multiplayer, P2_DungeonAI.md Stage 1).
 */
UCLASS()
class UEnvQueryContext_Player : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:

	/** Provides the context locations or actors for this EnvQuery */
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
