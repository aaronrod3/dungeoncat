// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCItemPickupComponent.h"
#include "DCItemConfig.h"
#include "DCPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

UDCItemPickupComponent::UDCItemPickupComponent()
{
}

void UDCItemPickupComponent::OnInteract_Implementation(ADCPlayerCharacter* Interactor)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !bIsInteractable || !Item || !Interactor)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = Interactor->GetAbilitySystemComponent())
	{
		for (const TSubclassOf<UGameplayEffect>& EffectClass : Item->GrantedEffects)
		{
			if (!EffectClass)
			{
				continue;
			}

			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(this);
			const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(EffectClass, 1.f, Context);
			if (Spec.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}

	// Guard against a double-pickup race between two co-op players reaching this in the same tick -
	// disable before Destroy rather than relying on Destroy's own (not instant) teardown.
	bIsInteractable = false;

	Super::OnInteract_Implementation(Interactor);

	GetOwner()->Destroy();
}
