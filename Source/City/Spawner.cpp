// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "simplexnoise.h"
#include "Spawner.h"


// Sets default values
ASpawner::ASpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PathSpline = CreateDefaultSubobject<USplineMeshComponent>(TEXT("Path"));

	FString pathName = "StaticMesh'/Game/road.road'";
	ConstructorHelpers::FObjectFinder<UStaticMesh> buildingMesh(*pathName);
	meshRoad = buildingMesh.Object;
}



int convertToMapIndex(int x, int y) {
	return x * 10000 + y;
}



void addRoadToMap(TMap <int, TArray<FRoadSegment*>*> &map, FRoadSegment* current, float intervalLength) {
	FVector middle = (current->p2 - current->p1) / 2 + current->p1;
	int x = FMath::RoundToInt((middle.X / intervalLength));
	int y = FMath::RoundToInt((middle.Y / intervalLength));
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			if (map.Find(convertToMapIndex(i+x, j+y)) == nullptr) {
				TArray<FRoadSegment*>* newArray = new TArray<FRoadSegment*>();
				newArray->Add(current);
				map.Emplace(convertToMapIndex(i+x, j+y), newArray);
			}
			else {
				map[convertToMapIndex(i+x, j+y)]->Add(current);
			}
		}
	}

}

TArray<TArray<FRoadSegment*>*> getRelevantSegments(TMap <int, TArray<FRoadSegment*>*> &map, FRoadSegment* current, float intervalLength) {
	TArray<TArray<FRoadSegment*>*> allInteresting;
	FVector middle = (current->p2 - current->p1) / 2 + current->p1;
	int x = FMath::RoundToInt((middle.X / intervalLength));
	int y = FMath::RoundToInt((middle.Y / intervalLength));

	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			if (map.Find(convertToMapIndex(i+x, j+y)) != nullptr) {
				allInteresting.Add(map[convertToMapIndex(i+x, j+y)]);
			}
		}
	}

	return allInteresting;

}


void ASpawner::addVertices(FRoadSegment* road) {
	road->v1 = road->p1 + FRotator(0, 90, 0).RotateVector(road->beginTangent) * standardWidth*road->width / 2;
	road->v2 = road->p1 - FRotator(0, 90, 0).RotateVector(road->beginTangent) * standardWidth*road->width / 2;
	FVector endTangent = road->p2 - road->p1;
	endTangent.Normalize();
	road->v3 = road->p2 + FRotator(0, 90, 0).RotateVector(endTangent) * standardWidth*road->width / 2;
	road->v4 = road->p2 - FRotator(0, 90, 0).RotateVector(endTangent) * standardWidth*road->width / 2;

}


float noise(float multiplier, float x, float y) {
	return scaled_octave_noise_2d(2, 1.0, multiplier, 0, 1, x + noiseXOffset, y + noiseYOffset);
}

