// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DCUserWidgetBase.h"
#include "DCUserWidgetBase_DungeonHUD.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class UWidget;
struct FOnAttributeChangeData;

/**
 *  The in-run HUD (Docs/UIUX.md's Dungeon HUD spec - exact component list/positions/BP-build steps
 *  live there; this class only owns the native functionality side, per that doc's "functionality is
 *  native, Blueprint is layout/styling only" rule). BindWidget property names match UIUX.md's table
 *  exactly, so a WBP_DC_HUD_Dungeon built to that spec binds to this with zero graph logic.
 *
 *  Wired to real, already-shipped P1 systems: own Health/Stamina via the ASC's attribute-change
 *  delegates (not polling), and the downed overlay via UDCDownedComponent::OnDownedChanged (this
 *  session's earlier cluster). NOT wired, flagged rather than faked: ally health/stamina bars (needs
 *  the same GameState->PlayerArray multiplayer-aware lookup the P2 targeting fix established, not
 *  duplicated blind here) and the 5 ability-icon cooldown fills (needs mapping "which granted ability
 *  is Slot X," a reasonable follow-up once ability-slot metadata exists in a queryable form).
 */
UCLASS(Abstract)
class DUNGEONCAT_API UDCUserWidgetBase_DungeonHUD : public UDCUserWidgetBase
{
	GENERATED_BODY()

protected:

	virtual void NativeConstruct() override;

	// Plain C++ member functions, not UFUNCTIONs - bound via AddUObject below (a native multicast
	// delegate, FOnGameplayAttributeValueChange), which needs no UHT reflection. Marking these
	// UFUNCTION() would force UHT to reflect FOnAttributeChangeData, which fails on just a forward
	// declaration (confirmed by hitting exactly that error) and isn't needed for this binding anyway.
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleStaminaChanged(const FOnAttributeChangeData& Data);

	/** This one IS a real UFUNCTION - bound via AddDynamic below, since
	 *  UDCDownedComponent::OnDownedChanged is a BlueprintAssignable dynamic delegate. */
	UFUNCTION()
	void HandleDownedChanged(bool bIsDowned);

	void RefreshHealthBar() const;
	void RefreshStaminaBar() const;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar_Self;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar_Self;

	/** Declared per Docs/UIUX.md's spec, not yet wired - see class comment. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar_Ally1;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar_Ally2;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ObjectiveText;

	/** The paw-print ability cluster (Docs/UIUX.md) - icon textures/positions are BP-side; cooldown
	 *  fill/greying-out isn't wired yet, see class comment. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> AbilityIcon_Ult;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> AbilityIcon_Consumable;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> AbilityIcon_West;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> AbilityIcon_East;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> AbilityIcon_South;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> DownedOverlay;
};
