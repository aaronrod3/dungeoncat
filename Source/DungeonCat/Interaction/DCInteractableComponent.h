// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DCInteractableComponent.generated.h"

class ADCPlayerCharacter;
class UDCInteractableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDCOnInteracted, UDCInteractableComponent*, Interactable, ADCPlayerCharacter*, Interactor);

/**
 *  Ported pattern (not class) from zombieshooter's UZSInteractableComponent, P0-audit-confirmed
 *  reusable (SystemsDesign.md §10/§6). Attach to any actor - a loot pickup, a Hub interactable
 *  (Loadout/Vendor/Portal/Trophy Board/Cosmetics Mirror, Docs/GameplayLoops.md), a skill-gated
 *  container (Docs/Items.md) - to make it interactable via a prompt, not walk-over auto-pickup, so
 *  "who grabs it" stays server-authoritative-simple in co-op (SystemsDesign.md §6).
 *
 *  Detection/range is intentionally NOT this component's job - a character's own nearest-interactable
 *  scan drives that (not yet built; this component is passive/cheap on purpose so a scene can hold
 *  many of these at once without each one polling). Server-authoritative: OnInteract should only be
 *  called from server-authoritative code (a Server_Interact RPC on the character, not yet built) -
 *  this component has no HasAuthority() guard of its own, matching UZSInteractableComponent's own
 *  precedent of leaving that gate to the caller.
 */
UCLASS(ClassGroup = (DC), meta = (BlueprintSpawnableComponent))
class DUNGEONCAT_API UDCInteractableComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	UDCInteractableComponent();

	/** Shown in the world prompt as "<bound key> - <Verb>" once a prompt widget exists (Docs/UIUX.md's Hub HUD). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DC|Interaction")
	FText InteractionVerb = FText::FromString(TEXT("Interact"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DC|Interaction")
	float InteractionRadius = 150.f;

	/** Toggle to temporarily disable (e.g. an already-looted pickup) without destroying the component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DC|Interaction")
	bool bIsInteractable = true;

	UFUNCTION(BlueprintPure, Category = "DC|Interaction")
	bool CanInteract() const { return bIsInteractable; }

	/** Gameplay execution point - override per interactable type (a C++ subclass like
	 *  UDCItemPickupComponent, or a Blueprint override) rather than branching on type here. */
	UFUNCTION(BlueprintNativeEvent, Category = "DC|Interaction")
	void OnInteract(ADCPlayerCharacter* Interactor);
	virtual void OnInteract_Implementation(ADCPlayerCharacter* Interactor);

	/** Broadcast after a successful OnInteract - for Blueprint-side FX/sound/UI without a C++ override. */
	UPROPERTY(BlueprintAssignable, Category = "DC|Interaction")
	FDCOnInteracted OnInteracted;
};
