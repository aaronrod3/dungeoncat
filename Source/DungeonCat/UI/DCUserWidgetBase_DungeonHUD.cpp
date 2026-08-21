// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCUserWidgetBase_DungeonHUD.h"
#include "DCPlayerCharacter.h"
#include "GAS/DCAttributeSet.h"
#include "Combat/DCDownedComponent.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/Widget.h"

void UDCUserWidgetBase_DungeonHUD::NativeConstruct()
{
	Super::NativeConstruct();

	ADCPlayerCharacter* Character = GetDCPlayerCharacter();
	if (!Character)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UDCAttributeSet::GetHealthAttribute()).AddUObject(this, &UDCUserWidgetBase_DungeonHUD::HandleHealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UDCAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UDCUserWidgetBase_DungeonHUD::HandleHealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UDCAttributeSet::GetStaminaAttribute()).AddUObject(this, &UDCUserWidgetBase_DungeonHUD::HandleStaminaChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UDCAttributeSet::GetMaxStaminaAttribute()).AddUObject(this, &UDCUserWidgetBase_DungeonHUD::HandleStaminaChanged);
	}

	if (UDCDownedComponent* Downed = Character->GetDownedComponent())
	{
		Downed->OnDownedChanged.AddDynamic(this, &UDCUserWidgetBase_DungeonHUD::HandleDownedChanged);
	}

	// Initial paint - the delegates above only fire on a future change, not retroactively for
	// whatever Health/Stamina/downed-state already is at widget-construction time.
	RefreshHealthBar();
	RefreshStaminaBar();
	if (DownedOverlay)
	{
		const bool bStartsDowned = Character->GetDownedComponent() && Character->GetDownedComponent()->IsDowned();
		DownedOverlay->SetVisibility(bStartsDowned ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UDCUserWidgetBase_DungeonHUD::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	RefreshHealthBar();
}

void UDCUserWidgetBase_DungeonHUD::HandleStaminaChanged(const FOnAttributeChangeData& Data)
{
	RefreshStaminaBar();
}

void UDCUserWidgetBase_DungeonHUD::HandleDownedChanged(bool bIsDowned)
{
	if (DownedOverlay)
	{
		DownedOverlay->SetVisibility(bIsDowned ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UDCUserWidgetBase_DungeonHUD::RefreshHealthBar() const
{
	if (!HealthBar_Self)
	{
		return;
	}

	const UDCAttributeSet* AttributeSet = GetDCAttributeSet();
	if (!AttributeSet)
	{
		return;
	}

	const float MaxHealth = AttributeSet->GetMaxHealth();
	HealthBar_Self->SetPercent(MaxHealth > 0.f ? AttributeSet->GetHealth() / MaxHealth : 0.f);
}

void UDCUserWidgetBase_DungeonHUD::RefreshStaminaBar() const
{
	if (!StaminaBar_Self)
	{
		return;
	}

	const UDCAttributeSet* AttributeSet = GetDCAttributeSet();
	if (!AttributeSet)
	{
		return;
	}

	const float MaxStamina = AttributeSet->GetMaxStamina();
	StaminaBar_Self->SetPercent(MaxStamina > 0.f ? AttributeSet->GetStamina() / MaxStamina : 0.f);
}