bool ASpawner::placementCheck(TArray<FRoadSegment*> &segments, logicRoadSegment* current, TMap <int, TArray<FRoadSegment*>*> &map){

	// use SAT collision between all roads
	FVector tangent1 = current->segment->p2 - current->segment->p1;
	tangent1.Normalize();
	FVector tangent2 = FRotator(0, 90, 0).RotateVector(tangent1);

	addVertices(current->segment);
	TArray<FVector> vert1;
	vert1.Add(current->segment->v1);
	vert1.Add(current->segment->v2);
	vert1.Add(current->segment->v3);
	vert1.Add(current->segment->v4);

	for (FRoadSegment* f : segments){
		if (f == current->previous->segment)
			continue; 
		
		TArray<FVector> tangents;

		// can't be too close to another segment
		if (FVector::Dist((f->p2 - f->p1) / 2 + f->p1, (current->segment->p2 - current->segment->p1) / 2 + current->segment->p1) < 5000) {
			return false;
		}


		FVector tangent3 = f->p2 - f->p1;
		FVector tangent4 = FRotator(0, 90, 0).RotateVector(tangent3);

		tangents.Add(tangent1);
		tangents.Add(tangent2);
		tangents.Add(tangent3);
		tangents.Add(tangent4);

		TArray<FVector> vert2;
		vert2.Add(f->v1);
		vert2.Add(f->v2);
		vert2.Add(f->v3);
		vert2.Add(f->v4);

		if (testCollision(tangents, vert1, vert2, collisionLeniency)) {
			FVector newE = intersection(current->segment->p1, current->segment->p2, f->p1, f->p2);
			if (newE.X == 0) {
				// check if they collide into each other from the back
				FVector otherTangent = f->p2 - f->p1;
				otherTangent = FRotator(0, 90, 0).RotateVector(otherTangent);
				otherTangent.Normalize();

				newE = intersection(f->p1 + otherTangent * 200, f->p1 - otherTangent * 200, current->segment->p1, current->segment->p2);
				if (newE.X == 0.0f) {
					newE = intersection(f->p2 + otherTangent * 200, f->p2 - otherTangent * 200, current->segment->p1, current->segment->p2);

				}
				if (newE.X == 0.0f)
					continue;
			}
				current->time = 100000;
				FVector tangent = newE - current->segment->p1;
				tangent.Normalize();
				float len = FVector::Dist(newE, current->segment->p1);
				current->segment->p2 = current->segment->p1 + (len - standardWidth / 2) * tangent;
				current->segment->roadInFront = true;

				FVector naturalTangent = current->segment->p2 - current->segment->p1;
				naturalTangent.Normalize();
				FVector pot1 = FRotator(0, 90, 0).RotateVector(f->p2 - f->p1);
				FVector pot2 = FRotator(0, 270, 0).RotateVector(f->p2 - f->p1);
				addVertices(current->segment);
		}


	}

	return true;
	

}

FRotator getBestRotation(float maxDiffAllowed, FRotator original, FVector originalPoint, FVector step, float noiseScale, TArray<logicRoadSegment*> &others, float maxDist, float detriment) {
	float bestVal = -1000000;
	FRotator bestRotator;
	for (int i = 0; i < 10; i++) {
		FRotator curr = original + FRotator(0, FMath::FRandRange(-maxDiffAllowed, maxDiffAllowed), 0);
		FVector testPoint = originalPoint + curr.RotateVector(step);
		float val = noise(noiseScale, testPoint.X, testPoint.Y);
		for (logicRoadSegment* t : others) {
			if (FVector::Dist(t->segment->getMiddle(), testPoint) < maxDist)
				val -= detriment;// * (FVector::Dist(t->segment->getMiddle(), testPoint))/maxDist;
		}
		if (val > bestVal) {
			bestRotator = curr;
			bestVal = noise(noiseScale, testPoint.X, testPoint.Y);
		}
	}
	return bestRotator;
}

