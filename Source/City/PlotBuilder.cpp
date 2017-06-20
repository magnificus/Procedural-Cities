// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "NoiseSingleton.h"
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

TArray<FMetaPolygon> APlotBuilder::sanityCheck(TArray<FMetaPolygon> plots, TArray<FPolygon> others) {
	TArray<FMetaPolygon> added;
	for (FMetaPolygon p : plots) {
		bool shouldAdd = true;
		//for (FPolygon o : others) {

		//}
		if (shouldAdd) {
			for (FMetaPolygon a : added) {
				if (testCollision(p, a, -1000)) {
					shouldAdd = false;
					break;
				}
			}
		}
		if (shouldAdd)
			added.Add(p);
	}
	return added;
}

FHousePolygon getRandomModel(float minSize, float maxSize, int minFloors, int maxFloors, float noiseScale, RoomType type, FRandomStream stream) {
	FHousePolygon pol;
	float xLen = stream.FRandRange(minSize, maxSize);
	float yLen = stream.FRandRange(minSize, maxSize);
	FVector tangent = FVector(stream.FRand(), stream.FRand(), 0);
	tangent.Normalize();
	FVector normal = FRotator(0, 90, 0).RotateVector(tangent);
	pol.points.Add(FVector(0, 0, 0));
	pol.points.Add(xLen * tangent);
	pol.points.Add(xLen * tangent + normal * yLen);
	pol.points.Add(normal*yLen);
	pol.points.Add(FVector(0, 0, 0));
	for (int i = 1; i < pol.points.Num(); i++) {
		pol.entrances.Add(i);
		pol.windows.Add(i);
		pol.open = false;
	}
	pol.housePosition = pol.getCenter();
	pol.height = FMath::FRandRange(minFloors, maxFloors * NoiseSingleton::getInstance()->noise(pol.housePosition.X, pol.housePosition.Y, noiseScale));//randFloat() * (maxFloors - minFloors) + minFloors;
	//if (NoiseSingleton::getInstance()->noise((pol.housePosition.X + noiseXOffset)*noiseScale, (pol.housePosition.Y + noiseYOffset)*noiseScale) > 0.7) {
	//	pol.height *= 2;
	//}
	pol.type = type;
	pol.offset(-pol.getCenter());
	return pol;
}


