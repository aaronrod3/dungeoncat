// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DCItemConfig.generated.h"

class UGameplayEffect;
class UTexture2D;
class UStaticMesh;

/** Docs/Items.md's beta rarity tiers - Common/Uncommon/Rare/Epic, no fifth tier planned. */
UENUM(BlueprintType)
enum class EDCItemRarity : uint8
{
	Common,
	Uncommon,
	Rare,
	Epic
};

/** The 6-affix pool (Docs/Items.md, added 2026-08-19) - matches UDCAttributeSet's fields 1:1, no
 *  stat invented just for itemization. */
UENUM(BlueprintType)
enum class EDCItemAffixType : uint8
{
	MaxHealth,
	MaxStamina,
	Armor,
	MoveSpeed,
	CritChance,
	CritMultiplier
};

/** One rolled affix on an item instance - SystemsDesign.md §6's technical shape for Items.md's affix
 *  system. Applied as a GameplayEffect modifier at grant time (UDCItemPickupComponent), the same
 *  mechanism UDCItemConfig::GrantedEffects already uses for fixed stat modifiers, just parameterized
 *  instead of fixed per item. */
USTRUCT(BlueprintType)
struct FDCItemAffix
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DC|Item")
	EDCItemAffixType AffixType = EDCItemAffixType::MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DC|Item")
	float RolledValue = 0.f;
};

/**
 *  Ported shape (not class) from zombieshooter's UZSItemConfig, deliberately much smaller - Docs/Items.md's
 *  beta scope is explicit that there's no equip/inventory UI yet ("stat trinkets auto-apply their
 *  GameplayEffect on pickup... consumables are usable directly from a simple carried-list"), so this
 *  doesn't carry ZS's weight/stacking/equip-slot machinery. SystemsDesign.md §6 is the technical spec
 *  this implements; the actual item catalog content lives in Docs/Items.md.
 */
UCLASS(BlueprintType)
class DUNGEONCAT_API UDCItemConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Item")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Item")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Item")
	EDCItemRarity Rarity = EDCItemRarity::Common;

	/** Fixed stat-modifier effects granted on pickup (SystemsDesign.md §6) - for a unique
	 *  (bIsUnique == true) this is the bespoke hardcoded effect; for a rolled item it's typically
	 *  empty, with Affixes below doing the work instead. Both can be populated at once (nothing stops
	 *  a rolled item from also carrying one fixed effect) - not mutually exclusive by construction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Item")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

	/** Rolled affixes (Docs/Items.md) - count-by-rarity (0/1/2/3 for Common/Uncommon/Rare/Epic) is a
	 *  design/drop-table concern, not enforced here; this just holds whatever was actually rolled. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Item")
	TArray<FDCItemAffix> Affixes;

	/** Docs/Items.md's uniques - a fixed, non-random effect (GrantedEffects) instead of rolled
	 *  Affixes. Whether to skip affix-rolling for a unique is the roll-time caller's decision (this
	 *  flag just records intent); nothing here enforces Affixes staying empty when true. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Item")
	bool bIsUnique = false;

	/** Docs/Items.md's beta consumable ("a heal-equivalent, usable directly from a simple carried-list")
	 *  - the carried-list itself isn't built yet (flagged as a gap, not silently assumed), but the data
	 *  shape is ready for it. 0 = not a consumable / no effect, the default for a stat trinket. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Item", meta = (ClampMin = "0.0"))
	float HealthRestoreAmount = 0.f;

	/** World-space pickup representation, graceful-if-unset (an invisible pickup, not a crash) - same
	 *  "content not sourced yet" tolerance as zombieshooter's equivalent field. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Item")
	TObjectPtr<UStaticMesh> WorldMesh;
};
