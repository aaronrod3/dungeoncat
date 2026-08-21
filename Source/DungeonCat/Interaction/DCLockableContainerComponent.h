// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DCInteractableComponent.h"
#include "DCLockableContainerComponent.generated.h"

/**
 *  Docs/Items.md's skill-gated container: "a lockable container carries RequiredSkill +
 *  RequiredSkillLevel; locked until the opening player's tracked skill level meets the requirement."
 *  A UDCInteractableComponent subclass rather than a parallel system, so it reuses the same
 *  interact-prompt flow every other interactable uses (SystemsDesign.md §6). What's actually inside a
 *  container once unlocked isn't decided yet (Docs/Items.md's own open item) - OnUnlocked below is a
 *  BlueprintImplementableEvent hook for that content-specific reveal, not implemented here.
 */
UCLASS(ClassGroup = (DC), meta = (BlueprintSpawnableComponent))
class DUNGEONCAT_API UDCLockableContainerComponent : public UDCInteractableComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DC|Item")
	FGameplayTag RequiredSkill;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DC|Item", meta = (ClampMin = "0"))
	int32 RequiredSkillLevel = 1;

	/** Server-authoritative gate check against the interactor's ADCPlayerState::GetSkillLevel -
	 *  proceeds to Super (broadcasts OnInteracted) and OnUnlocked only if the requirement is met;
	 *  otherwise a no-op (a locked-feedback UI reaction is a future concern, not built here). */
	virtual void OnInteract_Implementation(ADCPlayerCharacter* Interactor) override;

	/** Content-specific reveal (loot spawn, door animation, etc.) - left to a Blueprint override since
	 *  "what's actually inside" isn't decided yet (Docs/Items.md). */
	UFUNCTION(BlueprintImplementableEvent, Category = "DC|Item")
	void OnUnlocked(ADCPlayerCharacter* Interactor);
};
