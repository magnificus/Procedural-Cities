// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "Spawner.h"




// Sets default values
ASpawner::ASpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}




float randFloat() {
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}


void getMinMax(float &min, float &max, FVector tangent, FVector v1, FVector v2, FVector v3, FVector v4) {
	float res = FVector::DotProduct(tangent, v1);
	min, max = res;
	res = FVector::DotProduct(tangent, v2);
	min = std::min(min, res);
	max = std::max(max, res);
	res = FVector::DotProduct(tangent, v3);
	min = std::min(min, res);
	max = std::max(max, res);
	res = FVector::DotProduct(tangent, v4);
	min = std::min(min, res);
	max = std::max(max, res);
}


bool lineToLineIntersection(const FVector &fromA, const FVector &fromB, const FVector &toA, const FVector &toB, FVector& _outIntersection)
{
	FVector da = fromB - fromA;
	FVector db = toB - toA;
	FVector dc = toA - fromA;

	FVector crossDaDb = FVector::CrossProduct(da, db);
	float prod = crossDaDb.X * crossDaDb.X + crossDaDb.Y * crossDaDb.Y + crossDaDb.Z * crossDaDb.Z;
	if (prod == 0 || FVector::DotProduct(dc, crossDaDb) != 0)
	{
		return false;
	}
	float res = FVector::DotProduct(FVector::CrossProduct(dc, db), FVector::CrossProduct(da, db)) / prod;

	_outIntersection = fromA + da * FVector(res, res, res);

	FVector fromAToIntersectPoint = _outIntersection - fromA;
	FVector fromBToIntersectPoint = _outIntersection - fromB;
	FVector toAToIntersectPoint = _outIntersection - toA;
	FVector toBToIntersectPoint = _outIntersection - toB;
	if (FVector::DotProduct(fromAToIntersectPoint, fromBToIntersectPoint) <= 0 && FVector::DotProduct(toAToIntersectPoint, toBToIntersectPoint) <= 0)
	{
		return true;
	}
	return false;
}

bool ASpawner::placementCheck(TArray<FRoadSegment*> &segments, logicRoadSegment* current){

	// use SAT collision between all roads

	FVector myTangent1 = current->segment->end - current->segment->start;
	myTangent1.Normalize();
	FVector myTangent2 = FRotator(0, 90, 0).RotateVector(myTangent1);
	float myMin1;
	float myMax1;
	getMinMax(myMin1, myMax1, myTangent1, current->segment->v1, current->segment->v2, current->segment->v3, current->segment->v4);

	float myMin2;
	float myMax2;
	getMinMax(myMin2, myMax2, myTangent2, current->segment->v1, current->segment->v2, current->segment->v3, current->segment->v4);

	float myMin3;
	float myMax3;

	float myMin4;
	float myMax4;

	//current->segment->v1.ToString();
	UE_LOG(LogTemp, Log, TEXT("v1: %s, v2: %s\nv3: %s, v4: %s"), *current->segment->v1.ToString(), *current->segment->v2.ToString(), *current->segment->v3.ToString(), *current->segment->v4.ToString());


	for (FRoadSegment* f : segments) {
		// try all tangents (there are four since both shapes are rectangles)

		float min;
		float max;
		// return true means no overlap, there are in fact 8 cases but well be alright only testing for 4 (fight me)
		getMinMax(min, max, myTangent1, f->v1, f->v2, f->v3, f->v4);
		if (std::max(min, myMin1) >= std::min(max, myMax1)- collisionLeniency) {
			//UE_LOG(LogTemp, Log, TEXT("failed on 1st pass"));
			continue;
		}
		getMinMax(min, max, myTangent2, f->v1, f->v2, f->v3, f->v4);
		if (std::max(min, myMin2) >= std::min(max, myMax2) - collisionLeniency) {
			//UE_LOG(LogTemp, Log, TEXT("failed on 2nd pass"));
			continue;
		}

		FVector thirdTangent = f->end - f->start;
		thirdTangent.Normalize();
		getMinMax(myMin3, myMax3, thirdTangent, current->segment->v1, current->segment->v2, current->segment->v3, current->segment->v4);
		getMinMax(min, max, thirdTangent, f->v1, f->v2, f->v3, f->v4);
		if (std::max(min, myMin3) >= std::min(max, myMax3) - collisionLeniency) {
			//UE_LOG(LogTemp, Log, TEXT("failed on third pass"));
			continue;
		}

		FVector fourthTangent = FRotator(0, 90, 0).RotateVector(thirdTangent);
		getMinMax(myMin4, myMax4, fourthTangent, current->segment->v1, current->segment->v2, current->segment->v3, current->segment->v4);
		getMinMax(min, max, fourthTangent, f->v1, f->v2, f->v3, f->v4);
		if (std::max(min, myMin4) >= std::min(max, myMax4) - collisionLeniency) {
			//UE_LOG(LogTemp, Log, TEXT("failed on fourth pass"));
			continue;
		}

		// OVERLAP! assume it's the end, if they are simply too close, remove the new part, otherwise adjust it
		float dist = FVector::Dist((f->end - f->start) / 2 + f->start, (current->segment->end - current->segment->start) / 2 + current->segment->start);
		//FVector point;
		//if (dist < minRoadCenterDist) {
		return false;
		//}
		//lineToLineIntersection(current->segment->start, f->start, current->segment->end, f->end, current->segment->end);
		
	}


	//for (FRoadSegment* f : segments) {
	//	// dumb collision check, just measure between centres
	//	float dist = FVector::Dist((f->end - f->start) / 2 + f->start, (current->segment->end - current->segment->start) / 2 + current->segment->start);
	//	if (dist < minRoadCenterDist) {
	//		return false;
	//	}
	//}
	return true;
	

}

