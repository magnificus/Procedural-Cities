// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "PlotBuilder.h"



// Sets default values
APlotBuilder::APlotBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlotBuilder::BeginPlay()
{
	Super::BeginPlay();
	
}

bool testCollision(TArray<FPolygon> &polygons, FPolygon &pol) {

	for (FPolygon p2 : polygons) {
		FVector res = intersection(pol, p2);
		if (res.X != 0.0f)
			return true;
	}
	return false;
}

TArray<FHousePolygon> APlotBuilder::generateHousePolygons(FPlotPolygon p, TArray<FPolygon> others) {
	TArray<FHousePolygon> housePolygons;

	if (!p.open) {
		FHousePolygon fh;
		fh.points = p.points;
		fh.population = p.population;
		fh.type = p.type;
		FVector center = p.getCenter();
		fh.housePosition = center;
		fh.height = randFloat() * (maxFloors - minFloors) + minFloors;
		float area = p.getArea();
		UE_LOG(LogTemp, Log, TEXT("area of new polygon: %f"), area);

		//if (area > minArea) {
		housePolygons.Add(fh);
		others.Add(fh);
		return housePolygons;
	}
	else {
		//return TArray<FHousePolygon>();
		for (int i = 1; i < p.points.Num(); i++) {
			// one house per segment
			FHousePolygon fh;
			FVector toRotate = p.points[i] - p.points[i - 1];
			toRotate.Normalize();
			FVector tangent = FRotator(0, p.buildLeft ? 90 : 270, 0).RotateVector(toRotate);



			FPolygon pol;
			float offset = (randFloat() * 10000 + 4000);
			pol.points.Add(p.points[i - 1] + tangent * 10);
			pol.points.Add(p.points[i] + tangent * 10);
			pol.points.Add(p.points[i] + tangent * offset);
			pol.points.Add(p.points[i - 1] + tangent * offset);
			pol.points.Add(p.points[i - 1] + tangent * 10);
			FVector center = p.getCenter();


			if (!testCollision(others, pol)) {
				fh.points = pol.points;
				fh.population = 1.0;
				fh.height = randFloat() * (maxFloors - minFloors) + minFloors;
				fh.housePosition = center;
				others.Add(fh);
				housePolygons.Add(fh);
			}
		}
		return housePolygons;

	}

}

FPolygon APlotBuilder::generateSidewalkPolygon(FPlotPolygon p) {
	FPolygon polygon;
	//if (!p.open) {
		FVector center = p.getCenter();
		FPolygon sidewalk;
		for (int i = 1; i < p.points.Num(); i+=1) {
			FVector tangent = p.points[i] - p.points[i - 1];
			tangent.Normalize();
			FVector offset = (p.buildLeft ? FRotator(0, 270, 0) : FRotator(0, 90, 0)).RotateVector(tangent * offsetSize);
			polygon.points.Add(p.points[i - 1] + offset);
			polygon.points.Add(p.points[i] + offset);
		}
		if (!p.open) {
			FVector tangent = p.points[1] - p.points[0];
			tangent.Normalize();
			FVector offset = (p.buildLeft ? FRotator(0, 270, 0) : FRotator(0, 90, 0)).RotateVector(tangent * offsetSize);
			polygon.points.Add(p.points[0] + offset);
		}


	//}
	return polygon;
}

TArray<FHousePolygon> APlotBuilder::getHousePolygons(FPlotPolygon p) {
	TArray<FHousePolygon> housePolygons;

	if (!p.open) {
		FHousePolygon fh;
		fh.points = p.points;
		fh.population = p.population;
		fh.type = p.type;
		FVector center = p.getCenter();
		fh.housePosition = center;
		fh.height = randFloat() * (maxFloors - minFloors) + minFloors;
		float area = p.getArea();
		UE_LOG(LogTemp, Log, TEXT("area of new polygon: %f"), area);

		//if (area > minArea) {
			housePolygons.Add(fh);
		//}
	}
	else {
		// TODO fix open polygon houses
		// just have to make sure the buildings overlap each other or the outsides of the plot
		TArray<FPolygon> placed;
		placed.Add(p);
		for (int i = 1; i < p.points.Num(); i++) {
			// one house per segment
			FHousePolygon fh;
			FVector toRotate = p.points[i] - p.points[i - 1];
			if (toRotate.Size() < 5000)
				continue;

			toRotate.Normalize();
			FVector tangent = FRotator(0, p.buildLeft ? 90 : 270, 0).RotateVector(toRotate);

			FPolygon pol;
			float offset = (randFloat() * 12000 + 2000);
			pol.points.Add(p.points[i - 1] + tangent *10);
			pol.points.Add(p.points[i] + tangent *10);
			pol.points.Add(p.points[i] + tangent * offset);
			pol.points.Add(p.points[i - 1] + tangent * offset);
			pol.points.Add(p.points[i - 1] + tangent * 10);
			FVector center = p.getCenter();


			if (!testCollision(placed, pol)) {
				fh.points = pol.points;
				fh.population = 1.0;
				fh.height = randFloat() * (maxFloors - minFloors) + minFloors;
				fh.housePosition = center;
				placed.Add(pol);
				housePolygons.Add(fh);
			}
		}
	}
	return housePolygons;

	
}



// Called every frame
void APlotBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

