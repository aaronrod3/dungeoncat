// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DCPlayerState.h"
#include "DCSaveGame_Profile.generated.h"

/**
 *  Profile save (SystemsDesign.md §7) - persistent across runs, separate from ephemeral run state
 *  (current dungeon/loot-carried-this-run, which has no save representation at all per the beta's
 *  deliberate no-mid-run-save-resume decision). Cosmetic unlocks, meta-currency, class/spec unlocks,
 *  skill levels - SaveVersion from day one even though the beta needs no migration logic yet, per
 *  §7's explicit "cheap now, expensive to retrofit later" reasoning.
 *
 *  Autosave trigger (Run-End screen only, per §7) and the actual SaveGameToSlot/LoadGameFromSlot call
 *  sites aren't wired here - this is the data shape only, matching how UDCItemConfig/UDCEnemyConfig
 *  are data shapes their own systems don't wire end-to-end yet either.
 */
UCLASS(BlueprintType)
class DUNGEONCAT_API UDCSaveGame_Profile : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DC|Save")
	int32 SaveVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DC|Save")
	TArray<FDCSkillLevelEntry> SkillLevels;

	/** No defined spend/earn mechanism yet - Docs/IdeaBacklog.md's "character leveling grants tokens"
	 *  fork is one candidate, not decided. Field exists so the save shape doesn't need a later schema
	 *  change once that's resolved, same reasoning §7 already applied to CritChance/CritMultiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DC|Save")
	int32 MetaCurrency = 0;

	/** Cosmetic unlock IDs (Docs/Items.md's fur colors/patterns/accessories) - plain FName tags for
	 *  now rather than a richer struct, since the actual cosmetic catalog is still first-draft content. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DC|Save")
	TArray<FName> UnlockedCosmetics;

	/** Class/spec unlocks (Docs/GameDevPlan.md §6 - post-beta, 4 classes/8 specs). Empty/unused for the
	 *  beta's fixed-Knight scope. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DC|Save")
	TArray<FName> UnlockedClassesAndSpecs;
};
