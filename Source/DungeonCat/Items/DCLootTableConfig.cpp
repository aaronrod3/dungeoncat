// Copyright Epic Games, Inc. All Rights Reserved.

#include "DCLootTableConfig.h"

FPrimaryAssetId UDCLootTableConfig::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("DCLootTableConfig"), GetFName());
}

TArray<UDCItemConfig*> UDCLootTableConfig::RollLoot(int32 PartySize, FRandomStream& Stream) const
{
	TArray<UDCItemConfig*> Results;

	const int32 GuaranteedRolls = bScaleRollsWithPartySize
		? BaseGuaranteedRolls * FMath::Max(PartySize, 1)
		: BaseGuaranteedRolls;

	int32 TotalRolls = GuaranteedRolls;
	if (BonusRollChance > 0.f && Stream.FRand() < BonusRollChance)
	{
		TotalRolls += 1;
	}

	for (int32 i = 0; i < TotalRolls; ++i)
	{
		const EDCItemRarity Rarity = RollRarity(Stream);
		if (UDCItemConfig* Item = RollItemOfRarity(Rarity, Stream))
		{
			Results.Add(Item);
		}
	}

	return Results;
}

EDCItemRarity UDCLootTableConfig::RollRarity(FRandomStream& Stream) const
{
	const float Roll = Stream.FRand();

	float Cumulative = CommonChance;
	if (Roll < Cumulative)
	{
		return EDCItemRarity::Common;
	}

	Cumulative += UncommonChance;
	if (Roll < Cumulative)
	{
		return EDCItemRarity::Uncommon;
	}

	Cumulative += RareChance;
	if (Roll < Cumulative)
	{
		return EDCItemRarity::Rare;
	}

	return EDCItemRarity::Epic;
}

UDCItemConfig* UDCLootTableConfig::RollItemOfRarity(EDCItemRarity Rarity, FRandomStream& Stream) const
{
	TArray<const FDCLootTableEntry*> Candidates;
	float TotalWeight = 0.f;

	for (const FDCLootTableEntry& Entry : Entries)
	{
		if (Entry.Item && Entry.Item->Rarity == Rarity && Entry.Weight > 0.f)
		{
			Candidates.Add(&Entry);
			TotalWeight += Entry.Weight;
		}
	}

	if (Candidates.Num() == 0 || TotalWeight <= 0.f)
	{
		return nullptr;
	}

	float Pick = Stream.FRandRange(0.f, TotalWeight);
	for (const FDCLootTableEntry* Candidate : Candidates)
	{
		Pick -= Candidate->Weight;
		if (Pick <= 0.f)
		{
			return Candidate->Item;
		}
	}

	// Floating-point edge case (Pick landed exactly at TotalWeight) - last candidate wins rather than
	// returning nullptr and silently dropping a roll that should have produced something.
	return Candidates.Last()->Item;
}
