// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DCGameplayAbility.generated.h"

class ADCPlayerCharacter;
class UDCAttributeSet;

/**
 *  Base class for every DungeonCat GameplayAbility (SystemsDesign.md §2.3). ServerInitiated by
 *  default (SystemsDesign.md's open item on LocalPredicted is left for later - revisit only if
 *  latency is a felt problem in real co-op testing, not preemptively). Convenience getters mirror
 *  zombieshooter's UZSUserWidgetBase pattern, confirmed reusable in the P0 audit.
 */
UCLASS(Abstract, HideCategories = Input)
class DUNGEONCAT_API UDCGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UDCGameplayAbility();

protected:

	UFUNCTION(BlueprintPure, Category = "DC|Ability")
	ADCPlayerCharacter* GetDCPlayerCharacter() const;

	UFUNCTION(BlueprintPure, Category = "DC|Ability")
	const UDCAttributeSet* GetDCAttributeSet() const;
};