void ASpawner::addRoadForward(std::priority_queue<logicRoadSegment*, std::deque<logicRoadSegment*>, roadComparator> &queue, logicRoadSegment* previous, std::vector<logicRoadSegment*> &allsegments) {
	FRoadSegment* prevSeg = previous->segment;
	logicRoadSegment* newRoadL = new logicRoadSegment();
	FRoadSegment* newRoad = new FRoadSegment();
	FVector stepLength = prevSeg->type == RoadType::main ? primaryStepLength : secondaryStepLength;
	//newRoadL->secondDegreeRot = previous->secondDegreeRot + FRotator(0, (prevSeg->type == RoadType::main ? changeIntensity : secondaryChangeIntensity)*(randFloat() - 0.5f), 0);

	//float inset = 50;
	//FVector tan = prevSeg->p2 - prevSeg->p1;
	//tan.Normalize();
	newRoad->p1 = prevSeg->p2;
	// set seconddegreerot to attempt to change towards that direction

	TArray<logicRoadSegment*> otherMain;
	for (logicRoadSegment* f : allsegments) {
		if (f->segment->type == RoadType::main) {
			otherMain.Add(f);
		}
	}

	FRotator bestRotator = getBestRotation((prevSeg->type == RoadType::main ? changeIntensity : secondaryChangeIntensity), previous->firstDegreeRot,newRoad->p1, stepLength, noiseScale, otherMain, mainRoadDetrimentRange, mainRoadDetrimentImpact);

	newRoadL->firstDegreeRot = bestRotator;//previous->firstDegreeRot + newRoadL->secondDegreeRot;


	newRoad->p2 = newRoad->p1 + newRoadL->firstDegreeRot.RotateVector(stepLength);
	newRoad->beginTangent = prevSeg->p2 - prevSeg->p1;
	newRoad->beginTangent.Normalize();
	newRoad->width = prevSeg->width;
	newRoad->type = prevSeg->type;
	newRoad->endTangent = newRoad->p2 - newRoad->p1;
	newRoadL->segment = newRoad;
	FVector mP = middle(newRoad->p1, newRoad->p2);
	newRoadL->time = -noise(noiseScale, mP.X, mP.Y) + ((newRoad->type == RoadType::main) ? mainRoadAdvantage : 0) + std::abs(0.1*previous->time);// + FMath::FRand() * 0.1;
	//newRoadL->time = - raw_noise_2d(((newRoad->p1.X + newRoad->p2.X)/2)*noiseScale, ((newRoad->p1.Y + newRoad->p2.Y)/2)*noiseScale) - ((newRoad->type == RoadType::main) ? mainRoadAdvantage : 0);
	newRoadL->roadLength = previous->roadLength + 1;
	newRoadL->previous = previous;
	addVertices(newRoad);
	queue.push(newRoadL);
	allsegments.push_back(newRoadL);
	
}

void ASpawner::addRoadSide(std::priority_queue<logicRoadSegment*, std::deque<logicRoadSegment*>, roadComparator> &queue, logicRoadSegment* previous, bool left, float width, std::vector<logicRoadSegment*> &allsegments, RoadType newType) {
	FRoadSegment* prevSeg = previous->segment;
	logicRoadSegment* newRoadL = new logicRoadSegment();
	FRoadSegment* newRoad = new FRoadSegment();
	FVector stepLength = newType == RoadType::main ? primaryStepLength : secondaryStepLength;

	newRoadL->secondDegreeRot = FRotator(0, 0, 0);//FRotator(0, (prevSeg->type == RoadType::main ? changeIntensity : secondaryChangeIntensity)*(randFloat() - 0.5f), 0);
	FRotator newRotation = left ? FRotator(0, 90, 0) : FRotator(0, 270, 0);
	newRoadL->firstDegreeRot = previous->firstDegreeRot + newRotation;
	FVector startOffset = newRoadL->firstDegreeRot.RotateVector(FVector(standardWidth*previous->segment->width / 2, 0, 0));
	newRoad->p1 = /*prevSeg->end - (standardWidth * (prevSeg->end - prevSeg->start).Normalize()/2) + startOffset; */prevSeg->p1 + (prevSeg->p2 - prevSeg->p1) / 2 + startOffset;


	//float bestVal = 10000;
	//FRotator bestRotator;
	//float diffAllowed = 20;
	// get best direction
	//for (int i = 0; i < 5; i++) {
	//	FRotator curr = newRoadL->firstDegreeRot + FRotator(0, FMath::FRandRange(-diffAllowed, diffAllowed), 0);
	//	FVector testPoint = newRoad->p1 + curr.RotateVector(stepLength);
	//	if (noise(noiseScale, testPoint.X, testPoint.Y) < bestVal) {
	//		bestRotator = curr;
	//		bestVal = noise(noiseScale, testPoint.X, testPoint.Y);
	//	}
	//}
	TArray<logicRoadSegment*> segs;
	FRotator bestRotator = getBestRotation((prevSeg->type == RoadType::main ? changeIntensity : secondaryChangeIntensity), newRoadL->firstDegreeRot, newRoad->p1, stepLength, noiseScale, segs, 0, 0);
	newRoadL->firstDegreeRot = bestRotator;//previous->firstDegreeRot + newRoadL->secondDegreeRot;


	newRoad->p2 = newRoad->p1 + newRoadL->firstDegreeRot.RotateVector(stepLength);
	newRoad->beginTangent = FRotator(0, left ? 90 : 270, 0).RotateVector(previous->segment->p2 - previous->segment->p1); //->p2 - newRoad->p1;
	newRoad->beginTangent.Normalize();
	newRoad->width = width;
	newRoad->type = newType;
	newRoad->endTangent = newRoad->p2 - newRoad->p1;

	newRoadL->segment = newRoad;
	// every side track has less priority

	FVector mP = middle(newRoad->p1, newRoad->p2);
	newRoadL->time = -noise(noiseScale, mP.X, mP.Y) + ((newRoad->type == RoadType::main) ? mainRoadAdvantage : 0) + std::abs(0.01*previous->time);// +FMath::FRand() * 0.1;// ((newRoad->type == RoadType::main) ? mainRoadAdvantage : 0);

	newRoadL->roadLength = (previous->segment->type == RoadType::main && newType != RoadType::main) ? 1 : previous->roadLength+1;
	newRoadL->previous = previous;

	addVertices(newRoad);
	queue.push(newRoadL);
	allsegments.push_back(newRoadL);

}

