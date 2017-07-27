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
FLine getCrossingLine(float dist, FPolygon road) {
	FVector tanL = road.points[2] - road.points[1];
	float totLen = tanL.Size();
	tanL.Normalize();
	FVector startP = road.points[1] + totLen*dist*tanL;
	FVector endP = intersection(startP, startP + FRotator(0, 90, 0).RotateVector(tanL) * 10000, road.points[0], road.points[3]);
	if (endP.X == 0.0f)
		return{ FVector(0,0,0),FVector(0,0,0)};
	return{ startP, endP };
}


TArray<FMaterialPolygon> getCrossingAt(float dist, FPolygon road, float lineWidth) {
	//FVector startP = middle(road.points[1], road.points[2]);
	//FVector endP = middle(road.points[0], road.points[3]);
	FLine line = getCrossingLine(dist, road);
	if (line.p1.X == 0.0f)
		return TArray<FMaterialPolygon>();
	float lineInterval = 200;
	float lineLen = 100;
	TArray<FMaterialPolygon> lines;
	FVector tangent = line.p2 - line.p1;
	tangent.Normalize();
	int spaces = FVector::Dist(line.p1, line.p2) / (lineInterval + 1);
	for (int i = 1; i < spaces; i++) {
		FVector startPos = tangent * lineInterval * i + line.p1;
		FVector endPos = startPos + tangent*lineLen;
		FVector normal = getNormal(endPos, startPos, true);
		normal.Normalize();
		FMaterialPolygon newLine;
		newLine.type = PolygonType::roadMiddle;
		newLine.points.Add(startPos + normal*lineWidth);
		newLine.points.Add(startPos - normal*lineWidth);
		newLine.points.Add(endPos - normal*lineWidth);
		newLine.points.Add(endPos + normal*lineWidth);
		newLine.offset(FVector(0, 0, 11));
		lines.Add(newLine);
					
	}
	return lines;
}

