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

bool ASpawner::placementCheck(TArray<FRoadSegment*> &segments, logicRoadSegment* current){
	//if (current->segment->start.Size() > maxDist)
		//return false;

	for (FRoadSegment* f : segments) {
		// dumb collision check, just measure between centres
		float dist = FVector::Dist((f->end - f->start) / 2 + f->start, (current->segment->end - current->segment->start) / 2 + current->segment->start);
		if (dist < 10000) {
			return false;
		}
	}
	return true;
	

}

void ASpawner::addRoadForward(std::priority_queue<logicRoadSegment*> &queue, logicRoadSegment* previous, std::vector<logicRoadSegment*> &allsegments) {
	FRoadSegment* prevSeg = previous->segment;
	logicRoadSegment* newRoadL = new logicRoadSegment();
	FRoadSegment* newRoad = new FRoadSegment();

	newRoadL->secondDegreeRot = previous->secondDegreeRot + FRotator(0, changeIntensity*(randFloat() - 0.5f), 0);
	newRoadL->firstDegreeRot = previous->firstDegreeRot + newRoadL->secondDegreeRot;

	newRoad->start = prevSeg->end;
	newRoad->end = newRoad->start + newRoadL->firstDegreeRot.RotateVector(stepLength);
	newRoad->beginTangent = prevSeg->end - prevSeg->start;
	newRoad->width = prevSeg->width;
	newRoad->type = prevSeg->type;
	newRoadL->segment = newRoad;
	newRoadL->time = previous->time;
	newRoadL->roadLength = previous->roadLength + 1;
	newRoad->out = Direction::F;
	newRoad->dir = Direction::F;
	newRoadL->previous = previous;

	queue.push(newRoadL);
	allsegments.push_back(newRoadL);
	
}

void ASpawner::addRoadSide(std::priority_queue<logicRoadSegment*> &queue, logicRoadSegment* previous, bool left, float width, std::vector<logicRoadSegment*> &allsegments, RoadType newType) {
	FRoadSegment* prevSeg = previous->segment;
	logicRoadSegment* newRoadL = new logicRoadSegment();
	FRoadSegment* newRoad = new FRoadSegment();

	newRoadL->secondDegreeRot = FRotator(0, changeIntensity*(randFloat() - 0.5f), 0);
	FRotator newRotation = left ? FRotator(0, 270, 0) : FRotator(0, 90, 0);
	newRoadL->firstDegreeRot = previous->firstDegreeRot + newRoadL->secondDegreeRot + newRotation;

	FVector startOffset = !left ? FVector(0, 0, 0) : newRotation.RotateVector(FVector(0, -standardWidth, 0));
	newRoad->start = prevSeg->start + (prevSeg->end - prevSeg->start) / 2;// + startOffset;
	newRoad->end = newRoad->start + newRoadL->firstDegreeRot.RotateVector(stepLength);
	newRoad->beginTangent = newRoad->end - newRoad->start;
	newRoad->width = width;
	newRoad->type = newType;
	newRoadL->segment = newRoad;
	// every side track has less priority
	newRoad->dir = left ? Direction::L : Direction::R;
	newRoad->out = Direction::F;
	newRoadL->time = previous->time + 1;
	newRoadL->roadLength = previous->segment->type == RoadType::main ? 1 : previous->roadLength+1;
	newRoadL->previous = previous;

	queue.push(newRoadL);
	allsegments.push_back(newRoadL);

}

void ASpawner::addExtensions(std::priority_queue<logicRoadSegment*> &queue, logicRoadSegment* current, std::vector<logicRoadSegment*> &allsegments) {
	FVector tangent = current->segment->end - current->segment->start;
	if (current->segment->type == RoadType::main) {
		// on the main road, can add extensions
		// forward
		if (current->roadLength < maxMainRoadLength)
			addRoadForward(queue, current, allsegments);

		if (randFloat() < mainRoadBranchChance) {
			if (randFloat() < 0.2f) {
				addRoadSide(queue, current, true, 4.0f, allsegments, RoadType::main);
			}

			else if (randFloat() < 0.5f) {
				addRoadSide(queue, current, true, 2.0f, allsegments, RoadType::secondary);
			}
			else {
				addRoadSide(queue, current, false, 2.0f, allsegments, RoadType::secondary);

			}
		}
		//forward.
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
	std::priority_queue<logicRoadSegment*> queue;


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
		if (current->segment->dir != Direction::F) {
			// wasnt forward, have to adjust previous road
			logicRoadSegment* prev = current->previous;
			if (prev->segment->out == Direction::F) {
				if (current->segment->dir == Direction::L) {
					prev->segment->out = Direction::LF;
				}
				else if (current->segment->dir == Direction::R) {
					prev->segment->out = Direction::FR;
				}
			}
			else {
				prev->segment->out = Direction::LFR;
			}

		}
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

