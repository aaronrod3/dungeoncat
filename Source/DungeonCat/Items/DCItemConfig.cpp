// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCItemConfig.h"

FPrimaryAssetId UDCItemConfig::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("DCItemConfig"), GetFName());
}
