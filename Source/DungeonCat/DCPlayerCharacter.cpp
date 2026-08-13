// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCPlayerCharacter.h"
#include "DCPlayerState.h"
#include "GAS/DCAttributeSet.h"
#include "GAS/DCAbilitySystemComponent.h"
#include "GAS/DCGameplayAbility_TestDamage.h"
#include "AbilitySystemComponent.h"

ADCPlayerCharacter::ADCPlayerCharacter()
{
	TestAbilityClass = UDCGameplayAbility_TestDamage::StaticClass();
}

UAbilitySystemComponent* ADCPlayerCharacter::GetAbilitySystemComponent() const
{
	if (const ADCPlayerState* DCPlayerState = GetDCPlayerState())
	{
		return DCPlayerState->GetAbilitySystemComponent();
	}
	return nullptr;
}

ADCPlayerState* ADCPlayerCharacter::GetDCPlayerState() const
{
	return GetPlayerState<ADCPlayerState>();
}

UDCAttributeSet* ADCPlayerCharacter::GetDCAttributeSet() const
{
	if (ADCPlayerState* DCPlayerState = GetDCPlayerState())
	{
		return DCPlayerState->GetDCAttributeSet();
	}
	return nullptr;
}

void ADCPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server-side half of the PlayerState/ASC init-timing fix - PossessedBy only ever runs on the
	// server, and PlayerState is guaranteed valid here (see class header comment).
	if (ADCPlayerState* DCPlayerState = GetDCPlayerState())
	{
		if (UAbilitySystemComponent* ASC = DCPlayerState->GetAbilitySystemComponent())
		{
			ASC->InitAbilityActorInfo(DCPlayerState, this);
			GrantTestAbility();
		}
	}
}

void ADCPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Client-side half of the same fix - OnRep_PlayerState is the first point a client has a valid,
	// replicated PlayerState to hand to InitAbilityActorInfo. The server already did this in
	// PossessedBy above; re-running it here on the listen-server host's own local client is a
	// harmless no-op, InitAbilityActorInfo is idempotent.
	if (ADCPlayerState* DCPlayerState = GetDCPlayerState())
	{
		if (UAbilitySystemComponent* ASC = DCPlayerState->GetAbilitySystemComponent())
		{
			ASC->InitAbilityActorInfo(DCPlayerState, this);
		}
	}
}

void ADCPlayerCharacter::GrantTestAbility()
{
	if (!HasAuthority() || !TestAbilityClass)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->GiveAbility(FGameplayAbilitySpec(TestAbilityClass, 1, INDEX_NONE, this));
	}
}

void ADCPlayerCharacter::DC_TestAbility_DealDamageToSelf()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->TryActivateAbilityByClass(TestAbilityClass);
	}
}
