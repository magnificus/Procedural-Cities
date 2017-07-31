// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "City.h"
#include "CityGameMode.h"
//#include "CityHUD.h"
#include "CityCharacter.h"

ACityGameMode::ACityGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FObjectFinder<UBlueprint> Blueprint(TEXT("Blueprint'/Game/MyCityCharacter.MyCityCharacter'"));
	//if (Blueprint.Object) {
	//	DefaultPawnClass = (UClass*)Blueprint.Object->GeneratedClass;
	//}

	// use our custom HUD class
	//HUDClass = ACityHUD::StaticClass();
}
