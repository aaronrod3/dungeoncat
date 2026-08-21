// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCLockableContainerComponent.h"
#include "DCPlayerCharacter.h"
#include "DCPlayerState.h"

void UDCLockableContainerComponent::OnInteract_Implementation(ADCPlayerCharacter* Interactor)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !bIsInteractable || !Interactor)
	{
		return;
	}

	const ADCPlayerState* DCPlayerState = Interactor->GetDCPlayerState();
	const int32 ActualLevel = DCPlayerState ? DCPlayerState->GetSkillLevel(RequiredSkill) : 0;

	if (ActualLevel < RequiredSkillLevel)
	{
		// Still locked - no-op. A "you need Nimble Paws 2" feedback prompt is future UI work, not
		// simulated here.
		return;
	}

	Super::OnInteract_Implementation(Interactor);
	OnUnlocked(Interactor);
}
