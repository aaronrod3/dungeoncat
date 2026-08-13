// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DCGameplayAbility.h"
#include "DCGameplayAbility_TestDamage.generated.h"

class UGameplayEffect;

/**
 *  Week-1 replication-proof ability only (ProductionPlan.md P1) - applies a fixed amount of test
 *  damage to self through the real GAS damage pipeline (UDCDamageExecCalculation), so activating it
 *  on one PIE client and confirming the Health change is visible/correct on both proves replication
 *  works before any of the real 4 Knight abilities get built.
 *
 *  Pure C++, no montage/Blueprint asset dependency on purpose, and self-targeted rather than traced -
 *  this is a replication proof, not a real attack. Triggered via
 *  ADCPlayerCharacter::DC_TestAbility_DealDamageToSelf's Exec console command rather than a bound
 *  Input Action, since no unreal-mcp toolset can create Input Action/IMC assets (confirmed in
 *  zombieshooter's own MCP-tooling notes) and no editor session is available to make one by hand this
 *  pass.
 *
 *  Delete this class (and UDCGameplayEffect_TestDamage) once the real 4 abilities exist and the
 *  replication proof has served its purpose.
 */
UCLASS()
class DUNGEONCAT_API UDCGameplayAbility_TestDamage : public UDCGameplayAbility
{
	GENERATED_BODY()

public:

	UDCGameplayAbility_TestDamage();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Flat pre-mitigation damage this test ability deals - EditDefaultsOnly so it's tweakable without a recompile even though this ability has no Blueprint asset of its own. */
	UPROPERTY(EditDefaultsOnly, Category = "DC|Test")
	float TestDamageAmount = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "DC|Test")
	TSubclassOf<UGameplayEffect> TestDamageEffectClass;
};
