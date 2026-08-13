// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DCAttributeSet.generated.h"

#define DC_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 *  The one AttributeSet for the Knight beta (SystemsDesign.md §2.1). Lives on ADCPlayerState via its
 *  AbilitySystemComponent, not on the character - see ADCPlayerState's header comment.
 *
 *  Damage is a meta attribute: UDCDamageExecCalculation is the single entry point for all damage
 *  (SystemsDesign.md §2.4) and writes its final, post-mitigation result here; PostGameplayEffectExecute
 *  below converts that into a real Health change and resets Damage to 0 in the same call, so nothing
 *  should ever read a stale Damage value. Nothing outside this AttributeSet + the exec calculation
 *  should ever touch Health directly - same "single entry point" discipline the P0 audit confirmed
 *  worth carrying over from zombieshooter's UZSHealthComponent, reimplemented in GAS's own idiom
 *  instead of a hand-rolled ReplicatedUsing/OnRep component (GAS's attribute replication already
 *  supersedes that pattern - see SystemsDesign.md §10).
 */
UCLASS()
class DUNGEONCAT_API UDCAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UDCAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "DC|Attributes")
	FGameplayAttributeData Health;
	DC_ATTRIBUTE_ACCESSORS(UDCAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "DC|Attributes")
	FGameplayAttributeData MaxHealth;
	DC_ATTRIBUTE_ACCESSORS(UDCAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "DC|Attributes")
	FGameplayAttributeData Stamina;
	DC_ATTRIBUTE_ACCESSORS(UDCAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "DC|Attributes")
	FGameplayAttributeData MaxStamina;
	DC_ATTRIBUTE_ACCESSORS(UDCAttributeSet, MaxStamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "DC|Attributes")
	FGameplayAttributeData Armor;
	DC_ATTRIBUTE_ACCESSORS(UDCAttributeSet, Armor)

	/** Multiplier applied on top of the base walk speed (SystemsDesign.md §2.1) - 1.0 = no change, feeds CharacterMovementComponent::MaxWalkSpeed, never set directly on the movement component. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed, Category = "DC|Attributes")
	FGameplayAttributeData MoveSpeed;
	DC_ATTRIBUTE_ACCESSORS(UDCAttributeSet, MoveSpeed)

	/** 0-1. Deliberately 0 by default for the beta (P1 design decision, SystemsDesign.md §10 open items) - plumbed through so a later itemization pass doesn't need a schema change, but the Knight doesn't roll crits yet. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CritChance, Category = "DC|Attributes")
	FGameplayAttributeData CritChance;
	DC_ATTRIBUTE_ACCESSORS(UDCAttributeSet, CritChance)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CritMultiplier, Category = "DC|Attributes")
	FGameplayAttributeData CritMultiplier;
	DC_ATTRIBUTE_ACCESSORS(UDCAttributeSet, CritMultiplier)

	/** Meta attribute - never replicated (see GetLifetimeReplicatedProps), reset to 0 immediately after being consumed in PostGameplayEffectExecute. Nothing should read this outside that function. */
	UPROPERTY(BlueprintReadOnly, Category = "DC|Attributes|Meta")
	FGameplayAttributeData Damage;
	DC_ATTRIBUTE_ACCESSORS(UDCAttributeSet, Damage)

protected:

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CritChance(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CritMultiplier(const FGameplayAttributeData& OldValue);
};