void ASpawner::addExtensions(std::priority_queue<logicRoadSegment*, std::deque<logicRoadSegment*>, roadComparator> &queue, logicRoadSegment* current, std::vector<logicRoadSegment*> &allsegments) {
	float mainRoadSize = 4.0f;
	float sndRoadMinSize = 1.5f;
	float sndRoadSize = std::max(sndRoadMinSize, current->segment->width - 2.0f);
	FVector tangent = current->segment->p2 - current->segment->p1;
	if (current->segment->type == RoadType::main) {
		// on the main road
		if (current->roadLength < maxMainRoadLength)
			addRoadForward(queue, current, allsegments);

		if (randFloat() < mainRoadBranchChance) {
			if (randFloat() < 0.5) {
				addRoadSide(queue, current, true, mainRoadSize, allsegments, RoadType::main);
			}

			else{
				addRoadSide(queue, current, false, mainRoadSize, allsegments, RoadType::main);
			}
		}
		else {
			//if (randFloat() < secondaryRoadBranchChance) {
				addRoadSide(queue, current, true, sndRoadSize, allsegments, RoadType::secondary);
			//}
			//if (randFloat() < secondaryRoadBranchChance) {
				addRoadSide(queue, current, false, sndRoadSize, allsegments, RoadType::secondary);
			//}
		}
		//}
	}

	else if (current->segment->type == RoadType::secondary) {
		// side road
		if (current->roadLength < maxSecondaryRoadLength) {
			addRoadForward(queue, current, allsegments);

			addRoadSide(queue, current, true, sndRoadSize, allsegments, RoadType::secondary);
			addRoadSide(queue, current, false, sndRoadSize, allsegments, RoadType::secondary);
		}
	}


}

