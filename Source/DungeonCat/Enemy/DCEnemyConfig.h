// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DCEnemyConfig.generated.h"

class UStateTree;
class USkeletalMesh;

/**
 *  The "multi-config rule" data asset (SystemsDesign.md §5): a new enemy is a new
 *  DA_DC_EnemyConfig_* instance assigned to ADCEnemyCharacter::Config, never a new C++ subclass.
 *  Fields cover the shared shape across all 4 archetypes from Docs/Bestiary.md - archetype-specific
 *  behavior (retreat/approach bands for the ranged spitter, telegraph timing for Brute/Tank, etc.)
 *  is expressed as optional fields left at their neutral default (0) for archetypes that don't use
 *  them, per SystemsDesign.md §5.1's per-archetype tuning tables, rather than a field per archetype
 *  living on a forked subclass.
 */
UCLASS(BlueprintType)
class DUNGEONCAT_API UDCEnemyConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Enemy")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Enemy", meta = (ClampMin = "1.0"))
	float MaxHealth = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Enemy", meta = (ClampMin = "0.0"))
	float DamagePerHit = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Enemy", meta = (ClampMin = "0.0"))
	float MoveSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Enemy", meta = (ClampMin = "0.0"))
	float AttackRange = 150.f;

	/** Brute/Tank-style long wind-up before an attack lands (Docs/Bestiary.md) - 0 (the default) means
	 *  no telegraph delay, correct for the melee chaser/swarm archetypes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Enemy", meta = (ClampMin = "0.0"))
	float AttackTelegraphSeconds = 0.f;

	/** Ranged spitter's retreat-if-closer-than threshold (SystemsDesign.md §5.1) - 0 means unused
	 *  (this archetype doesn't maintain a preferred range band). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Enemy", meta = (ClampMin = "0.0"))
	float PreferredRangeMin = 0.f;

	/** Ranged spitter's approach-if-farther-than threshold (SystemsDesign.md §5.1) - 0 means unused. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Enemy", meta = (ClampMin = "0.0"))
	float PreferredRangeMax = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Enemy")
	TSoftObjectPtr<USkeletalMesh> Mesh;

	/** The StateTree asset (Content/, editor-authored) this archetype's AIController runs - assigned
	 *  per instance, not hardcoded, so a new archetype/reskin never needs a C++ change. Unset is a
	 *  valid, non-fatal state for grey-box testing (an enemy with no behavior yet, not a crash). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DC|Enemy")
	TObjectPtr<UStateTree> BehaviorStateTree;
};
