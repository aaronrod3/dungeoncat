// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DCGameplayAbility.h"
#include "DCGameplayAbility_ShieldBash.generated.h"

/**
 *  Headbutt (SystemsDesign.md §2.3) - GA_DC_Knight_ShieldBash. Short-range gap-closer, Stamina cost,
 *  on cooldown, commits fully (not Dash-cancelable - see Dash's own header comment). Weapon/shield
 *  stay equipped - the head just dips forward through the motion, layered on a normal shield-charge
 *  (AssetPipeline.md §4).
 *
 *  Stagger application is deferred, not yet implemented: SystemsDesign.md §2.3 calls for applying
 *  "State.Staggered" to hit targets, but nothing in the codebase reads that tag yet (no enemy AI
 *  exists - P2 work) and the originally-planned mechanism (a GameplayEffect granting the tag via
 *  UTargetTagsGameplayEffectComponent) crashes when added from the effect's own constructor in
 *  UE5.8 - confirmed via a headless smoke-test crash, not a guess (see
 *  DCGameplayEffect_AttackMoveSlow.cpp's comment for the exact error). Revisit with a per-target
 *  timed UAbilitySystemComponent::AddLooseGameplayTag/RemoveLooseGameplayTag call once an enemy
 *  actually needs to react to being staggered - deals damage only for now.
 */
UCLASS()
class DUNGEONCAT_API UDCGameplayAbility_ShieldBash : public UDCGameplayAbility
{
	GENERATED_BODY()

public:

	UDCGameplayAbility_ShieldBash();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "DC|ShieldBash")
	float Damage = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "DC|ShieldBash")
	float TraceDistance = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "DC|ShieldBash")
	float TraceRadius = 70.f;

	/** Forward launch speed for the gap-close lunge. */
	UPROPERTY(EditDefaultsOnly, Category = "DC|ShieldBash")
	float LaunchSpeed = 900.f;
};
