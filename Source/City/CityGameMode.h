// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "CityGameMode.generated.h"

UCLASS(minimalapi)
class ACityGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACityGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UClass* pawn;
};



