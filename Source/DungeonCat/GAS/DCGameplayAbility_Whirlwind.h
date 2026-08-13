// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DCGameplayAbility.h"
#include "DCGameplayAbility_Whirlwind.generated.h"

/**
 *  Bunny Kick (SystemsDesign.md §2.3) - GA_DC_Knight_Whirlwind. AoE crowd-control around the
 *  player, on cooldown, commits fully (not Dash-cancelable). The one deliberate "instinct breaks
 *  through" exception in the kit (AssetPipeline.md §4, dev-confirmed 2026-08-12) - weapon held
 *  loose/sheathed, claws out, an all-limbs flurry, not a weapon-spin attack.
 */
UCLASS()
class DUNGEONCAT_API UDCGameplayAbility_Whirlwind : public UDCGameplayAbility
{
	GENERATED_BODY()

public:

	UDCGameplayAbility_Whirlwind();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "DC|Whirlwind")
	float Damage = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "DC|Whirlwind")
	float Radius = 250.f;
};