TArray<FRoadSegment> ASpawner::determineRoadSegments()
{
	FVector origin;


	TArray<FRoadSegment*> determinedSegments;
	TArray<FRoadSegment> finishedSegments;

	noiseXOffset = FMath::FRandRange(-1000000, 1000000);
	noiseYOffset = FMath::FRandRange(-1000000, 1000000);


	std::priority_queue<logicRoadSegment*, std::deque<logicRoadSegment*>, roadComparator> queue;


	std::vector<logicRoadSegment*> allSegments;
	logicRoadSegment* start = new logicRoadSegment();
	start->time = 0;
	FRoadSegment* startR = new FRoadSegment();
	startR->beginTangent = primaryStepLength;

	FVector point = FVector(0, 0, 0);

	float curr = noise(point.X, point.Y, noiseScale);
	float stepLen = 10000;
	while (true) {
		FVector alt1 = point + FVector(stepLen, 0, 0);
		FVector alt2 = point + FVector(-stepLen, 0, 0);
		FVector alt3 = point + FVector(0, stepLen, 0);
		FVector alt4 = point + FVector(0, -stepLen, 0);
		float res1 = noise(alt1.X, alt1.Y, noiseScale);
		float res2 = noise(alt2.X, alt2.Y, noiseScale);
		float res3 = noise(alt3.X, alt3.Y, noiseScale);
		float res4 = noise(alt4.X, alt4.Y, noiseScale);

		if (res1 > curr) {
			curr = res1;
			point = alt1;
		}
		else if (res2 > curr) {
			curr = res2;
			point = alt2;
		}
		else if (res3 > curr) {
			curr = res3;
			point = alt3;
		}
		else if (res4 > curr) {
			curr = res4;
			point = alt4;
		}
		else {
			break;
		}
	}


	startR->p1 = point;


	start->segment = startR;
	float bestVal = -1000000;
	FRotator bestRot;
	for (int i = 0; i < 360; i++) {
		FVector testPoint = point + FRotator(0, i, 0).RotateVector(primaryStepLength);
		if (noise(noiseScale, point.X, point.Y) > bestVal) {
			bestVal = noise(noiseScale, point.X, point.Y);
			bestRot = FRotator(0, i, 0);
		}
	}
	startR->p2 = startR->p1 + bestRot.RotateVector(primaryStepLength);
	startR->width = 5.0f;
	startR->type = RoadType::main;
	startR->endTangent = startR->p2 - startR->p1;

	start->firstDegreeRot = bestRot;
	start->secondDegreeRot = FRotator(0, 0, 0);
	start->roadLength = 1;
	addVertices(startR);

	queue.push(start);

	// map for faster comparisons
	TMap <int, TArray<FRoadSegment*>*> segmentsOrganized;

	// loop for everything else

	while (queue.size() > 0 && determinedSegments.Num() < length) {
		logicRoadSegment* current = queue.top();
		queue.pop();
		if (placementCheck(determinedSegments, current, segmentsOrganized)){/// && FVector::Dist(current->segment->p1, current->segment->p2) > 2000) {
			determinedSegments.Add(current->segment);
			if (current->previous && FVector::Dist(current->previous->segment->p2, current->segment->p1) < 1.0f)
				current->previous->segment->roadInFront = true;
			//addRoadToMap(segmentsOrganized, current->segment, primaryStepLength.Size() / 2);

			UE_LOG(LogTemp, Warning, TEXT("CURRENT SEGMENT PRIORITY %f"), current->time);
			addExtensions(queue, current, allSegments);
		}
	}



	// make sure roads attach properly if there is another road not too far in front of them
	for (int k = 0; k < 1; k++) {
		for (int i = 0; i < determinedSegments.Num(); i++) {
			FRoadSegment* f2 = determinedSegments[i];
			if (f2->roadInFront)
				continue;
			FVector tangent = f2->p2 - f2->p1;
			tangent.Normalize();
			FVector p2Prev = f2->p2;
			FVector p1Prev = f2->p1;
			f2->p2 += tangent*maxAttachDistance;
			//f2->p1 -= tangent*maxAttachDistance;
			float closestDist = 10000000.0f;
			FRoadSegment* closest = nullptr;
			FVector impactP;
			//addVertices(f2);
			bool foundCollision = false;
			for (int j = 0; j < determinedSegments.Num(); j++) {
				FRoadSegment* f = determinedSegments[j];
				if (i == j) {
					continue;
				}
				//if (FVector::Dist(f->p1, f2->p2) < 100 || FVector::Dist(f->p2, f2->p1) < 100) {
				//	foundCollision = false;
				//	break;
				//}
				FVector fTan = f->p2 - f->p1;
				//FVector fTan = FVector(0,0,0);
				fTan.Normalize();
				FVector res = intersection(f2->p1, f2->p2, f->p1 - fTan * 100, f->p2 + fTan * 100);
				if (res.X != 0.0f && FVector::Dist(f2->p1, res) < closestDist) {
					closestDist = FVector::Dist(f2->p1, res);
					closest = f;
					impactP = res;
					foundCollision = true;
				}


			}
			if (foundCollision) {
				//FVector tangent2 = impactP - f2->p1;
				//tangent2.Normalize();
				float len = FVector::Dist(impactP, f2->p1);
				f2->p2 = impactP;// f2->p1 + (len - standardWidth / 2) * tangent;

				FVector naturalTangent = f2->p2 - f2->p1;
				naturalTangent.Normalize();
				FVector pot1 = FRotator(0, 90, 0).RotateVector(closest->p2 - closest->p1);
				pot1.Normalize();
				FVector pot2 = FRotator(0, 270, 0).RotateVector(closest->p2 - closest->p1);
				pot2.Normalize();
				addVertices(f2);
				//f2->endTangent = FVector::DistSquared(naturalTangent, pot1) < FVector::DistSquared(naturalTangent, pot2) ? pot1 : pot2;
				f2->roadInFront = true;
			}
			else {
				f2->p2 = p2Prev;
				//f2->p1 += tangent*maxAttachDistance;
				//addVertices(f2);
			}
		}


	}


	TArray<int> keys;
	segmentsOrganized.GetKeys(keys);
	UE_LOG(LogTemp, Warning, TEXT("deleting %i keys in map"), keys.Num());
	for (int key : keys) {
		delete(segmentsOrganized[key]);
	}

	for (FRoadSegment* f : determinedSegments) {
		finishedSegments.Add(*f);
	}

	for (logicRoadSegment* s : allSegments){
		delete(s->segment);
		delete(s);
	}

	return finishedSegments;
}

