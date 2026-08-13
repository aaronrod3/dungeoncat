// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

UDCAttributeSet::UDCAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitArmor(0.f);
	InitMoveSpeed(1.f);
	InitCritChance(0.f);
	InitCritMultiplier(1.5f);
	InitDamage(0.f);
}

void UDCAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// COND_None (not COND_OwnerOnly) for every real attribute - co-op teammates need to see each
	// other's Health for a shared HUD/revive prompts (SystemsDesign.md §7/§8). Damage is a meta
	// attribute and deliberately excluded here - see its header comment.
	DOREPLIFETIME_CONDITION_NOTIFY(UDCAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDCAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDCAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDCAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDCAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDCAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDCAttributeSet, CritChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDCAttributeSet, CritMultiplier, COND_None, REPNOTIFY_Always);
}

void UDCAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
}

void UDCAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute != GetDamageAttribute())
	{
		return;
	}

	// Consume the meta attribute immediately - see the header comment on why this must never be left
	// holding a stale value between one effect execution and the next.
	const float LocalDamage = GetDamage();
	SetDamage(0.f);

	if (LocalDamage <= 0.f)
	{
		return;
	}

	SetHealth(FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth()));

	// P1 replication-proof decision (SystemsDesign.md §10 open items): a debug log/on-screen message
	// is the agreed "proof it worked" for the week-1 two-client test, not a visual GameplayCue.
	// Deliberately temporary - remove once real GameplayCues exist for damage feedback.
	const FString Msg = FString::Printf(TEXT("%s took %.1f damage (Health now %.1f/%.1f)"),
		*GetNameSafe(Data.Target.GetAvatarActor()), LocalDamage, GetHealth(), GetMaxHealth());
	UE_LOG(LogTemp, Log, TEXT("%s"), *Msg);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Yellow, Msg);
	}
}

void UDCAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDCAttributeSet, Health, OldValue);
}

void UDCAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDCAttributeSet, MaxHealth, OldValue);
}

void UDCAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDCAttributeSet, Stamina, OldValue);
}

void UDCAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDCAttributeSet, MaxStamina, OldValue);
}

void UDCAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDCAttributeSet, Armor, OldValue);
}

void UDCAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDCAttributeSet, MoveSpeed, OldValue);
}

void UDCAttributeSet::OnRep_CritChance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDCAttributeSet, CritChance, OldValue);
}

void UDCAttributeSet::OnRep_CritMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDCAttributeSet, CritMultiplier, OldValue);
}
