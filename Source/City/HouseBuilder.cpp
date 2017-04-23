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

	TArray<FPolygon> polygons;
	FVector start = outer.points[1];
	FVector end = outer.points[2];

	// sort holes according to dist to starting point so that they can be applied in order
	holes.Sort([start](const FPolygon &p1, const FPolygon &p2) {
		return FVector::DistSquared(p1.points[1], start) < FVector::DistSquared(p2.points[1], start);
	});
	if (holes.Num() > 0) {

		FVector sideTangent = outer.points[1] - outer.points[0];

		FVector attach1 = outer.points[0];
		FVector attach2 = (holes[0].points[0] - outer.points[0]).ProjectOnTo(sideTangent) + outer.points[0];
		FVector attach3 = (holes[0].points[1] - outer.points[0]).ProjectOnTo(sideTangent) + outer.points[0];
		FVector attach4 = outer.points[1];

		FVector tangentUp = outer.points[3] - outer.points[0];
		FVector tangentDown = outer.points[2] - outer.points[1];
		//tangentUp.Normalize();
		//tangentDown.Normalize();

		int count = 0;
		for (FPolygon p : holes) {
			//UE_LOG(LogTemp, Warning, TEXT("attach1 %s, %i"), *attach1.ToString(), count++);
			FPolygon p1 = FPolygon();
			p1.points.Add(attach1);
			p1.points.Add(p.points[3]);

			p1.points.Add((p.points[3] - outer.points[0]).ProjectOnTo(tangentUp) + outer.points[0]);

			FPolygon p2 = FPolygon();
			p2.points.Add(attach1);
			p2.points.Add(attach2);
			p2.points.Add(p.points[3]);

			FPolygon p3 = FPolygon();
			p3.points.Add(attach2);
			p3.points.Add(p.points[1]);
			p3.points.Add(p.points[0]);

			FPolygon p4 = FPolygon();
			p4.points.Add(attach2);
			p4.points.Add(attach3);
			p4.points.Add(p.points[1]);

			FPolygon p5 = FPolygon();
			p5.points.Add(attach3);
			p5.points.Add((p.points[2] - outer.points[1]).ProjectOnTo(tangentDown) + outer.points[1]);
			p5.points.Add(p.points[2]);

			FPolygon p6 = FPolygon();
			p6.points.Add(attach3);
			p6.points.Add(attach4);
			p6.points.Add((p.points[2] - outer.points[1]).ProjectOnTo(tangentDown) + outer.points[1]);

			polygons.Add(p1);
			polygons.Add(p2);
			polygons.Add(p3);
			polygons.Add(p4);
			polygons.Add(p5);
			polygons.Add(p6);

			attach1 = p1.points[2];
			attach2 = p1.points[1];
			attach3 = p5.points[2];
			attach4 = p5.points[1];
		}
		// attach to end of outer
		//UE_LOG(LogTemp, Warning, TEXT("attach1 %s, %i"), *attach1.ToString(), count++);

		FPolygon p7;
		p7.points.Add(attach1);
		p7.points.Add(attach4);
		p7.points.Add(outer.points[2]);

		FPolygon p8;
		p8.points.Add(attach1);
		p8.points.Add(outer.points[2]);
		p8.points.Add(outer.points[3]);

		polygons.Add(p7);
		polygons.Add(p8);
	}
	else {
		polygons.Add(outer);
	}



	return polygons;
}

