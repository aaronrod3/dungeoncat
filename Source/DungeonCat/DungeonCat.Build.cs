// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DungeonCat : ModuleRules
{
	public DungeonCat(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"DungeonCat",
			"DungeonCat/GAS",
			"DungeonCat/Enemy",
			"DungeonCat/Combat",
			"DungeonCat/Interaction",
			"DungeonCat/Items",
			"DungeonCat/Variant_Platforming",
			"DungeonCat/Variant_Platforming/Animation",
			"DungeonCat/Variant_Combat",
			"DungeonCat/Variant_Combat/AI",
			"DungeonCat/Variant_Combat/Animation",
			"DungeonCat/Variant_Combat/Gameplay",
			"DungeonCat/Variant_Combat/Interfaces",
			"DungeonCat/Variant_Combat/UI",
			"DungeonCat/Variant_SideScrolling",
			"DungeonCat/Variant_SideScrolling/AI",
			"DungeonCat/Variant_SideScrolling/Gameplay",
			"DungeonCat/Variant_SideScrolling/Interfaces",
			"DungeonCat/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
