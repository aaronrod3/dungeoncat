// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DungeonCatCharacter.h"
#include "AbilitySystemInterface.h"
#include "DCPlayerCharacter.generated.h"

class ADCPlayerState;
class UDCAttributeSet;
class UAbilitySystemComponent;
class UGameplayAbility;
class UInputAction;
class UDCDownedComponent;

/** Maps to each granted ability's FGameplayAbilitySpec::InputID (SystemsDesign.md §2.3's 4 Knight abilities). ADCPlayerCharacter::SetupPlayerInputComponent binds IMC_DC_Default's 4 ability Input Actions to AbilityInputPressed/Released with these. */
UENUM(BlueprintType)
enum class EDCAbilityInputID : uint8
{
	None,
	BasicAttack,	// Claw Flurry / Pounce
	ShieldBash,		// Headbutt
	Dash,			// Zoomies
	Whirlwind		// Bunny Kick
};

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
 *  of a client/server pair - confirmed working via the week-1 replication proof this fix enabled
 *  (ProductionPlan.md P1, dev-confirmed 2026-08-12).
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

	UFUNCTION(BlueprintPure, Category = "DC|Downed")
	UDCDownedComponent* GetDownedComponent() const { return DownedComponent; }

	/** Routes to AbilitySystemComponent::AbilityLocalInputPressed - bound to each ability Input
	 *  Action's Started event in SetupPlayerInputComponent below. */
	UFUNCTION(BlueprintCallable, Category = "DC|Ability")
	void AbilityInputPressed(EDCAbilityInputID InputID);

	/** Routes to AbilitySystemComponent::AbilityLocalInputReleased - required for Claw Flurry/Pounce's tap-vs-hold branching (UDCGameplayAbility_BasicAttack), a no-op release for the other 3. */
	UFUNCTION(BlueprintCallable, Category = "DC|Ability")
	void AbilityInputReleased(EDCAbilityInputID InputID);

protected:

	/** Binds the 4 ability Input Actions on top of the inherited Move/Look/Jump/MouseLook bindings
	 *  (ADungeonCatCharacter::SetupPlayerInputComponent) - each routes straight to
	 *  AbilityInputPressed/Released via Enhanced Input's payload-binding overload, no wrapper
	 *  functions needed. Started->Pressed, Completed/Canceled->Released; BasicAttack's tap-vs-hold
	 *  split happens inside UDCGameplayAbility_BasicAttack itself (WaitInputRelease), not here. */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Grants the 4 real Knight abilities server-side once the ASC is valid - called from both
	 *  PossessedBy and (harmlessly, idempotently) from OnRep_PlayerState's InitAbilityActorInfo call
	 *  site. Each is granted with its EDCAbilityInputID as FGameplayAbilitySpec::InputID. */
	void GrantKnightAbilities();

	UPROPERTY(EditDefaultsOnly, Category = "DC|Ability")
	TSubclassOf<UGameplayAbility> BasicAttackAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "DC|Ability")
	TSubclassOf<UGameplayAbility> ShieldBashAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "DC|Ability")
	TSubclassOf<UGameplayAbility> DashAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "DC|Ability")
	TSubclassOf<UGameplayAbility> WhirlwindAbilityClass;

	/** Ability Input Actions, assigned on BP_DCPlayerCharacter (same convention as the inherited
	 *  Move/Look/Jump/MouseLook actions on ADungeonCatCharacter - asset references live on a
	 *  Blueprint child, not hardcoded in C++). Part of IMC_DC_Default. */
	UPROPERTY(EditAnywhere, Category = "DC|Ability|Input")
	TObjectPtr<UInputAction> BasicAttackAction;

	UPROPERTY(EditAnywhere, Category = "DC|Ability|Input")
	TObjectPtr<UInputAction> ShieldBashAction;

	UPROPERTY(EditAnywhere, Category = "DC|Ability|Input")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditAnywhere, Category = "DC|Ability|Input")
	TObjectPtr<UInputAction> WhirlwindAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DC|Downed")
	TObjectPtr<UDCDownedComponent> DownedComponent;
};