void ASpawner::addVertices(FRoadSegment* road) {
	road->v1 = road->start + FRotator(0, 90, 0).RotateVector(road->beginTangent) * standardWidth*road->width/2;
	road->v2 = road->start + FRotator(0, 270, 0).RotateVector(road->beginTangent) * standardWidth*road->width /2;
	FVector endTangent = road->end - road->start;
	endTangent.Normalize();
	road->v3 = road->end + FRotator(0, 90, 0).RotateVector(endTangent) * standardWidth*road->width /2;
	road->v4 = road->end + FRotator(0, 270, 0).RotateVector(endTangent) * standardWidth*road->width /2;

}

void ASpawner::addRoadForward(std::priority_queue<logicRoadSegment*, std::deque<logicRoadSegment*>, roadComparator> &queue, logicRoadSegment* previous, std::vector<logicRoadSegment*> &allsegments) {
	FRoadSegment* prevSeg = previous->segment;
	logicRoadSegment* newRoadL = new logicRoadSegment();
	FRoadSegment* newRoad = new FRoadSegment();

	newRoadL->secondDegreeRot = previous->secondDegreeRot + FRotator(0, changeIntensity*(randFloat() - 0.5f), 0);
	newRoadL->firstDegreeRot = previous->firstDegreeRot + newRoadL->secondDegreeRot;

	newRoad->start = prevSeg->end;
	newRoad->end = newRoad->start + newRoadL->firstDegreeRot.RotateVector(stepLength);
	newRoad->beginTangent = prevSeg->end - prevSeg->start;
	newRoad->beginTangent.Normalize();
	newRoad->width = prevSeg->width;
	newRoad->type = prevSeg->type;
	newRoadL->segment = newRoad;
	newRoadL->time = previous->segment->type != RoadType::main ? previous->time + FMath::Rand() % 3 : previous->time;
	newRoadL->roadLength = previous->roadLength + 1;
	newRoad->out = Direction::F;
	newRoad->dir = Direction::F;
	newRoadL->previous = previous;
	addVertices(newRoad);
	queue.push(newRoadL);
	allsegments.push_back(newRoadL);
	
}

void ASpawner::addRoadSide(std::priority_queue<logicRoadSegment*, std::deque<logicRoadSegment*>, roadComparator> &queue, logicRoadSegment* previous, bool left, float width, std::vector<logicRoadSegment*> &allsegments, RoadType newType) {
	FRoadSegment* prevSeg = previous->segment;
	logicRoadSegment* newRoadL = new logicRoadSegment();
	FRoadSegment* newRoad = new FRoadSegment();

	newRoadL->secondDegreeRot = FRotator(0, changeIntensity*(randFloat() - 0.5f), 0);
	FRotator newRotation = left ? FRotator(0, 90, 0) : FRotator(0, 270, 0);
	newRoadL->firstDegreeRot = previous->firstDegreeRot + newRoadL->secondDegreeRot + newRotation;

	FVector startOffset = newRoadL->firstDegreeRot.RotateVector(FVector(standardWidth*previous->segment->width / 2, 0, 0));
	newRoad->start = prevSeg->start + (prevSeg->end - prevSeg->start) / 2 + startOffset;
	newRoad->end = newRoad->start + newRoadL->firstDegreeRot.RotateVector(stepLength);
	newRoad->beginTangent = newRoad->end - newRoad->start;
	newRoad->beginTangent.Normalize();
	newRoad->width = width;
	newRoad->type = newType;
	newRoadL->segment = newRoad;
	// every side track has less priority
	newRoad->dir = left ? Direction::L : Direction::R;
	newRoad->out = Direction::F;

	newRoadL->time = previous->segment->type != RoadType::main ? previous->time +FMath::Rand() % 3: (previous->time + 1);
	newRoadL->roadLength = (previous->segment->type == RoadType::main && newType != RoadType::main) ? 1 : previous->roadLength+1;
	newRoadL->previous = previous;

	addVertices(newRoad);
	queue.push(newRoadL);
	allsegments.push_back(newRoadL);

}

