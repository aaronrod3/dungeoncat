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

	/** Manual cooldown check (see StartCooldown's comment for why this isn't CooldownGameplayEffectClass). 0 (the default) means no cooldown - BasicAttack never overrides this. */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	/** This project's abilities use manual cooldown tracking (see StartCooldown's comment), not GAS's built-in CooldownGameplayEffectClass system - named distinctly from the base class's own (unused here) GetCooldownTimeRemaining() to avoid a UFUNCTION name collision (that one isn't virtual). */
	UFUNCTION(BlueprintPure, Category = "DC|Ability")
	float GetDCCooldownTimeRemaining() const;

protected:

	UFUNCTION(BlueprintPure, Category = "DC|Ability")
	ADCPlayerCharacter* GetDCPlayerCharacter() const;

	UFUNCTION(BlueprintPure, Category = "DC|Ability")
	const UDCAttributeSet* GetDCAttributeSet() const;

	/** Sphere sweep from a named socket along the actor's forward vector - the audited `AnimNotify_DoAttackTrace`/`ACombatEnemy::DoAttackTrace` pattern (P0 audit, SystemsDesign.md §10), ported to a GAS-callable helper. If SocketName doesn't exist yet (no skeleton authored), falls back to the actor's own transform - non-fatal, matches AssetPipeline.md's "missing socket = default transform, not a crash" tolerance for grey-box work. Server-only (no-ops without authority) - every concrete ability must only call this from server-authoritative code. */
	TArray<FHitResult> SphereTraceFromSocket(FName SocketName, float TraceDistance, float TraceRadius) const;

	/** Sphere overlap centered on the caster's own location - for AoE abilities (Bunny Kick) rather than a forward-swept melee trace. Server-only. */
	TArray<FHitResult> SphereOverlapAroundSelf(float Radius) const;

	/** Applies DCGameplayEffect_Damage to every unique GAS-capable actor in Hits (skips anything without an ASC - e.g. world geometry the sphere trace also caught), via the Data.Damage SetByCaller tag DCDamageExecCalculation reads. Server-only. */
	void ApplyDamageToTargets(const TArray<FHitResult>& Hits, float DamageAmount) const;

	/** Applies EffectClass (no SetByCaller magnitude) to every unique GAS-capable actor in Hits - for a future Modifiers-only debuff/knockback GE. Don't use this to grant a tag via a GameplayEffectComponent - see DCGameplayEffect_AttackMoveSlow's header comment for why that crashes when the component is added in the GE's own constructor. Server-only. */
	void ApplyEffectToTargets(const TArray<FHitResult>& Hits, TSubclassOf<UGameplayEffect> EffectClass) const;

	/** Applies EffectClass to the caster itself (e.g. DCGameplayEffect_AttackMoveSlow). Server-only. */
	void ApplySelfEffect(TSubclassOf<UGameplayEffect> EffectClass) const;

	/** Cooldown length in seconds - 0 (default) means no cooldown. Set by concrete abilities that need one (ShieldBash/Dash/Whirlwind); BasicAttack leaves this at 0. Manual timestamp tracking, not GAS's CooldownGameplayEffectClass system - that system requires a GameplayEffect to grant a cooldown tag via UTargetTagsGameplayEffectComponent, which crashes when added from the GE's own constructor in UE5.8 (confirmed via a headless smoke-test crash - see DCGameplayEffect_AttackMoveSlow.cpp's comment for the exact error). Manual tracking sidesteps that entirely and needs no GameplayEffect class at all. */
	UPROPERTY(EditDefaultsOnly, Category = "DC|Ability|Cooldown")
	float CooldownDurationSeconds = 0.f;

	/** Call on successful activation for any ability with CooldownDurationSeconds > 0. */
	void StartCooldown();

private:

	/** Shared by ApplyDamageToTargets/ApplyEffectToTargets - collects unique GAS-capable (IAbilitySystemInterface) actors from a hit array, since a sweep can return multiple hit results for the same actor's multiple collision components and a melee swing should only ever hit each target once. */
	static TArray<AActor*> GetUniqueGASTargets(const TArray<FHitResult>& Hits);

	float CooldownEndTimeSeconds = -1.f;
};
