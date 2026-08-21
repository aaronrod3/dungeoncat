// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "DCEnemyCharacter.generated.h"

class UDCAbilitySystemComponent;
class UDCAttributeSet;
class UDCEnemyConfig;
class ADCEnemyCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDCOnEnemyDeath, ADCEnemyCharacter*, DeadEnemy);

/**
 *  Base class for every enemy archetype (SystemsDesign.md §5's "one base ADCEnemyCharacter ...
 *  parameterized per archetype rather than forked" multi-config rule - a new enemy is a new
 *  DA_DC_EnemyConfig_* instance assigned to Config below, never a new C++ subclass; see
 *  Docs/Bestiary.md for the actual archetype/creature roster this drives). Owns its own
 *  UDCAbilitySystemComponent + UDCAttributeSet directly, unlike ADCPlayerCharacter (which forwards to
 *  its PlayerState) - enemies have no PlayerState to host one on, and don't need the
 *  survive-a-respawn property that's the whole reason the player's ASC lives there. Reuses the same
 *  UDCAttributeSet / UDCDamageExecCalculation pipeline players use (SystemsDesign.md §2.4's single
 *  damage entry point covers both, not a parallel enemy-only damage system) - this is the from-scratch
 *  rebuild the P0 audit called for after finding CombatEnemy's damage/health entirely non-replicated.
 *
 *  Driving AI behavior is out of scope for this class - reuses ACombatAIController as-is
 *  (P0-audit-confirmed) plus a StateTree asset authored in-editor per archetype (Config->BehaviorStateTree).
 *  This class only needs to be a valid, GAS-integrated pawn for that AI to drive; it implements no
 *  behavior of its own.
 */
UCLASS()
class DUNGEONCAT_API ADCEnemyCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ADCEnemyCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "DC|Enemy")
	UDCAttributeSet* GetDCAttributeSet() const { return AttributeSet; }

	UFUNCTION(BlueprintPure, Category = "DC|Enemy")
	bool IsDead() const { return bIsDead; }

	/** The DA_DC_EnemyConfig_* instance driving this enemy's stats - assigned per archetype on a
	 *  Blueprint child, same pattern as ADCPlayerCharacter's ability-class TSubclassOf properties
	 *  being BP-assigned rather than hardcoded in this base. */
	UPROPERTY(EditDefaultsOnly, Category = "DC|Enemy")
	TObjectPtr<UDCEnemyConfig> Config;

	/** Broadcast once, the instant this enemy dies (see Die()) - AI/despawn/loot-drop logic hooks in
	 *  here rather than each reimplementing its own zero-health check. Server-side only, matching
	 *  UDCAttributeSet::OnOutOfHealth's own server-authoritative scope. */
	UPROPERTY(BlueprintAssignable, Category = "DC|Enemy")
	FDCOnEnemyDeath OnEnemyDeath;

protected:

	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server-only: pushes Config's MaxHealth/MoveSpeed into AttributeSet via the DC_ATTRIBUTE_ACCESSORS
	 *  Init* setters (DCAttributeSet.h) - a one-time starting-stat seed, not a GameplayEffect, since
	 *  these are fixed per-archetype baselines rather than a stackable buff/debuff. No-ops (with a
	 *  warning) if Config is unset - a Blueprint child forgetting to assign one is a content bug worth
	 *  surfacing loudly rather than silently spawning a 0-HP enemy. */
	void InitializeAttributesFromConfig();

	/** Bound to AttributeSet::OnOutOfHealth in PostInitializeComponents. Server-only. */
	UFUNCTION()
	void HandleOutOfHealth();

	/** Server-only: sets bIsDead, disables collision/movement, broadcasts OnEnemyDeath. No-op if
	 *  already dead (guards against a repeat OnOutOfHealth broadcast from a second hit landing before
	 *  despawn). */
	void Die();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DC|Enemy")
	TObjectPtr<UDCAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DC|Enemy")
	TObjectPtr<UDCAttributeSet> AttributeSet;

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, VisibleAnywhere, BlueprintReadOnly, Category = "DC|Enemy")
	bool bIsDead = false;

	UFUNCTION()
	void OnRep_IsDead();
};
