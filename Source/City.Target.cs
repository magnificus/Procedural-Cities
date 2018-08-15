// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class CityTarget : TargetRules
{
	public CityTarget(TargetInfo Target) : base (Target)
	{
		Type = TargetType.Game;

        ExtraModuleNames.Add("City");
	}
}
