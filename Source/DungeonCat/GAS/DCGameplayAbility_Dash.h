// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DCGameplayAbility.h"
#include "DCGameplayAbility_Dash.generated.h"

/**
 *  Zoomies (SystemsDesign.md §2.3) - GA_DC_Knight_Dash. Burst mobility + i-frames, on cooldown.
 *  Cancels Claw Flurry/Pounce specifically on activation (P1 design decision - Zoomies is the
 *  panic/escape button; Headbutt/Bunny Kick commit fully and are deliberately NOT cancelable by
 *  this). Pure mobility, no weapon interaction (AssetPipeline.md §4).
 *
 *  i-frames are a loose ASC tag ("State.Invulnerable"), added on activation and removed when the
 *  dash ends - NOT a GameplayEffect, since UGameplayEffect::FindOrAddComponent (the mechanism that
 *  would grant the tag via a GE) crashes when called from the effect's own constructor in UE5.8
 *  (confirmed via a headless smoke-test crash). A loose tag applied directly by the ability at
 *  runtime sidesteps that entirely.
 */
UCLASS()
class DUNGEONCAT_API UDCGameplayAbility_Dash : public UDCGameplayAbility
{
	GENERATED_BODY()

public:

	UDCGameplayAbility_Dash();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "DC|Dash")
	float LaunchSpeed = 1400.f;

	/** How long the ability stays active (and i-frames last) before ending - grey-box stand-in for "until the dash montage finishes." */
	UPROPERTY(EditDefaultsOnly, Category = "DC|Dash")
	float DashDurationSeconds = 0.3f;

protected:

	UFUNCTION()
	void HandleDashFinished();
};
