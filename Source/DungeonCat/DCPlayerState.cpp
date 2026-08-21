// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCPlayerState.h"
#include "GAS/DCAbilitySystemComponent.h"
#include "GAS/DCAttributeSet.h"
#include "Net/UnrealNetwork.h"

ADCPlayerState::ADCPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UDCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed: attributes replicate to everyone (co-op teammates need to see each other's Health for a
	// shared HUD/revive prompts, SystemsDesign.md §7/§8) while gameplay effects/cues stay owner-only.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UDCAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ADCPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ADCPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADCPlayerState, SkillLevels);
}

int32 ADCPlayerState::GetSkillLevel(FGameplayTag Skill) const
{
	for (const FDCSkillLevelEntry& Entry : SkillLevels)
	{
		if (Entry.Skill == Skill)
		{
			return Entry.Level;
		}
	}
	return 0;
}

void ADCPlayerState::Server_SetSkillLevel(FGameplayTag Skill, int32 NewLevel)
{
	if (!HasAuthority())
	{
		return;
	}

	for (FDCSkillLevelEntry& Entry : SkillLevels)
	{
		if (Entry.Skill == Skill)
		{
			if (NewLevel > Entry.Level)
			{
				Entry.Level = NewLevel;
			}
			return;
		}
	}

	if (NewLevel > 0)
	{
		FDCSkillLevelEntry NewEntry;
		NewEntry.Skill = Skill;
		NewEntry.Level = NewLevel;
		SkillLevels.Add(NewEntry);
	}
}
