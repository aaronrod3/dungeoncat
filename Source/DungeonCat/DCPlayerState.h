// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "DCPlayerState.generated.h"

class UDCAbilitySystemComponent;
class UDCAttributeSet;

/** One skill's tracked level (Docs/Classes.md's "Skill & progression system" - Nimble Paws, Keen
 *  Senses, etc.). A replicated TArray of these instead of the TMap<FGameplayTag,int32> SystemsDesign.md
 *  §6 originally described in prose - classic property replication doesn't support TMap directly, and
 *  a plain-struct TArray replicates as a whole-array diff, which is fine for data that changes this
 *  infrequently (a skill level-up, not a per-tick value). Functionally equivalent, just replication-safe. */
USTRUCT(BlueprintType)
struct FDCSkillLevelEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DC|Skill")
	FGameplayTag Skill;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DC|Skill", meta = (ClampMin = "0"))
	int32 Level = 0;
};

/**
 *  Hosts the ability system for every DungeonCat player (SystemsDesign.md §2.2) - lives on
 *  PlayerState, not the character, so the ASC survives character destroy/respawn (co-op revive,
 *  death/respawn per SystemsDesign.md §7) without an ASC re-init + ability re-grant dance.
 *  ADCPlayerCharacter forwards IAbilitySystemInterface here rather than owning its own ASC.
 *
 *  Also hosts skill-level tracking (Docs/Classes.md/Docs/Items.md) - profile-scope progression, same
 *  reasoning as the ASC: PlayerState is what survives a character respawn, and skill levels are
 *  exactly the kind of persistent-across-runs data SystemsDesign.md §7's profile save scope covers
 *  (UDCSaveGame_Profile is where this gets written to/read from between sessions, not built here).
 */
UCLASS()
class DUNGEONCAT_API ADCPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ADCPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "DC|Ability")
	UDCAttributeSet* GetDCAttributeSet() const { return AttributeSet; }

	/** 0 if Skill has never been leveled - a missing entry means "level 0," not an error. */
	UFUNCTION(BlueprintPure, Category = "DC|Skill")
	int32 GetSkillLevel(FGameplayTag Skill) const;

	/** Server-only. Only ever raises a skill's level (matches "learn by doing + milestone" -
	 *  Docs/Classes.md never describes a skill regressing) - a NewLevel at or below the current one is
	 *  a silent no-op, not an error, so a caller doesn't need to check-before-set itself. */
	UFUNCTION(BlueprintCallable, Category = "DC|Skill")
	void Server_SetSkillLevel(FGameplayTag Skill, int32 NewLevel);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DC|Ability")
	TObjectPtr<UDCAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DC|Ability")
	TObjectPtr<UDCAttributeSet> AttributeSet;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "DC|Skill")
	TArray<FDCSkillLevelEntry> SkillLevels;
};
