// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class c47 : ModuleRules
{
	public c47(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Niagara", "Json", "JsonUtilities" });
	
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd", "EditorFramework"
			});
		}
	}
}