// just the sides of the house with a hole for a single door
TArray<FPolygon> getGroundPolygons(FHousePolygon f, float floorHeight, float doorHeight, float doorWidth) {

	TArray<FPolygon> polygons;

	//int doorLoc = FMath::FloorToInt(randFloat() * (f.points.Num() - 1) + 1);
	int doorLoc = 1;
	FVector side = f.points[doorLoc] - f.points[doorLoc - 1];
	float sideLen = side.Size();
	side.Normalize();

	float distToDoor = (sideLen - doorWidth) / 2;
	TArray<FPolygon> holes;
	FPolygon doorPolygon;
	doorPolygon.points.Add(f.points[doorLoc - 1] + side*distToDoor + FVector(0, 0, doorHeight));
	doorPolygon.points.Add(f.points[doorLoc - 1] + side*distToDoor);// + FVector(0, 0, 100));
	doorPolygon.points.Add(f.points[doorLoc] - side*distToDoor);// + FVector(0, 0, 100));
	doorPolygon.points.Add(f.points[doorLoc] - side*distToDoor + FVector(0, 0, doorHeight));
	holes.Add(doorPolygon);

	FPolygon outer;
	outer.points.Add(f.points[doorLoc - 1] + FVector(0, 0, floorHeight));
	outer.points.Add(f.points[doorLoc - 1] + FVector(0, 0, 0));
	outer.points.Add(f.points[doorLoc] + FVector(0, 0, 0));
	outer.points.Add(f.points[doorLoc] + FVector(0, 0, floorHeight));

	polygons.Append(getSideWithHoles(outer, holes));

	for (int i = 1; i < f.points.Num(); i++) {
		if (i == doorLoc)
			continue;
		FPolygon newP;

		newP.points.Add(f.points[i - 1] + FVector(0, 0, floorHeight));
		newP.points.Add(f.points[i - 1]);
		newP.points.Add(f.points[i]);
		newP.points.Add(f.points[i] + FVector(0, 0, floorHeight));
		polygons.Add(newP);

	}

	return polygons;
}

TArray<FPolygon> getFloorPolygons(FHousePolygon &f, float floorBegin, float floorHeight, float windowDensity, float windowBegin, float windowWidth, float windowHeight) {
	TArray<FPolygon> polygons;

	FPolygon floor;

	for (FVector f2 : f.points) {
		floor.points.Add(f2 + FVector(0,0, floorBegin));
	}

	polygons.Add(floor);
	for (int i = 1; i < f.points.Num(); i++) {
		FPolygon outer;
		outer.points.Add(f.points[i - 1] + FVector(0, 0, floorHeight + floorBegin));
		outer.points.Add(f.points[i - 1] + FVector(0, 0, floorBegin));
		outer.points.Add(f.points[i] + FVector(0, 0, floorBegin));
		outer.points.Add(f.points[i] + FVector(0, 0, floorHeight + floorBegin));

		FVector tangent = f.points[i] - f.points[i - 1];
		float len = tangent.Size();
		tangent.Normalize();

		TArray<FPolygon> windows;
		int spaces = FMath::FloorToInt(windowDensity * len) + 1;
		float jumpLen = len / (float) spaces;

		for (int j = 1; j < spaces; j++) {
			FPolygon currWindow;
			FVector p1 = f.points[i - 1] + tangent * j * jumpLen + FVector(0, 0, floorBegin + windowBegin + windowHeight) - (tangent * windowWidth / 2);
			FVector p2 = p1 - FVector(0, 0, windowHeight);
			FVector p3 = p2 + tangent * windowWidth;
			FVector p4 = p3 + FVector(0, 0, windowHeight);

			currWindow.points.Add(p1);
			currWindow.points.Add(p2);
			currWindow.points.Add(p3);
			currWindow.points.Add(p4);

			windows.Add(currWindow);

		}

		TArray<FPolygon> pols = getSideWithHoles(outer, windows);
		for (FPolygon p : pols) {
			int count = 0;
			for (FVector f : p.points) {
				UE_LOG(LogTemp, Warning, TEXT("%s %i"), *f.ToString(), count++);
			}

		}

		polygons.Append(pols);

	}
	return polygons;
}
TArray<FPolygon> AHouseBuilder::getHousePolygons(FHousePolygon f, int floors, float floorHeight)
{
	TArray<FPolygon> toReturn = getGroundPolygons(f, floorHeight, 400, 200);

	float wDens = randFloat() * 0.0010 + 0.0005;

	float wWidth = randFloat() * 500 + 500;
	float wHeight = randFloat() * 400 + 200;

	float currHeight = floorHeight;
	for (int i = 1; i < floors; i++) {
		toReturn.Append(getFloorPolygons(f, currHeight, floorHeight, wDens, floorHeight / 3, wWidth, wHeight));
		currHeight += floorHeight;
	}
	FPolygon roof;
	for (FVector f2 : f.points) {
		roof.points.Add(f2 + FVector(0, 0, currHeight));
	}
	toReturn.Add(roof);
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

