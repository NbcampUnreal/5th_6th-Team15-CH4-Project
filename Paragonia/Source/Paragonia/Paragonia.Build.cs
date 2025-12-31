// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Paragonia : ModuleRules
{
	public Paragonia(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
            "AIModule",
            "UMG",
            "Slate",
			"SlateCore",

			// GAS
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",

			// NetWork
            "Sockets",
			"Networking",
			"NetCore",

			// Navigation
			"NavigationSystem",

			// State Tree
			"StateTreeModule",
            "GameplayStateTreeModule",
			
			"Paper2D",

			// OSS
			"OnlineSubsystem",
            "OnlineSubsystemUtils",
            "OnlineSubsystemEOS",
        });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        PublicIncludePaths.AddRange(new string[] {
            "Paragonia",
        });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
