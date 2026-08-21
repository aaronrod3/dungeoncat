// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DCUserWidgetBase.generated.h"

class ADCPlayerCharacter;
class UDCAttributeSet;

/**
 *  Shared base for every HUD/menu widget (SystemsDesign.md §8: "every HUD/menu widget is a dedicated
 *  UDCUserWidgetBase subclass... Blueprint side is layout/Class-Defaults only"). Convenience getters
 *  mirror UDCGameplayAbility's own GetDCPlayerCharacter()/GetDCAttributeSet() pattern
 *  (zombieshooter's UZSUserWidgetBase precedent, P0-audit-confirmed reusable) so every concrete
 *  widget doesn't re-derive "get the local player's character/attribute set" itself.
 */
UCLASS(Abstract)
class DUNGEONCAT_API UDCUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()

protected:

	UFUNCTION(BlueprintPure, Category = "DC|UI")
	ADCPlayerCharacter* GetDCPlayerCharacter() const;

	UFUNCTION(BlueprintPure, Category = "DC|UI")
	UDCAttributeSet* GetDCAttributeSet() const;
};