FPlotInfo APlotBuilder::generateHousePolygons(FPlotPolygon p, int maxFloors, int minFloors, float noiseScale) {
	FPlotInfo info;
	FVector cen = p.getCenter();
	FRandomStream stream(cen.X * 1000 + cen.Y);

	float maxArea = 4500.0f;
	float minArea = 1200.0f;
	if (!p.open) {
		FHousePolygon original;
		original.points = p.points;
		original.checkOrientation();
		original.buildLeft = true;
		original.open = false;
		original.population = p.population;
		original.type = p.type;
		original.housePosition = original.getCenter();
		for (int32 i = 1; i < original.points.Num(); i++) {
			original.entrances.Add(i);
			original.windows.Add(i);
		}
		FVector center = p.getCenter();
		p.type = NoiseSingleton::getInstance()->noise((center.X + noiseXOffset*2)*noiseScale*2, (center.Y + noiseYOffset*2)*noiseScale*2) < 0 ? RoomType::office : RoomType::apartment;

		bool normalPlacement = !(p.getArea() > 5500 && stream.FRand() < 0.2);
		if (!normalPlacement) {
			// create a special plot with several identical houses placed around a green area, this happens in real cities sometimes
			FHousePolygon model = getRandomModel(3500,6000, minFloors, maxFloors, noiseScale, p.type, stream);
			model.checkOrientation();
			model.canBeModified = false;
			FPolygon shaft = AHouseBuilder::getShaftHolePolygon(model);
			for (int i = 0; i < 3; i++) {
				TArray<FSimplePlot> temp;
				AHouseBuilder::makeInteresting(model, temp, shaft, stream);
			}

			TArray<FPolygon> placed;
			for (int i = 0; i < 20; i++) {
				FHousePolygon newH = model;
				newH.rotate(FRotator(0, stream.FRandRange(0, 360), 0));
				newH.offset(p.getRandomPoint(true, 2000));
				newH.housePosition = newH.getCenter();
				if (!testCollision(newH, placed, 0, p)) {
					info.houses.Add(newH);
					placed.Add(newH);
				}
			}
			if (placed.Num() < 1) {
				normalPlacement = true;
			}
			else {
					FSimplePlot fs;

					//fm.points.RemoveAt(fm.points.Num() - 1);
					fs.pol = p;
					fs.pol.offset(FVector(0, 0, 30));
					fs.type = p.type == RoomType::apartment ? SimplePlotType::green : SimplePlotType::asphalt;
					fs.decorate(placed);
					info.leftovers.Add(fs);
				//TArray<FPolygon> holesWithoutLastPoint;
				//for (FPolygon h :placed) {
				//	h.points.RemoveAt(h.points.Num() - 1);
				//	h.reverse();
				//	holesWithoutLastPoint.Add(h);
				//}
				//cp.points.RemoveAt(cp.points.Num() - 1);
				//cp.reverse();
				//FPolygon cp = p;
				//cp.points.RemoveAt(cp.points.Num() - 1);
				//TArray<FMaterialPolygon> results = ARoomBuilder::getSideWithHoles(cp, holesWithoutLastPoint, p.type == RoomType::apartment ? PolygonType::green : PolygonType::concrete);

				//cp.reverse();
				//holesWithoutLastPoint.Empty();
				//for (FPolygon h : placed) {
				//	h.points.RemoveAt(h.points.Num() - 1);
				//	//h.reverse();
				//	holesWithoutLastPoint.Add(h);
				//}
				//results.Append(ARoomBuilder::getSideWithHoles(cp, holesWithoutLastPoint, p.type == RoomType::apartment ? PolygonType::green : PolygonType::concrete));
				//for (FMaterialPolygon fm : results) {
				//	FSimplePlot fs;

				//	//fm.points.RemoveAt(fm.points.Num() - 1);
				//	fs.pol = fm;
				//	fs.pol.offset(FVector(0, 0, 30));
				//	fs.type = fm.type == PolygonType::concrete ? SimplePlotType::asphalt : SimplePlotType::green;
				//	fs.decorate();
				//	info.leftovers.Add(fs);

				//}
			}
		}
		if (normalPlacement) {
			TArray<FHousePolygon> refinedPolygons = original.refine(maxArea, 0, 0);
			for (FHousePolygon r : refinedPolygons) {
				//r.height = randFloat() * (maxFloors - minFloors) + minFloors;
				r.housePosition = r.getCenter();
				r.height = FMath::FRandRange(minFloors, maxFloors * NoiseSingleton::getInstance()->noise(r.housePosition.X, r.housePosition.Y, noiseScale));//randFloat() * (maxFloors - minFloors) + minFloors;

				//if (raw_noise_2d((r.housePosition.X + noiseXOffset)*noiseScale*10, (r.housePosition.Y + noiseYOffset)*noiseScale*10) > 0.7) {
				//	r.height *= 2;
				//}
				r.type = p.type;
				r.simplePlotType = r.type == RoomType::office ? SimplePlotType::asphalt : SimplePlotType::green;

				float area = r.getArea();
				//UE_LOG(LogTemp, Log, TEXT("area of new house polygon: %f"), area);

				if (area < minArea || area > maxArea) {

					//if (r.getIsClockwise())
					//	r.reverse();
					//r.points.RemoveAt(r.points.Num() - 1);
					FSimplePlot fs;
					fs.pol = r;
					fs.pol.offset(FVector(0, 0, 30));
					fs.type = stream.FRand() < 0.5? SimplePlotType::green : SimplePlotType::asphalt;
					fs.decorate();
					info.leftovers.Add(fs);
				}
				else {
					r.checkOrientation();
					info.houses.Add(r);
				}
			}
		}

	}
	return info;

}


