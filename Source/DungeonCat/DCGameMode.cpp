// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCGameMode.h"
#include "DCPlayerCharacter.h"
#include "DCPlayerState.h"

ADCGameMode::ADCGameMode()
{
	DefaultPawnClass = ADCPlayerCharacter::StaticClass();
	PlayerStateClass = ADCPlayerState::StaticClass();
}
