// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DungeonCatCharacter.h"
#include "AbilitySystemInterface.h"
#include "DCPlayerCharacter.generated.h"

class ADCPlayerState;
class UDCAttributeSet;
class UAbilitySystemComponent;
class UDCGameplayAbility_TestDamage;

/**
 *  The real playable character (SystemsDesign.md's DC-prefixed classes are the actual game, as
 *  opposed to the stock-template ADungeonCatCharacter scaffold this extends purely for its
 *  camera-boom setup, per SystemsDesign.md §9). Non-abstract on purpose, unlike its parent - P1 is
 *  grey-box only (ProductionPlan.md), no mesh/AnimBP needed yet to prove replication, so this doesn't
 *  require a Blueprint child just to be spawnable. Reconsider going back to UCLASS(abstract) once a
 *  real mesh/AnimBP is set on a BP child and this base is no longer meant to be spawned directly.
 *
 *  Does NOT own the AbilitySystemComponent - that lives on ADCPlayerState (SystemsDesign.md §2.2).
 *  GetAbilitySystemComponent() forwards there. PossessedBy (server-side) and OnRep_PlayerState
 *  (client-side) both call InitAbilityActorInfo - this is the standard GAS fix for the well-known
 *  "PlayerState isn't valid/replicated yet when the character's own BeginPlay runs" race. Skipping
 *  either one is the single most common cause of an ability silently failing to activate on one side
 *  of a client/server pair, which is exactly the kind of bug the week-1 replication proof exists to
 *  catch early (ProductionPlan.md P1).
 */
UCLASS()
class DUNGEONCAT_API ADCPlayerCharacter : public ADungeonCatCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ADCPlayerCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UFUNCTION(BlueprintPure, Category = "DC|Ability")
	ADCPlayerState* GetDCPlayerState() const;

	UFUNCTION(BlueprintPure, Category = "DC|Ability")
	UDCAttributeSet* GetDCAttributeSet() const;

	/** Week-1 replication-proof trigger only (ProductionPlan.md P1) - type
	 *  "DC_TestAbility_DealDamageToSelf" into the PIE console on either client rather than binding an
	 *  Input Action, since no unreal-mcp toolset can create Input Action/IMC assets (confirmed in
	 *  zombieshooter's own MCP-tooling notes) and no editor session is available to make one by hand
	 *  this pass. Delete once the real abilities have real Enhanced Input bindings. */
	UFUNCTION(Exec, Category = "DC|Debug")
	void DC_TestAbility_DealDamageToSelf();

protected:

	/** Grants the one-off test ability server-side once the ASC is valid - called from both PossessedBy and (harmlessly, idempotently) from OnRep_PlayerState's InitAbilityActorInfo call site. */
	void GrantTestAbility();

	UPROPERTY(EditDefaultsOnly, Category = "DC|Debug")
	TSubclassOf<UDCGameplayAbility_TestDamage> TestAbilityClass;
};