TArray<FMaterialPolygon> APlotBuilder::getSideWalkPolygons(FPlotPolygon p, float width) {
	TArray<FMaterialPolygon> pols;
	FVector prevP1 = FVector(0,0,0);
	FVector prevP2 = FVector(0, 0, 0);

	float endWidth = 40;
	float endHeight = 40;
	for (int i = 1; i < p.points.Num(); i++) {
		// add the straight part
		FMaterialPolygon current;
		current.type = PolygonType::concrete;
		FVector normal = getNormal(p.points[i - 1], p.points[i], !p.buildLeft);
		normal.Normalize();
		current.points.Add(p.points[i - 1]);
		current.points.Add(p.points[i]);
		current.points.Add(p.points[i] + width*normal);
		current.points.Add(p.points[i - 1] + width*normal);
		current.points.Add(p.points[i - 1]);
		current.offset(FVector(0, 0, 30));

		FMaterialPolygon currentOuterLine;
		currentOuterLine.type = PolygonType::concrete;
		currentOuterLine.points.Add(p.points[i] + width*normal);
		currentOuterLine.points.Add(p.points[i] + (width + endWidth)*normal);
		currentOuterLine.points.Add(p.points[i - 1] + (width+endWidth)*normal);
		currentOuterLine.points.Add(p.points[i - 1] + width*normal);
		currentOuterLine.points.Add(p.points[i] + width*normal);

		currentOuterLine.offset(FVector(0, 0, endHeight));
		pols.Append(getSidesOfPolygon(currentOuterLine, PolygonType::concrete, endHeight));
		//currentOuterLine.width = 
		pols.Add(currentOuterLine);
		if (i != 1) {
			// add the corner
			FMaterialPolygon corner;
			corner.type = PolygonType::concrete;
			corner.points.Add(prevP1);
			corner.points.Add(p.points[i - 1] + width*normal);
			corner.points.Add(prevP2);
			corner.points.Add(prevP1);
			corner.offset(FVector(0, 0, 30));

			FVector otherTan = prevP2 - prevP1;
			otherTan.Normalize();
			currentOuterLine.points.Empty();
			currentOuterLine.type = PolygonType::concrete;
			currentOuterLine.points.Add(prevP2);
			currentOuterLine.points.Add(p.points[i - 1] + width*normal);
			currentOuterLine.points.Add(p.points[i - 1] + (width+endWidth)*normal);
			currentOuterLine.points.Add(prevP2 + (endWidth)*otherTan);
			currentOuterLine.points.Add(prevP2);
			currentOuterLine.offset(FVector(0, 0, endHeight));
			pols.Append(getSidesOfPolygon(currentOuterLine, PolygonType::concrete, endHeight));
			pols.Add(currentOuterLine);
			pols.Add(corner);
		}
		//if (i != p.points.Num() - 1) {
			prevP1 = p.points[i];
			prevP2 = p.points[i] + width*normal;
		//}
		pols.Add(current);

	}

	FVector normal = getNormal(p.points[1], p.points[0], p.buildLeft);
	normal.Normalize();
	//prevP1 = p.points[p.points.Num() - 2];
	//prevP2 = p.points[p.points.Num() - 1] + width*normal;
	FMaterialPolygon corner;
	corner.type = PolygonType::concrete;
	corner.points.Add(p.points[0] + width*normal);
	corner.points.Add(prevP2);
	corner.points.Add(prevP1);

	corner.offset(FVector(0, 0, 30));
	pols.Add(corner);
	return pols;
}

FPolygon APlotBuilder::generateSidewalkPolygon(FPlotPolygon p, float offsetSize) {
	FPolygon polygon;
	if (!p.open && p.getArea() > 700) {
		FVector center = p.getCenter();
		for (int i = 1; i < p.points.Num(); i++) {
			FVector tangent = p.points[i] - p.points[i - 1];
			tangent.Normalize();
			FVector offset = (p.buildLeft ? FRotator(0, 270, 0) : FRotator(0, 90, 0)).RotateVector(tangent * offsetSize);
			polygon.points.Add(p.points[i - 1] + offset);
			polygon.points.Add(p.points[i] + offset);
		}
		//if (FVector::Dist(p.points[p.points.Num() - 1], p.points[p.points.Num() - 2]) > 100.0f) {
		//	polygon.points.Add(p.points[p.points.Num() - 1]);
		//}

		if (!p.open) {
			//polygon.points.RemoveAt(polygon.points.Num() - 1);
			//polygon.points.Add(FVector(polygon.points[0]));
			polygon.points.Add(FVector(polygon.points[1]));

			//polygon.points.Add(FVector(polygon.points[2]));


		}
		else {
			FVector last = p.points[p.points.Num() - 1];
			polygon.points.Add(last);
			//polygon.points.Add(last);

		}
	}
	return polygon;
}

FSidewalkInfo APlotBuilder::getSideWalkInfo(FPolygon sidewalk)
{
	FSidewalkInfo toReturn;
	// trees
	if (FMath::FRand() < 0.1f) {
		float placeRatio = 0.001;
		for (int i = 1; i < sidewalk.points.Num(); i += 2) {
			int toPlace = placeRatio * (sidewalk.points[i] - sidewalk.points[i - 1]).Size();
			for (int j = 1; j < toPlace; j++) {
				FVector origin = sidewalk.points[i - 1];
				FVector target = sidewalk.points[i];
				FVector tan = target - origin;
				float len = tan.Size();
				tan.Normalize();
				toReturn.staticMeshes.Add(FMeshInfo{ "tree", FTransform(origin + j * tan * (len / toPlace)) });
			}
		}
	}
	// lamp posts
	float placeRatio = 0.0007;

	for (int i = 1; i < sidewalk.points.Num(); i += 2) {
		int toPlace = placeRatio * (sidewalk.points[i] - sidewalk.points[i - 1]).Size();
		for (int j = 1; j < toPlace; j++) {
			FVector origin = sidewalk.points[i - 1];
			FVector target = sidewalk.points[i];
			FVector tan = target - origin;
			FVector normal = getNormal(origin, target, true);
			float len = tan.Size();
			tan.Normalize();
			toReturn.staticMeshes.Add(FMeshInfo{ "lamppost", FTransform(normal.Rotation(), origin + j * tan * (len / toPlace)) });
		}
	}


	return toReturn;
}

TArray<FMaterialPolygon> APlotBuilder::getSimplePlotPolygonsPB(TArray<FSimplePlot> plots) {
	return BaseLibrary::getSimplePlotPolygons(plots);
}



// Called every frame
void APlotBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

