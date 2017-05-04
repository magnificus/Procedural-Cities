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

	float maxArea = 1000.0f;
	float minArea = 5.0f;

	if (!p.open) {


		FHousePolygon original;
		original.points = p.points;
		original.buildLeft = p.buildLeft;
		original.open = p.open;
		original.population = p.population;
		original.type = p.type;
		for (int32 i = 1; i < original.points.Num(); i++) {
			original.entrances.Add(i);
			original.windows.Add(i);
		}
		if (FVector::Dist(original.points[0], original.points[original.points.Num()-1]) > 0.1f)
			UE_LOG(LogTemp, Warning, TEXT("END AND BEGINNING NOT CONNECTED IN PLOTBUILDER"));


		TArray<FHousePolygon> refinedPolygons = original.refine(maxArea, minArea);
		for (FHousePolygon r : refinedPolygons) {
			if (r.height != 50)
				r.height = randFloat() * (maxFloors - minFloors) + minFloors;
			float area = r.getArea();
			UE_LOG(LogTemp, Log, TEXT("area of new polygon: %f"), area);

			//if (area > minArea) {
			housePolygons.Add(r);
			others.Add(r);
		}

	}
	else {
		// wander along the line and place adjacent houses on the curve

		float minLen = 3000;
		float minWidth = 3000;

		float maxLen = 9000;
		float maxWidth = 9000;

		int next = 1;
		FVector currPos = p.points[0];
		while (next < p.points.Num()) {
			if (FVector::Dist(p.points[next], currPos) < minLen) {
				p.points[next];
				next++;
				continue;
			}
			FHousePolygon fh;
			FVector tangent1 = p.points[next] - currPos;
			tangent1.Normalize();
			FVector tangent2 = FRotator(0, p.buildLeft ? 90 : 270, 0).RotateVector(tangent1);

			FPolygon pol;
			float len = std::min(FVector::Dist(p.points[next], currPos), randFloat()*(maxLen - minLen) + minLen);
			float width = randFloat()*(maxWidth - minWidth) + minWidth;
			pol.points.Add(currPos + width*tangent2);
			pol.points.Add(currPos);
			pol.points.Add(currPos + len*tangent1);
			pol.points.Add(currPos + len*tangent1 + width*tangent2);
			pol.points.Add(currPos + width*tangent2);

			fh.entrances.Add(2);
			fh.windows.Add(2);
			fh.windows.Add(4);

			if (!testCollision(others, pol)) {
				fh.points = pol.points;
				fh.population = 1.0;
				fh.height = randFloat() * (maxFloors - minFloors) + minFloors;
				fh.housePosition = pol.getCenter();
				others.Add(fh);
				housePolygons.Add(fh);
			}

			currPos += len*tangent1;
			if (FVector::DistSquared(currPos, p.points[next]) < 20000) {
				currPos = p.points[next];
				next++;

			}

		}

	}
	return housePolygons;

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

//TArray<FHousePolygon> APlotBuilder::getHousePolygons(FPlotPolygon p) {
//	TArray<FHousePolygon> housePolygons;
//
//	if (!p.open) {
//		FHousePolygon fh;
//		fh.points = p.points;
//		fh.population = p.population;
//		fh.type = p.type;
//		FVector center = p.getCenter();
//		fh.housePosition = center;
//		fh.height = randFloat() * (maxFloors - minFloors) + minFloors;
//		float area = p.getArea();
//		UE_LOG(LogTemp, Log, TEXT("area of new polygon: %f"), area);
//
//		//if (area > minArea) {
//			housePolygons.Add(fh);
//		//}
//	}
//	else {
//		// TODO fix open polygon houses
//		// just have to make sure the buildings overlap each other or the outsides of the plot
//		TArray<FPolygon> placed;
//		placed.Add(p);
//		for (int i = 1; i < p.points.Num(); i++) {
//			// one house per segment
//			FHousePolygon fh;
//			FVector toRotate = p.points[i] - p.points[i - 1];
//			if (toRotate.Size() < 5000)
//				continue;
//
//			toRotate.Normalize();
//			FVector tangent = FRotator(0, p.buildLeft ? 90 : 270, 0).RotateVector(toRotate);
//
//			FPolygon pol;
//			float offset = (randFloat() * 12000 + 2000);
//			pol.points.Add(p.points[i - 1] + tangent *10);
//			pol.points.Add(p.points[i] + tangent *10);
//			pol.points.Add(p.points[i] + tangent * offset);
//			pol.points.Add(p.points[i - 1] + tangent * offset);
//			pol.points.Add(p.points[i - 1] + tangent * 10);
//			FVector center = p.getCenter();
//
//
//			if (!testCollision(placed, pol)) {
//				fh.points = pol.points;
//				fh.population = 1.0;
//				fh.height = randFloat() * (maxFloors - minFloors) + minFloors;
//				fh.housePosition = center;
//				placed.Add(pol);
//				housePolygons.Add(fh);
//			}
//		}
//	}
//	return housePolygons;
//
//	
//}



// Called every frame
void APlotBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

