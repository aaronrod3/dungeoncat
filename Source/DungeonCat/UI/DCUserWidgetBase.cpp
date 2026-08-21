// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCUserWidgetBase.h"
#include "DCPlayerCharacter.h"
#include "GAS/DCAttributeSet.h"

ADCPlayerCharacter* UDCUserWidgetBase::GetDCPlayerCharacter() const
{
	return Cast<ADCPlayerCharacter>(GetOwningPlayerPawn());
}

UDCAttributeSet* UDCUserWidgetBase::GetDCAttributeSet() const
{
	if (ADCPlayerCharacter* Character = GetDCPlayerCharacter())
	{
		return Character->GetDCAttributeSet();
	}
	return nullptr;
}
