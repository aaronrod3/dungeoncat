// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/DCInteractableComponent.h"
#include "DCItemPickupComponent.generated.h"

class UDCItemConfig;

/**
 *  The concrete pickup behavior Docs/Items.md describes ("stat trinkets auto-apply their
 *  GameplayEffect on pickup ... interact-prompt, not walk-over auto-pickup"). Attach alongside (or
 *  in place of) a plain UDCInteractableComponent on any pickup actor. Only handles the
 *  GrantedEffects auto-apply path for now - the beta consumable's "usable directly from a simple
 *  carried-list" behavior needs a carried-list that doesn't exist anywhere yet (flagged, not
 *  silently assumed); Item->HealthRestoreAmount > 0 items are picked up but not yet usable.
 */
UCLASS(ClassGroup = (DC), meta = (BlueprintSpawnableComponent))
class DUNGEONCAT_API UDCItemPickupComponent : public UDCInteractableComponent
{
	GENERATED_BODY()

public:

	UDCItemPickupComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DC|Item")
	TObjectPtr<UDCItemConfig> Item;

	/** Server-authoritative (HasAuthority()-gated, per SystemsDesign.md §3.2's "every ... loot pickup
	 *  is server-authoritative" rule) - applies Item's GrantedEffects to the interactor's ASC, then
	 *  disables further interaction and destroys the owning actor. No-op if Item is unset or this
	 *  component is already non-interactable (guards a double-pickup race between two co-op players
	 *  reaching it in the same tick). */
	virtual void OnInteract_Implementation(ADCPlayerCharacter* Interactor) override;
};
