// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Math/RandomStream.h"
#include "DCItemConfig.h"
#include "DCLootTableConfig.generated.h"

/** One weighted entry in a loot table - the entry's rarity comes from Item->Rarity (UDCItemConfig),
 *  not duplicated here. */
USTRUCT(BlueprintType)
struct FDCLootTableEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DC|Loot")
	TObjectPtr<UDCItemConfig> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DC|Loot", meta = (ClampMin = "0.0"))
	float Weight = 1.f;
};

/**
 *  SystemsDesign.md §6's drop-table technical shape (content/numbers: Docs/Items.md). One instance
 *  per enemy archetype / room type for regular drops (rarity odds default to the beta's
 *  Common 70 / Uncommon 20 / Rare 8 / Epic 2), or one per boss with the Boss-only fields below set
 *  (Common 10 / Uncommon 30 / Rare 40 / Epic 20, multiple rolls) - same class either way, just
 *  different data, matching the project's "content is data, not a new C++ type" convention.
 */
UCLASS(BlueprintType)
class DUNGEONCAT_API UDCLootTableConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Loot")
	TArray<FDCLootTableEntry> Entries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CommonChance = 0.70f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UncommonChance = 0.20f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RareChance = 0.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EpicChance = 0.02f;

	/** Boss tables only (Docs/Items.md, added 2026-08-19) - regular per-enemy/per-room tables leave
	 *  this at 1 (a single roll). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Loot|Boss", meta = (ClampMin = "1"))
	int32 BaseGuaranteedRolls = 1;

	/** Boss tables only - multiplies BaseGuaranteedRolls by the party size passed into RollLoot
	 *  ("guaranteed rolls = party size at the kill," Docs/Items.md). False for regular tables. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Loot|Boss")
	bool bScaleRollsWithPartySize = false;

	/** Boss tables only - flat chance at one extra roll beyond the guaranteed count (Docs/Items.md
	 *  default: 25%). 0 for regular tables. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Loot|Boss", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BonusRollChance = 0.f;

	/** Rolls loot server-side (SystemsDesign.md §6: "rolled server-side only"). PartySize only matters
	 *  when bScaleRollsWithPartySize is true. Stream is caller-owned (not generated internally) so
	 *  rolls stay reproducible from a given seed, matching the project's "seed everything" convention
	 *  (SystemsDesign.md §4/§5.1) rather than each roll being independently non-deterministic. A roll
	 *  landing on a rarity tier with no matching Entries is silently skipped - an author leaving a
	 *  tier's pool empty on this table is a valid "never drops that rarity" choice, not an error. */
	UFUNCTION(BlueprintCallable, Category = "DC|Loot")
	TArray<UDCItemConfig*> RollLoot(int32 PartySize, UPARAM(ref) FRandomStream& Stream) const;

private:

	EDCItemRarity RollRarity(FRandomStream& Stream) const;
	UDCItemConfig* RollItemOfRarity(EDCItemRarity Rarity, FRandomStream& Stream) const;
};
