// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCEnemyConfig.h"

FPrimaryAssetId UDCEnemyConfig::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("DCEnemyConfig"), GetFName());
}
