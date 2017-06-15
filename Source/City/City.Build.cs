// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class City : ModuleRules
{
	public City(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay","ProceduralMeshComponent","ShaderCore", "RenderCore", "RHI", "RuntimeMeshComponent" });
    }
}
