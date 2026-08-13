// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "DCAbilitySystemComponent.generated.h"

/**
 *  Thin project subclass - exists as the one place to add project-wide ASC behavior later (e.g. a
 *  centralized "ability activation failed" feedback hook) rather than reaching into Epic's base class
 *  from scattered call sites. Empty for now; SystemsDesign.md §2.2 only requires it exist and live on
 *  ADCPlayerState.
 */
UCLASS()
class DUNGEONCAT_API UDCAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
};
