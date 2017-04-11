// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "HouseBuilder.h"

struct FPolygon;


// Sets default values
AHouseBuilder::AHouseBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}


void AHouseBuilder::placeHouse(FHousePolygon f)
{
	if (f.polygon.buildLeft) {
		RootComponent = StaticMeshComponent;
		FVector Location(0, 0, 0);
		StaticMeshComponent->Mobility = EComponentMobility::Static;
		StaticMeshComponent->SetStaticMesh(Chair1);
	}
}

// Called when the game starts or when spawned
void AHouseBuilder::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHouseBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

