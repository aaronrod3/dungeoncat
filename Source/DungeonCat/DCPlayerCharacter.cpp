// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCPlayerCharacter.h"
#include "DCPlayerState.h"
#include "GAS/DCAttributeSet.h"
#include "GAS/DCAbilitySystemComponent.h"
#include "GAS/DCGameplayAbility_BasicAttack.h"
#include "GAS/DCGameplayAbility_ShieldBash.h"
#include "GAS/DCGameplayAbility_Dash.h"
#include "GAS/DCGameplayAbility_Whirlwind.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "Combat/DCDownedComponent.h"

ADCPlayerCharacter::ADCPlayerCharacter()
{
	BasicAttackAbilityClass = UDCGameplayAbility_BasicAttack::StaticClass();
	ShieldBashAbilityClass = UDCGameplayAbility_ShieldBash::StaticClass();
	DashAbilityClass = UDCGameplayAbility_Dash::StaticClass();
	WhirlwindAbilityClass = UDCGameplayAbility_Whirlwind::StaticClass();

	DownedComponent = CreateDefaultSubobject<UDCDownedComponent>(TEXT("DownedComponent"));
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
			GrantKnightAbilities();
		}

		if (DownedComponent)
		{
			DownedComponent->BindToAttributeSet(DCPlayerState->GetDCAttributeSet());
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

		if (DownedComponent)
		{
			DownedComponent->BindToAttributeSet(DCPlayerState->GetDCAttributeSet());
		}
	}
}

void ADCPlayerCharacter::GrantKnightAbilities()
{
	if (!HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	if (BasicAttackAbilityClass)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(BasicAttackAbilityClass, 1, static_cast<int32>(EDCAbilityInputID::BasicAttack), this));
	}
	if (ShieldBashAbilityClass)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(ShieldBashAbilityClass, 1, static_cast<int32>(EDCAbilityInputID::ShieldBash), this));
	}
	if (DashAbilityClass)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(DashAbilityClass, 1, static_cast<int32>(EDCAbilityInputID::Dash), this));
	}
	if (WhirlwindAbilityClass)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(WhirlwindAbilityClass, 1, static_cast<int32>(EDCAbilityInputID::Whirlwind), this));
	}
}

void ADCPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(BasicAttackAction, ETriggerEvent::Started, this, &ADCPlayerCharacter::AbilityInputPressed, EDCAbilityInputID::BasicAttack);
		EnhancedInputComponent->BindAction(BasicAttackAction, ETriggerEvent::Completed, this, &ADCPlayerCharacter::AbilityInputReleased, EDCAbilityInputID::BasicAttack);
		EnhancedInputComponent->BindAction(BasicAttackAction, ETriggerEvent::Canceled, this, &ADCPlayerCharacter::AbilityInputReleased, EDCAbilityInputID::BasicAttack);

		EnhancedInputComponent->BindAction(ShieldBashAction, ETriggerEvent::Started, this, &ADCPlayerCharacter::AbilityInputPressed, EDCAbilityInputID::ShieldBash);
		EnhancedInputComponent->BindAction(ShieldBashAction, ETriggerEvent::Completed, this, &ADCPlayerCharacter::AbilityInputReleased, EDCAbilityInputID::ShieldBash);
		EnhancedInputComponent->BindAction(ShieldBashAction, ETriggerEvent::Canceled, this, &ADCPlayerCharacter::AbilityInputReleased, EDCAbilityInputID::ShieldBash);

		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ADCPlayerCharacter::AbilityInputPressed, EDCAbilityInputID::Dash);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Completed, this, &ADCPlayerCharacter::AbilityInputReleased, EDCAbilityInputID::Dash);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Canceled, this, &ADCPlayerCharacter::AbilityInputReleased, EDCAbilityInputID::Dash);

		EnhancedInputComponent->BindAction(WhirlwindAction, ETriggerEvent::Started, this, &ADCPlayerCharacter::AbilityInputPressed, EDCAbilityInputID::Whirlwind);
		EnhancedInputComponent->BindAction(WhirlwindAction, ETriggerEvent::Completed, this, &ADCPlayerCharacter::AbilityInputReleased, EDCAbilityInputID::Whirlwind);
		EnhancedInputComponent->BindAction(WhirlwindAction, ETriggerEvent::Canceled, this, &ADCPlayerCharacter::AbilityInputReleased, EDCAbilityInputID::Whirlwind);
	}
}

void ADCPlayerCharacter::AbilityInputPressed(EDCAbilityInputID InputID)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->AbilityLocalInputPressed(static_cast<int32>(InputID));
	}
}

void ADCPlayerCharacter::AbilityInputReleased(EDCAbilityInputID InputID)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->AbilityLocalInputReleased(static_cast<int32>(InputID));
	}
}
