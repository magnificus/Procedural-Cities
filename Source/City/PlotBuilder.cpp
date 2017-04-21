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

bool testCollision(TArray<FPolygon> &polygons, FPolygon &plot, FPolygon &pol) {
	FVector res = intersection(pol, plot);
	if (res.X != 0.0f)
		return true;

	for (FPolygon p2 : polygons) {
		res = intersection(pol, p2);
		if (res.X != 0.0f)
			return true;
	}
	return false;
}

TArray<FHousePolygon> APlotBuilder::getHousePolygons(FPlotPolygon p) {
	TArray<FHousePolygon> housePolygons;

	if (!p.open) {
		float offsetTowardCenter = 1000;
		FHousePolygon fh;
		fh.points = p.points;
		fh.population = p.population;
		fh.type = p.type;
		FVector center = p.getCenter();
		fh.housePosition = center;
		fh.height = randFloat() * 
			15000 + 3000;
		float area = p.getArea();
		UE_LOG(LogTemp, Log, TEXT("area of new polygon: %f"), area);

		//if (area > minArea) {
			housePolygons.Add(fh);
		//}
	}
	else {
		// TODO fix open polygon houses
		return housePolygons;
		// just have to make sure the buildings overlap each other or the outsides of the plot
		TArray<FPolygon> placed;
		for (int i = 1; i < p.points.Num(); i++) {
			// one house per segment
			FHousePolygon fh;
			FVector toRotate = p.points[i] - p.points[i - 1];
			toRotate.Normalize();
			FVector tangent = FRotator(0, p.buildLeft ? 90 : 270, 0).RotateVector(toRotate);



			FPolygon pol;
			float offset = (randFloat() * 10000 + 4000);
			pol.points.Add(p.points[i - 1] + tangent *10);
			pol.points.Add(p.points[i] + tangent *10);
			pol.points.Add(p.points[i] + tangent * offset);
			pol.points.Add(p.points[i - 1] + tangent * offset);
			pol.points.Add(p.points[i - 1] + tangent * 10);
			FVector center = p.getCenter();


			if (!testCollision(placed, p, pol)) {
				fh.points = pol.points;
				fh.population = 1.0;
				fh.height = randFloat() * 6000 + 4000;
				fh.housePosition = center;
				placed.Add(pol);
				housePolygons.Add(fh);
			}
		}
	}
	return housePolygons;

	
}

//void APlotBuilder::BuildPlot(FPlotPolygon p) {
//	//for (AHouseBuilder* h :houses)
//	//{
//	//	h->Destroy();
//	//	delete(h);
//	//}
//
//	//houses.Empty();
//
//	for (int i = 1; i < p.f.points.Num(); i++) {
//		// one house per segment
//		FVector location;
//		FActorSpawnParameters spawnInfo;
//		location = (p.f.points[i] - p.f.points[i - 1]) / 2 + p.f.points[i - 1];
//		FVector offset =  FRotator(0, p.f.buildLeft ? 90 : 270, 0).RotateVector(FVector(2000, 0, 0));
//		
//		AHouseBuilder* h = GetWorld()->SpawnActor<AHouseBuilder>(location, FRotator(0, 0, 0), spawnInfo);
//		FHousePolygon fh;
//		FPolygon pol;
//		pol.points.Add(p.f.points[i - 1]);
//		pol.points.Add(p.f.points[i]);
//		pol.points.Add(p.f.points[i] + offset);
//		pol.points.Add(p.f.points[i - 1] + offset);
//		pol.center = getCenter(pol);
//		fh.polygon = pol;
//		fh.population = 1.0;
//		h->placeHouse(fh);
//		houses.Add(h);
//		//fh.
//		//h->placeHouse()
//	}
//	//if (p.f.open) {
//
//	//}
//
//}

// Called every frame
void APlotBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