TArray<FMaterialPolygon> ASpawner::getRoadLines(TArray<FRoadSegment> segments)
{
	float lineInterval = 600;
	float lineLen = 350;
	float lineWidth = 40;
	TArray<FMaterialPolygon> lines;
	for (FRoadSegment f : segments) {
		if (f.type == RoadType::main) {
			FVector tangent = f.p2 - f.p1;
			tangent.Normalize();
			int spaces = FVector::Dist(f.p2, f.p1) / (lineInterval+1);
			for (int i = 1; i < spaces; i++) {
				FVector startPos = tangent * lineInterval * i + f.p1;
				FVector endPos = startPos + tangent*lineLen;
				FVector normal = getNormal(endPos, startPos, true);
				normal.Normalize();
				FMaterialPolygon line;
				line.type = PolygonType::roadMiddle;
				line.points.Add(startPos + normal*lineWidth);
				line.points.Add(startPos - normal*lineWidth);
				line.points.Add(endPos - normal*lineWidth);
				line.points.Add(endPos + normal*lineWidth);
				line.offset(FVector(0, 0, 20));
				lines.Add(line);
				//lines.Add(FLine{ startPos, endPos,5});
			}
		}
	}

	return lines;
}

TArray<FPolygon> ASpawner::roadsToPolygons(TArray<FRoadSegment> segments)
{
	TArray<FPolygon> polygons;
	for (FRoadSegment f : segments) {
		FPolygon p;
		p.points.Add(f.v1);
		p.points.Add(f.v2);
		p.points.Add(f.v3);
		p.points.Add(f.v4);
		polygons.Add(p);
	}
	return polygons;
}


TArray<FTransform> ASpawner::visualizeNoise(int numSide, float noiseMultiplier, float posMultiplier) {
	TArray<FTransform> toReturn;
	for (int i = 0; i < numSide; i++) {
		for (int j = 0; j < numSide; j++) {
			FTransform f;
			f.SetLocation(FVector(posMultiplier * i, posMultiplier * j, 0));
			//float x = f.GetLocation().X * noiseMultiplier;
			//float y = f.GetLocation().Y * noiseMultiplier;
			//float res = raw_noise_2d(x, y);
			float res = noise(noiseMultiplier, f.GetLocation().X, f.GetLocation().Y);
			//res = octave_noise_2d(5, 0.5, 1.0, x, y);
			f.SetScale3D(FVector(posMultiplier/100, posMultiplier/100,  res* posMultiplier/10));
			toReturn.Add(f);
		}
	}
	return toReturn;
}




TArray<FMetaPolygon> ASpawner::getSurroundingPolygons(TArray<FRoadSegment> segments)
{
	return BaseLibrary::getSurroundingPolygons(segments, segments, standardWidth, 500, 100, 200, 50);
}

// Called when the game starts or when spawned
void ASpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
