// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCInteractableComponent.h"

UDCInteractableComponent::UDCInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDCInteractableComponent::OnInteract_Implementation(ADCPlayerCharacter* Interactor)
{
	OnInteracted.Broadcast(this, Interactor);
}
