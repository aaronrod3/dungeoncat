// Copyright Epic Games, Inc. All Rights Reserved.


#include "EnvQueryContext_Player.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"

void UEnvQueryContext_Player::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	// Every connected player's pawn, not just index 0 - GetPlayerPawn(WorldContext, 0) resolves to the
	// calling process's own local player, which for AI code running on a listen server means the host's
	// pawn only, silently ignoring every remote client (P2_DungeonAI.md Stage 1 finding). GameState's
	// PlayerArray is replicated server-side and covers every connected player, local or remote.
	TArray<AActor*> PlayerPawns;

	const UWorld* World = QueryInstance.Owner.IsValid() ? QueryInstance.Owner->GetWorld() : nullptr;
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;

	if (GameState)
	{
		for (APlayerState* PlayerState : GameState->PlayerArray)
		{
			if (APawn* Pawn = PlayerState ? PlayerState->GetPawn() : nullptr)
			{
				PlayerPawns.Add(Pawn);
			}
		}
	}

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, PlayerPawns);
}
