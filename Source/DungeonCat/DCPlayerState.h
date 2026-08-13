// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "DCPlayerState.generated.h"

class UDCAbilitySystemComponent;
class UDCAttributeSet;

/**
 *  Hosts the ability system for every DungeonCat player (SystemsDesign.md §2.2) - lives on
 *  PlayerState, not the character, so the ASC survives character destroy/respawn (co-op revive,
 *  death/respawn per SystemsDesign.md §7) without an ASC re-init + ability re-grant dance.
 *  ADCPlayerCharacter forwards IAbilitySystemInterface here rather than owning its own ASC.
 */
UCLASS()
class DUNGEONCAT_API ADCPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ADCPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "DC|Ability")
	UDCAttributeSet* GetDCAttributeSet() const { return AttributeSet; }

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DC|Ability")
	TObjectPtr<UDCAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DC|Ability")
	TObjectPtr<UDCAttributeSet> AttributeSet;
};