void ASpawner::addExtensions(std::priority_queue<logicRoadSegment*, std::deque<logicRoadSegment*>, roadComparator> &queue, logicRoadSegment* current, std::vector<logicRoadSegment*> &allsegments) {
	FVector tangent = current->segment->end - current->segment->start;
	if (current->segment->type == RoadType::main) {
		// on the main road
		if (current->roadLength < maxMainRoadLength)
			addRoadForward(queue, current, allsegments);

		if (randFloat() < mainRoadBranchChance) {
			if (randFloat() < 0.1f) {
				addRoadSide(queue, current, true, 4.0f, allsegments, RoadType::main);
			}

			else if (randFloat() < 0.2f) {
				addRoadSide(queue, current, false, 4.0f, allsegments, RoadType::main);
			}
			else {
				if (randFloat() < 0.5f) {
					addRoadSide(queue, current, true, 2.0f, allsegments, RoadType::secondary);
				}
				if (randFloat() < 0.5f) {
					addRoadSide(queue, current, false, 2.0f, allsegments, RoadType::secondary);

				}
			}
		}
	}

	else if (current->segment->type == RoadType::secondary) {
		// side road
		if (current->roadLength < maxSecondaryRoadLength){
			addRoadForward(queue, current, allsegments);

		addRoadSide(queue, current, true, 2.0f, allsegments, RoadType::secondary);
		addRoadSide(queue, current, false, 2.0f, allsegments, RoadType::secondary);
		}
	}


}

TArray<FRoadSegment> ASpawner::determineRoadSegments()
{
	FVector origin;


	TArray<FRoadSegment*> determinedSegments;
	TArray<FRoadSegment> finishedSegments;

	//.

	// the first segment
	std::priority_queue<logicRoadSegment*, std::deque<logicRoadSegment*>, roadComparator> queue;


	std::vector<logicRoadSegment*> allSegments;
	logicRoadSegment* start = new logicRoadSegment();
	start->time = 0;
	FRoadSegment* startR = new FRoadSegment();
	startR->beginTangent = stepLength;
	startR->start = FVector(0, 0, 0);
	startR->end = startR->start + stepLength;
	startR->width = 4.0f;
	startR->type = RoadType::main;
	start->segment = startR;
	start->firstDegreeRot = FRotator(0, 0, 0);
	start->secondDegreeRot = FRotator(0, 0, 0);
	start->roadLength = 1;
	startR->out = Direction::F;
	startR->dir = Direction::F;
	addVertices(startR);

	queue.push(start);

	// loop for everything else

	while (queue.size() > 0 && determinedSegments.Num() < length) {
		logicRoadSegment* current = queue.top();
		queue.pop();
		if (placementCheck(determinedSegments, current)) {
			determinedSegments.Add(current->segment);

			//UE_LOG(LogTemp, Warning, TEXT("CURRENT SEGMENT START X %f"), current->segment->start.X);
			addExtensions(queue, current, allSegments);
		}
		//if (current->segment->dir != Direction::F) {
		//	// wasnt forward, have to adjust previous road
		//	logicRoadSegment* prev = current->previous;
		//	if (prev->segment->out == Direction::F) {
		//		if (current->segment->dir == Direction::L) {
		//			prev->segment->out = Direction::LF;
		//		}
		//		else if (current->segment->dir == Direction::R) {
		//			prev->segment->out = Direction::FR;
		//		}
		//	}
		//	else {
		//		prev->segment->out = Direction::LFR;
		//	}

		//}
	}

	for (FRoadSegment* f : determinedSegments) {
		finishedSegments.Add(*f);
	}

	for (logicRoadSegment* s : allSegments){
		delete(s->segment);
		delete(s);
	}
	

	return finishedSegments	;
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

