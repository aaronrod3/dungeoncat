// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DCDownedComponent.generated.h"

class UDCAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDCOnDownedChanged, bool, bNewIsDowned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDCOnPlayerDeath);

/**
 *  Ported state machine (not component) from zombieshooter's UZSHealthComponent's downed/revive
 *  logic (P0 audit, SystemsDesign.md §7/§8/§10) - health-depleted -> enter downed -> timer ->
 *  revive-or-die, "already downed = finishing blow." Health itself stays in UDCAttributeSet
 *  (SystemsDesign.md §2.1), not duplicated here; this component only owns the downed/dead state
 *  machine layered on top of it.
 *
 *  Attach to ADCPlayerCharacter only - the docs never describe a downed state for enemies (they just
 *  die outright, ADCEnemyCharacter::Die()). ADCPlayerCharacter owns the actual bind-to-AttributeSet
 *  call (BindToAttributeSet below), invoked from the same PossessedBy/OnRep_PlayerState pair that
 *  already handles the ASC-not-ready-yet race for ability granting - this component's own BeginPlay
 *  can't safely do that binding itself, since the ASC/AttributeSet live on PlayerState and may not be
 *  valid yet when this component's BeginPlay runs.
 *
 *  Spec (SystemsDesign.md §8): 60s downed duration, revive requires a teammate within ~150uu
 *  channeling ~3s (the channel timing itself belongs to an interact system that doesn't exist yet -
 *  flagged below, not simulated here), a downed player crawls slowly (MaxWalkSpeed scaled) and has no
 *  abilities (State.Downed loose tag applied/removed - each ability still needs to add this to its
 *  own ActivationBlockedTags to actually respect it, not done here to avoid touching the 4 already-
 *  shipped P1 Knight abilities in this pass) and is still finishable by a fresh hit.
 */
UCLASS(ClassGroup = (DC), meta = (BlueprintSpawnableComponent))
class DUNGEONCAT_API UDCDownedComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UDCDownedComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "DC|Downed")
	bool IsDowned() const { return bIsDowned; }

	UFUNCTION(BlueprintPure, Category = "DC|Downed")
	bool IsDead() const { return bIsDead; }

	/** Call once the owner's ASC/AttributeSet are actually valid (ADCPlayerCharacter::PossessedBy /
	 *  OnRep_PlayerState) - idempotent, safe to call from both. Binds HandleOutOfHealth to
	 *  AttributeSetIn::OnOutOfHealth. */
	void BindToAttributeSet(UDCAttributeSet* AttributeSetIn);

	/** The teammate-revive entry point - NOT gated on distance/channel-time itself (that belongs to
	 *  the not-yet-built interact system; ReviveRadius/ReviveChannelSeconds below are the numbers a
	 *  future interact flow should enforce before calling this). No-op unless currently downed.
	 *  Bumps Health up to ReviveHealthAmount (never down) and ends the downed state. Server-only. */
	UFUNCTION(BlueprintCallable, Category = "DC|Downed")
	void Server_ReviveDowned();

	UPROPERTY(BlueprintAssignable, Category = "DC|Downed")
	FDCOnDownedChanged OnDownedChanged;

	UPROPERTY(BlueprintAssignable, Category = "DC|Downed")
	FDCOnPlayerDeath OnPlayerDeath;

	UPROPERTY(EditDefaultsOnly, Category = "DC|Downed", meta = (ClampMin = "0"))
	float DownedDurationSeconds = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "DC|Downed", meta = (ClampMin = "0"))
	float ReviveHealthAmount = 20.f;

	/** Reference value for the not-yet-built interact system to enforce - not read/enforced by this
	 *  component itself (see class comment). */
	UPROPERTY(EditDefaultsOnly, Category = "DC|Downed", meta = (ClampMin = "0"))
	float ReviveRadius = 150.f;

	/** Same as ReviveRadius above - a spec number for the future interact flow, not enforced here. */
	UPROPERTY(EditDefaultsOnly, Category = "DC|Downed", meta = (ClampMin = "0"))
	float ReviveChannelSeconds = 3.f;

	/** MaxWalkSpeed multiplier applied while downed ("crawls slowly," SystemsDesign.md §8) - 1 = no
	 *  slow, lower = slower. Applied/removed directly on the owning Character's movement component in
	 *  Server_EnterDowned/Server_ExitDowned. */
	UPROPERTY(EditDefaultsOnly, Category = "DC|Downed", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DownedMoveSpeedMultiplier = 0.3f;

protected:

	UFUNCTION()
	void HandleOutOfHealth();

	void Server_EnterDowned();
	void Server_ExitDowned();
	void HandleDownedTimerExpired();

	/** Server-only: sets bIsDead, broadcasts OnPlayerDeath. What actually happens next (run-fail for a
	 *  solo player, per SystemsDesign.md §7) is P4/GameMode-level flow that doesn't exist yet - this
	 *  component only owns the health-side state transition, same scope boundary
	 *  UZSHealthComponent::Die() drew for its own equivalent. */
	void Die();

	UPROPERTY(ReplicatedUsing = OnRep_IsDowned, VisibleAnywhere, BlueprintReadOnly, Category = "DC|Downed")
	bool bIsDowned = false;

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, VisibleAnywhere, BlueprintReadOnly, Category = "DC|Downed")
	bool bIsDead = false;

	UFUNCTION()
	void OnRep_IsDowned();

	UFUNCTION()
	void OnRep_IsDead();

	UPROPERTY()
	TObjectPtr<UDCAttributeSet> BoundAttributeSet;

	FTimerHandle DownedTimerHandle;

	/** Captured in OnRep_IsDowned when entering downed, restored exactly on exit - see that function's
	 *  own comment for why this avoids a multiply/divide-drift approach. Not replicated - each machine
	 *  independently captures its own Movement->MaxWalkSpeed at the moment it processes the transition. */
	float PreDownedMaxWalkSpeed = 0.f;
};
