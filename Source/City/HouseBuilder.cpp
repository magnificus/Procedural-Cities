// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "HouseBuilder.h"

struct FPolygon;


// Sets default values
AHouseBuilder::AHouseBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	placeHolderHouseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(placeHolderHouseMesh);



	FString pathName = "StaticMesh'/Game/Geometry/Meshes/1M_Cube.1M_Cube'";
	ConstructorHelpers::FObjectFinder<UStaticMesh> mySSMesh12412(*pathName);
	placeHolderHouseMesh->SetStaticMesh(mySSMesh12412.Object);
	placeHolderHouseMesh->SetMobility(EComponentMobility::Static);


	pathName = "StaticMesh'/Game/StarterContent/Props/SM_PillarFrame.SM_PillarFrame'";
	ConstructorHelpers::FObjectFinder<UStaticMesh> buildingMesh(*pathName);
	meshPolygon = buildingMesh.Object;

}
/*
holes are assumed to be of a certain structure, just four points upper left -> lower left -> lower right -> upper right, no window is allowed to overlap another
0---3
|	|
1---2
*/
TArray<FPolygon> getSideWithHoles(FPolygon outer, TArray<FPolygon> holes) {

	TArray<FPolygon> polygons = TArray<FPolygon>();
	FVector start = outer.points[1];
	FVector end = outer.points[2];

	// sort holes according to dist to starting point so that they can be applied in order
	holes.Sort([start](const FPolygon &p1, const FPolygon &p2) {
		return FVector::DistSquared(p1.points[1], start) > FVector::DistSquared(p2.points[2], start);
	});
	FVector attach1 = outer.points[0];
	FVector attach2 = attach1;
	attach2.Z = holes[0].points[0].Z;
	FVector attach3 = attach1;
	attach3.Z = holes[0].points[1].Z;
	FVector attach4 = attach1;
	attach4.Z = outer.points[1].Z;
	for (FPolygon p : holes) {
		FPolygon p1;
		p1.points.Add(attach1);
		p1.points.Add(p.points[3]);
	}


	return TArray<FPolygon>();
}

TArray<FPolygon> getGroundPolygons(FHousePolygon &f, float floorHeight, float doorHeight, float doorWidth) {
	int doorLoc = FMath::FloorToInt(randFloat() * (f.polygon.points.Num() - 1) + 1);
	FVector side = f.polygon.points[doorLoc] - f.polygon.points[doorLoc - 1];
	return TArray<FPolygon>();
}

TArray<FPolygon> getFloorPolygons(FHousePolygon &f, float floorHeight) {
	return TArray<FPolygon>();
}
TArray<FPolygon> AHouseBuilder::getHousePolygons(FHousePolygon &f, int floors, float floorHeight)
{
	TArray<FPolygon> toReturn;
	return toReturn;
	// we have the outline of the house, have to place levels, start with bottom & door



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

