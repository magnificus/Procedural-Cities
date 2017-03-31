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

bool ASpawner::placementCheck(TArray<FRoadSegment> &segments, logicRoadSegment* current){
	if (current->segment->start.Size() > maxDist)
		return false;

	for (FRoadSegment f : segments) {
		// dumb collision check
		float dist = FVector::Dist((f.end - f.start) / 2 + f.start, (current->segment->end - current->segment->start) / 2 + current->segment->start);
		if (dist < 800) {
			return false;
		}
	}
	return true;
	

}

void ASpawner::addRoadForward(std::priority_queue<logicRoadSegment*> &queue, logicRoadSegment* previous) {
	FRoadSegment* prevSeg = previous->segment;
	logicRoadSegment* newRoadL = new logicRoadSegment();
	FRoadSegment* newRoad = new FRoadSegment();

	newRoadL->secondDegreeRot = previous->secondDegreeRot + FRotator(0, changeIntensity*(randFloat() - 0.5f), 0);
	newRoadL->firstDegreeRot = previous->firstDegreeRot + newRoadL->secondDegreeRot;

	newRoad->start = prevSeg->end;
	newRoad->end = newRoad->start + newRoadL->firstDegreeRot.RotateVector(stepLength);
	newRoad->beginTangent = prevSeg->end - prevSeg->start;
	newRoad->width = prevSeg->width;
	newRoadL->segment = newRoad;
	newRoadL->time = previous->time;
	newRoadL->roadLength = previous->roadLength + 1;
	queue.push(newRoadL);
	
}

void ASpawner::addRoadSide(std::priority_queue<logicRoadSegment*> &queue, logicRoadSegment* previous, bool left, float width) {
	FRoadSegment* prevSeg = previous->segment;
	logicRoadSegment* newRoadL = new logicRoadSegment();
	FRoadSegment* newRoad = new FRoadSegment();

	newRoadL->secondDegreeRot = FRotator(0, changeIntensity*(randFloat() - 0.5f), 0);
	FRotator newRotation = left ? FRotator(0, 270, 0) : FRotator(0, 90, 0);
	newRoadL->firstDegreeRot = previous->firstDegreeRot + newRoadL->secondDegreeRot + newRotation;

	FVector startOffset = left ?  FVector(0, 0, 0) : newRoadL->firstDegreeRot.RotateVector(FVector(200, 0, 0));
	newRoad->start = (prevSeg->end - prevSeg->start)/2 + prevSeg->start + startOffset;
	newRoad->end = newRoad->start + newRoadL->firstDegreeRot.RotateVector(stepLength);
	newRoad->beginTangent = newRoad->end - newRoad->start;
	newRoad->width = width;
	newRoadL->segment = newRoad;
	newRoadL->time = previous->time+1;
	newRoadL->roadLength = 1;
	queue.push(newRoadL);
}

void ASpawner::addExtensions(std::priority_queue<logicRoadSegment*> &queue, logicRoadSegment* current) {
	FVector tangent = current->segment->end - current->segment->start;
	if (current->segment->width == 2.0f) {
		// on the main road, can add extensions

		// forward
		if (current->roadLength < maxMainRoadLength)
			addRoadForward(queue, current);

		if (randFloat() < mainRoadBranchChance) {
			if (randFloat() < 0.5f) {
				addRoadSide(queue, current, true, 1.5f);
			}
			else {
				addRoadSide(queue, current, false, 1.5f);

			}
		}
		//forward.
	}

	else if (current->segment->width == 1.5f) {
		// side road
		// forward
		if (current->roadLength < maxSecondaryRoadLength){
			addRoadForward(queue, current);

		//if (randFloat() < secondaryRoadBranchChance) {
		//	if (randFloat() < 0.5f) {
		addRoadSide(queue, current, true, 1.0f);
		//	}
		//	else {
		addRoadSide(queue, current, false, 1.0f);

		//	}
		}
	}
	else {
		if (current->roadLength < maxTertiaryLength)
			addRoadForward(queue, current);

		addRoadSide(queue, current, true, 1.0f);
		addRoadSide(queue, current, false, 1.0f);
	}

	// otherwise, only forward

}

TArray<FRoadSegment> ASpawner::determineRoadSegments()
{
	FVector origin;

	TArray<FRoadSegment> determinedSegments;

	//.

	// the first segment
	std::priority_queue<logicRoadSegment*> queue;
	logicRoadSegment* start = new logicRoadSegment();
	start->time = 0;
	FRoadSegment* startR = new FRoadSegment();
	startR->beginTangent = stepLength;
	startR->start = FVector(0, 0, 0);
	startR->end = startR->start + stepLength;
	startR->width = 2.0f;
	start->segment = startR;
	start->firstDegreeRot = FRotator(0, 0, 0);
	start->secondDegreeRot = FRotator(0, 0, 0);
	start->roadLength = 1;
	queue.push(start);

	// loop for everything else

	while (queue.size() > 0 && determinedSegments.Num() < length) {
		logicRoadSegment* current = queue.top();
		queue.pop();
		if (placementCheck(determinedSegments, current)) {
			determinedSegments.Add(*(current->segment));

			//UE_LOG(LogTemp, Warning, TEXT("CURRENT SEGMENT START X %f"), current->segment->start.X);
			addExtensions(queue, current);
		}
		delete(current->segment);
		delete(current);
	}
	
	return determinedSegments;
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