FCityDecoration APlotBuilder::getCityDecoration(TArray<FMetaPolygon> plots, TArray<FPolygon> roads) {
	FCityDecoration dec;
	TMap<FMetaPolygon*, TSet<FMetaPolygon*>> connectionsMap;

	for (FPolygon road : roads) {
		FMetaPolygon *firstHit = nullptr;
		FMetaPolygon *sndHit = nullptr;
		FLine line = getCrossingLine(0.25, road);
		FLine testLine;
		FVector tan = line.p2 - line.p1;
		tan.Normalize();
		testLine.p1 = line.p1 - tan * 100;
		testLine.p2 = line.p2 + tan * 100;
		for (FMetaPolygon &plot : plots) {
			if (intersection(testLine.p1, testLine.p2, plot).X != 0.0f) {
				if (firstHit) {
					sndHit = &plot;
					// if these two plots werent previously connected, connections are added and crossing is placed, otherwise discard
					if (!connectionsMap[firstHit].Contains(sndHit)) {
						if (!connectionsMap.Contains(sndHit)) {
							connectionsMap.Add(sndHit, TSet<FMetaPolygon*>());
						}
						connectionsMap[firstHit].Add(sndHit);
						connectionsMap[sndHit].Add(firstHit);
						float width = 300;
						dec.polygons.Append(getCrossingAt(0.25, road, width));
						// add traffic lights
						FLine crossingLine = getCrossingLine(0.25, road);
						FRotator lookingDir = getNormal(crossingLine.p1, crossingLine.p2, true).Rotation();
						FVector offset = crossingLine.p2 - crossingLine.p1;
						offset.Normalize();
						offset *= -300;
						offset += lookingDir.RotateVector(FVector(700, 0, 0));
						if (randFloat() < 0.5) {
							dec.meshes.Add(FMeshInfo{ "traffic_light", FTransform{ lookingDir + FRotator(0,90,0), crossingLine.p1 - offset, FVector(1.0,1.0,1.0) } });
							dec.meshes.Add(FMeshInfo{ "traffic_light", FTransform{ lookingDir + FRotator(0,270,0), crossingLine.p2 + offset, FVector(1.0,1.0,1.0) } });
						}


						break;

					}
				}
				else {
					firstHit = &plot;
					if (!connectionsMap.Contains(firstHit)) {
						connectionsMap.Add(firstHit, TSet<FMetaPolygon*>());
					}
				}
			}
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("Added %i sidewalks"), dec.polygons.Num())

	return dec;
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
	pol.height = stream.RandRange(minFloors, minFloors + (maxFloors - minFloors) * NoiseSingleton::getInstance()->noise(pol.housePosition.X, pol.housePosition.Y, noiseScale));//randFloat() * (maxFloors - minFloors) + minFloors;
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
		for (int32 i = 1; i < original.points.Num()+1; i++) {
			original.entrances.Add(i);
			original.windows.Add(i);
		}
		FVector center = p.getCenter();
		p.type = stream.FRand() < 0.5 ? RoomType::office : RoomType::apartment;// NoiseSingleton::getInstance()->noise(original.housePosition.X, original.housePosition.Y, noiseScale) > 0.5 ? RoomType::office : RoomType::apartment;

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
			for (int i = 0; i < 10; i++) {
				FHousePolygon newH = model;
				newH.rotate(FRotator(0, stream.FRandRange(0, 360), 0));
				newH.offset(p.getRandomPoint(true, 2000));
				newH.housePosition = newH.getCenter();
				newH.type = p.type;
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

			}
		}
		if (normalPlacement) {
			TArray<FHousePolygon> refinedPolygons = original.refine(maxArea, 0, 0);
			for (FHousePolygon r : refinedPolygons) {
				r.housePosition = r.getCenter();
				r.height = stream.FRandRange(minFloors, minFloors + (maxFloors - minFloors) * NoiseSingleton::getInstance()->noise(r.housePosition.X, r.housePosition.Y, noiseScale));

				if (stream.FRand() < 0.2) {
					r.height *= 2;
				}
				r.type = p.type;
				r.simplePlotType = r.type == RoomType::office ? SimplePlotType::asphalt : SimplePlotType::green;

				float area = r.getArea();

				if (area < minArea || area > maxArea) {

					//if (r.getIsClockwise())
					//	r.reverse();
					//r.points.RemoveAt(r.points.Num() - 1);
					FSimplePlot fs;
					fs.pol = r;
					fs.pol.offset(FVector(0, 0, 30));
					fs.type = p.type == RoomType::apartment ? SimplePlotType::green : SimplePlotType::asphalt;
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
	for (int i = 1; i < p.points.Num()+1; i++) {
		FVector p1 = p.points[i-1];
		FVector p2 = p.points[i%p.points.Num()];
		// add the straight part
		FMaterialPolygon current;
		current.type = PolygonType::concrete;
		FVector normal = getNormal(p1, p2, !p.buildLeft);
		normal.Normalize();
		current.points.Add(p1);
		current.points.Add(p2);
		current.points.Add(p2 + width*normal);
		current.points.Add(p1 + width*normal);
		current.offset(FVector(0, 0, 30));

		FMaterialPolygon currentOuterLine;
		currentOuterLine.type = PolygonType::concrete;
		currentOuterLine.points.Add(p2 + width*normal);
		currentOuterLine.points.Add(p2 + (width + endWidth)*normal);
		currentOuterLine.points.Add(p1 + (width+endWidth)*normal);
		currentOuterLine.points.Add(p1 + width*normal);
		//currentOuterLine.points.Add(p.points[i] + width*normal);

		currentOuterLine.offset(FVector(0, 0, endHeight));
		pols.Append(getSidesOfPolygon(currentOuterLine, PolygonType::concrete, endHeight));
		pols.Add(currentOuterLine);
		if (i != 1) {
			// add the corner
			FMaterialPolygon corner;
			corner.type = PolygonType::concrete;
			corner.points.Add(prevP1);
			corner.points.Add(p1 + width*normal);
			corner.points.Add(prevP2);
			//corner.points.Add(prevP1);
			corner.offset(FVector(0, 0, 30));

			FVector otherTan = prevP2 - prevP1;
			otherTan.Normalize();
			currentOuterLine.points.Empty();
			currentOuterLine.type = PolygonType::concrete;
			currentOuterLine.points.Add(prevP2);
			currentOuterLine.points.Add(p1 + width*normal);
			currentOuterLine.points.Add(p1 + (width+endWidth)*normal);
			currentOuterLine.points.Add(prevP2 + (endWidth)*otherTan);
			//currentOuterLine.points.Add(prevP2);
			currentOuterLine.offset(FVector(0, 0, endHeight));
			pols.Append(getSidesOfPolygon(currentOuterLine, PolygonType::concrete, endHeight));
			pols.Add(currentOuterLine);
			pols.Add(corner);
		}
		prevP1 = p2;
		prevP2 = p2 + width*normal;
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

	FVector otherTan = prevP2 - prevP1;
	otherTan.Normalize();
	FMaterialPolygon currentOuterLine;
	currentOuterLine.type = PolygonType::concrete;
	currentOuterLine.points.Add(prevP2);
	currentOuterLine.points.Add(p.points[p.points.Num()-1] + width*normal);
	currentOuterLine.points.Add(p.points[p.points.Num() - 1] + (width + endWidth)*normal);
	currentOuterLine.points.Add(prevP2 + (endWidth)*otherTan);
	currentOuterLine.points.Add(prevP2);
	currentOuterLine.offset(FVector(0, 0, endHeight));
	pols.Append(getSidesOfPolygon(currentOuterLine, PolygonType::concrete, endHeight));
	pols.Add(currentOuterLine);

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

	if (sidewalk.points.Num() < 2)
		return toReturn;
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

	// fire hydrants
	float placeChance = 0.4;
	if (FMath::FRand() < placeChance) {
		int place = FMath::FRandRange(1, sidewalk.points.Num()-1);
		FVector rot = getNormal(sidewalk[place - 1], sidewalk[place], true);
		rot.Normalize();
		FVector loc = sidewalk[place - 1] + (sidewalk[place] - sidewalk.points[place - 1]) * FMath::FRand() + rot * 200 + FVector(0,0,15);
		toReturn.staticMeshes.Add(FMeshInfo{ "fire_hydrant", FTransform(rot.Rotation(), loc) });

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

