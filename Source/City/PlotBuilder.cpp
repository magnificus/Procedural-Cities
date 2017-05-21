// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "PlotBuilder.h"



// Sets default values
APlotBuilder::APlotBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void APlotBuilder::BeginPlay()
{
	Super::BeginPlay();
	
}


TArray<FHousePolygon> APlotBuilder::generateHousePolygons(FPlotPolygon p, TArray<FPolygon> others, int maxFloors, int minFloors) {
	TArray<FHousePolygon> housePolygons;

	float maxArea = 10000.0f;
	float minArea = 800.0f;

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
			r.height = randFloat() * (maxFloors - minFloors) + minFloors;
			r.type = randFloat() < 0.5 ? RoomType::office : RoomType::apartment;

			float area = r.getArea();
			UE_LOG(LogTemp, Log, TEXT("area of new house polygon: %f"), area);

			//if (area > minArea) {
			housePolygons.Add(r);
			others.Add(r);
		}

	}
	else {
		// wander along the line and place adjacent houses on the curve

		float minLen = 3500;
		float minWidth = 3500;

		float maxLen = 8000;
		float maxWidth = 8000;

		int next = 1;
		FVector currPos = p.points[0];
		FVector prev1 = FVector(0.0f, 0.0f, 0.0f);
		FVector prevTan = FVector(0.0f, 0.0f, 0.0f);
		while (next < p.points.Num()) {
			if (FVector::Dist(p.points[next], currPos) < minLen) {
				currPos = p.points[next];
				next++;
				prev1 = FVector(0.0f, 0.0f, 0.0f);
				prevTan = FVector(0.0f, 0.0f, 0.0f);
				continue;
			}
			FHousePolygon fh;
			FVector tangent1 = p.points[next] - currPos;
			tangent1.Normalize();
			FVector tangent2 = FRotator(0, p.buildLeft ? 90 : 270, 0).RotateVector(tangent1);

			FPolygon pol;
			float len = std::min(FVector::Dist(p.points[next], currPos), randFloat()*(maxLen - minLen) + minLen);
			float width = randFloat()*(maxWidth - minWidth) + minWidth;
			if (prev1.X != 0.0f) {
				pol.points.Add(prev1 + prevTan * width);
				pol.points.Add(prev1 );
			}
			else {
				pol.points.Add(currPos + width*tangent2);
				pol.points.Add(currPos);
			}
			pol.points.Add(currPos + len*tangent1);
			pol.points.Add(currPos + len*tangent1 + width*tangent2);
			FVector first = pol.points[0];
			pol.points.Add(first);


			//fh.windows.Add(2);
			//fh.windows.Add(4);
			FPolygon tmp;
 			if (!testCollision(pol, others, 500, tmp)) {
				fh.points = pol.points;
				fh.population = 1.0;
				fh.height = randFloat() * (maxFloors - minFloors) + minFloors;
				fh.housePosition = pol.getCenter();
				fh.type = randFloat() < 0.5 ? RoomType::office : RoomType::apartment;

				fh.entrances.Add(2);
				for (int i = 1; i < fh.points.Num(); i++) {
					fh.windows.Add(i);
				}
				others.Add(fh);
				housePolygons.Add(fh);
				FVector tangent = pol.points[2] - pol.points[1];
				tangent.Normalize();
				prev1 = pol.points[2] + tangent;
				prevTan = pol.points[3] - pol.points[2];
				prevTan.Normalize();

			}


			currPos += len*tangent1;

		}

	}
	return housePolygons;

}

FPolygon APlotBuilder::generateSidewalkPolygon(FPlotPolygon p, float offsetSize) {
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
			polygon.points.Add(p.points[1] + offset);
		}
		else {
			FVector last = p.points[p.points.Num() - 1];
			polygon.points.Add(last);
			polygon.points.Add(last);

		}
	return polygon;
}


// Called every frame
void APlotBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

