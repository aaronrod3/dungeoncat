// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DCGameplayAbility.h"
#include "DCGameplayAbility_BasicAttack.generated.h"

/**
 *  Claw Flurry / Pounce (SystemsDesign.md §2.3) - GA_DC_Knight_BasicAttack. One ability/hotkey, tap
 *  vs hold: tapping chains the 3-hit Claw Flurry combo (Swipe -> Rake -> Shred), holding past
 *  HoldThresholdSeconds charges Pounce, a single heavy hit released on button-up. No cooldown/cost -
 *  the bread-and-butter attack. See AssetPipeline.md §4 for the weapon-consistent animation intent
 *  (sword-slash combo / leaping sword-lunge, not literal bare-claw animations).
 *
 *  Grey-box hit timing: no montage/AnimNotify exists yet, so each hit lands immediately at the
 *  moment of input release rather than at a notify-driven frame. Swap to notify-driven timing once
 *  real animation exists - PerformClawFlurryHit/PerformPounce don't need to change, just what calls
 *  them.
 */
UCLASS()
class DUNGEONCAT_API UDCGameplayAbility_BasicAttack : public UDCGameplayAbility
{
	GENERATED_BODY()

public:

	UDCGameplayAbility_BasicAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Seconds of hold before the input counts as charging Pounce instead of a Claw Flurry tap. */
	UPROPERTY(EditDefaultsOnly, Category = "DC|BasicAttack")
	float HoldThresholdSeconds = 0.25f;

	/** Damage per Claw Flurry hit - index 0 = Swipe, 1 = Rake, 2 = Shred (AssetPipeline.md §4). */
	UPROPERTY(EditDefaultsOnly, Category = "DC|BasicAttack")
	TArray<float> ClawFlurryHitDamage = { 8.f, 8.f, 12.f };

	UPROPERTY(EditDefaultsOnly, Category = "DC|BasicAttack")
	float PounceDamage = 25.f;

	/** Claw Flurry's forward-sweep reach - a quick swipe. */
	UPROPERTY(EditDefaultsOnly, Category = "DC|BasicAttack")
	float ClawFlurryTraceDistance = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "DC|BasicAttack")
	float ClawFlurryTraceRadius = 60.f;

	/** Pounce's reach - a leaping lunge, noticeably further than a Claw Flurry swipe. */
	UPROPERTY(EditDefaultsOnly, Category = "DC|BasicAttack")
	float PounceTraceDistance = 350.f;

	UPROPERTY(EditDefaultsOnly, Category = "DC|BasicAttack")
	float PounceTraceRadius = 80.f;

	/** Re-taps within this long of the last hit continue the combo; longer than this resets to Swipe. */
	UPROPERTY(EditDefaultsOnly, Category = "DC|BasicAttack")
	float ComboWindowSeconds = 1.2f;

protected:

	UFUNCTION()
	void HandleInputReleased(float TimeHeld);

	UFUNCTION()
	void HandleHoldThresholdReached();

	void PerformClawFlurryHit();
	void PerformPounce();

private:

	bool bIsCharging = false;
	int32 ComboIndex = 0;
	float LastHitTime = -1000.f;
};
