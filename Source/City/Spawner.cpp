// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "Spawner.h"



// Sets default values
ASpawner::ASpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}



int convertToMapIndex(int x, int y) {
	return x * 10000 + y;
}

void addRoadToMap(TMap <int, TArray<FRoadSegment*>*> &map, FRoadSegment* current, float intervalLength) {
	FVector middle = (current->end - current->start) / 2 + current->start;
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
	FVector middle = (current->end - current->start) / 2 + current->start;
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

float randFloat() {
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

void ASpawner::addVertices(FRoadSegment* road) {
	road->v1 = road->start + FRotator(0, 90, 0).RotateVector(road->beginTangent) * standardWidth*road->width / 2;
	road->v2 = road->start - FRotator(0, 90, 0).RotateVector(road->beginTangent) * standardWidth*road->width / 2;
	FVector endTangent = road->end - road->start;
	endTangent.Normalize();
	road->v3 = road->end + FRotator(0, 90, 0).RotateVector(endTangent) * standardWidth*road->width / 2;
	road->v4 = road->end - FRotator(0, 90, 0).RotateVector(endTangent) * standardWidth*road->width / 2;

}






void getMinMax(float &min, float &max, FVector tangent, FVector v1, FVector v2, FVector v3, FVector v4) {
	float res = FVector::DotProduct(tangent, v1);
	min = res;
	max = res;
	res = FVector::DotProduct(tangent, v2);
	min = std::min(min, res);
	max = std::max(max, res);
	res = FVector::DotProduct(tangent, v3);
	min = std::min(min, res);
	max = std::max(max, res);
	res = FVector::DotProduct(tangent, v4);
	min = std::min(min, res);
	max = std::max(max, res);

	//min = std::abs(min);
	//max = std::abs(max);
}


struct Point {
	float x;
	float y;
};



FVector intersection(FVector p1, FVector p2, FVector p3, FVector p4) {
	// Store the values for fast access and easy
	// equations-to-code conversion
	float x1 = p1.X, x2 = p2.X, x3 = p3.X, x4 = p4.X;
	float y1 = p1.Y, y2 = p2.Y, y3 = p3.Y, y4 = p4.Y;

	float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	// If d is zero, there is no intersection
	if (d == 0) return FVector{0,0,0};

	// Get the x and y
	float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
	float x = (pre * (x3 - x4) - (x1 - x2) * post) / d;
	float y = (pre * (y3 - y4) - (y1 - y2) * post) / d;

	// Check if the x and y coordinates are within both lines
	if (x < std::min(x1, x2) || x > std::max(x1, x2) ||
		x < std::min(x3, x4) || x > std::max(x3, x4)) return FVector{0,0,0};
	if (y < std::min(y1, y2) || y > std::max(y1, y2) ||
		y < std::min(y3, y4) || y > std::max(y3, y4)) return FVector{0,0,0};

	// Return the point of intersection
	FVector ret{ x,y,0 };
	return ret;
}

FVector NearestPointOnLine(FVector linePnt, FVector lineDir, FVector pnt)
{
	lineDir.Normalize();//this needs to be a unit vector
	FVector v = pnt - linePnt;
	float d = FVector::DotProduct(v, lineDir);
	return linePnt + lineDir * d;
}

bool ASpawner::placementCheck(TArray<FRoadSegment*> &segments, logicRoadSegment* current, TMap <int, TArray<FRoadSegment*>*> &map){

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


	float stepDist = stepLength.Size();
	//getRelevantRoads();
	TArray<TArray<FRoadSegment*>*> relevant = getRelevantSegments(map, current->segment, stepLength.Size() / 2);

	//UE_LOG(LogTemp, Log, TEXT("relevant elements: %i"), relevant.Num());

	float maxAttachDistanceSquared = FMath::Pow(maxAttachDistance, 2);
	for (TArray<FRoadSegment*>* s : relevant) {
		for (FRoadSegment* f : (*s)) {

			// ()
			FVector nearest = NearestPointOnLine(f->start, f->end - f->start, current->segment->end);
			if (FVector::DistSquared(nearest, current->segment->end) < maxAttachDistanceSquared) {
				current->segment->end = nearest;
				addVertices(current->segment);
				continue;
			}


			// try all tangents (there are four since both shapes are rectangles)

			float min;
			float max;
			// return true means no overlap, there are in fact 8 cases but well be alright only testing for 4 (fight me)
			getMinMax(min, max, myTangent1, f->v1, f->v2, f->v3, f->v4);
			if (std::max(min, myMin1) >= std::min(max, myMax1) - collisionLeniency) {
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

			// OVERLAP!

			FVector newE = intersection(current->segment->start, current->segment->end, f->start, f->end);

			if (newE.X == 0) {
				//continue;
				return false;
			}
			if (FVector::Dist(newE, current->segment->start) < stepLength.Size()/2) {
				// new road is too small
				return false;
			}
			current->segment->end = newE;
			addVertices(current->segment);


		}
	}

	return true;
	

}


void ASpawner::addRoadForward(std::priority_queue<logicRoadSegment*, std::deque<logicRoadSegment*>, roadComparator> &queue, logicRoadSegment* previous, std::vector<logicRoadSegment*> &allsegments) {
	FRoadSegment* prevSeg = previous->segment;
	logicRoadSegment* newRoadL = new logicRoadSegment();
	FRoadSegment* newRoad = new FRoadSegment();

	newRoadL->secondDegreeRot = previous->secondDegreeRot + FRotator(0, (prevSeg->type == RoadType::main ? changeIntensity : secondaryChangeIntensity)*(randFloat() - 0.5f), 0);
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

	newRoadL->secondDegreeRot = FRotator(0, (prevSeg->type == RoadType::main ? changeIntensity : secondaryChangeIntensity)*(randFloat() - 0.5f), 0);
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

	newRoadL->time = previous->segment->type != RoadType::main ? previous->time +FMath::Rand() % 2: (previous->time + 1);
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
			if (randFloat() < 0.15f) {
				addRoadSide(queue, current, true, 3.0f, allsegments, RoadType::main);
			}

			else if (randFloat() < 0.15f) {
				addRoadSide(queue, current, false, 3.0f, allsegments, RoadType::main);
			}
			else {
				if (randFloat() < secondaryRoadBranchChance) {
					addRoadSide(queue, current, true, 1.0f, allsegments, RoadType::secondary);
				}
				if (randFloat() < secondaryRoadBranchChance) {
					addRoadSide(queue, current, false, 1.0f, allsegments, RoadType::secondary);

				}
			}
		}
	}

	else if (current->segment->type == RoadType::secondary) {
		// side road
		if (current->roadLength < maxSecondaryRoadLength){
			addRoadForward(queue, current, allsegments);

		addRoadSide(queue, current, true, 1.0f, allsegments, RoadType::secondary);
		addRoadSide(queue, current, false, 1.0f, allsegments, RoadType::secondary);
		}
	}


}

TArray<FRoadSegment> ASpawner::determineRoadSegments()
{
	FVector origin;


	TArray<FRoadSegment*> determinedSegments;
	TArray<FRoadSegment> finishedSegments;


	std::priority_queue<logicRoadSegment*, std::deque<logicRoadSegment*>, roadComparator> queue;


	std::vector<logicRoadSegment*> allSegments;
	logicRoadSegment* start = new logicRoadSegment();
	start->time = 0;
	FRoadSegment* startR = new FRoadSegment();
	startR->beginTangent = stepLength;
	startR->start = FVector(0, 0, 0);
	startR->end = startR->start + stepLength;
	startR->width = 3.0f;
	startR->type = RoadType::main;
	start->segment = startR;
	start->firstDegreeRot = FRotator(0, 0, 0);
	start->secondDegreeRot = FRotator(0, 0, 0);
	start->roadLength = 1;
	startR->out = Direction::F;
	startR->dir = Direction::F;
	addVertices(startR);

	queue.push(start);

	// map for faster comparisons
	TMap <int, TArray<FRoadSegment*>*> segmentsOrganized;

	// loop for everything else

	while (queue.size() > 0 && determinedSegments.Num() < length) {
		logicRoadSegment* current = queue.top();
		queue.pop();
		if (placementCheck(determinedSegments, current, segmentsOrganized)) {
			determinedSegments.Add(current->segment);
			addRoadToMap(segmentsOrganized, current->segment, 10000);

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

void ASpawner::buildRoads(TArray<FRoadSegment> segments) {
	//spline
	for (USplineMeshComponent* s : splineComponents) {
		if (s)
			s->DestroyComponent();
	}
	


	for (FRoadSegment f : segments) {
		//UE_LOG(LogTemp, Warning, TEXT("Placing road..."));
		USplineMeshComponent *s = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		s->SetStaticMesh(meshRoad);
		s->SetCastShadow(false);
		s->SetStartScale(FVector2D(f.width, 1));
		s->SetEndScale(FVector2D(f.width, 1));
		s->SetStartAndEnd(f.start + FVector(0, 0, 50), f.beginTangent, f.end  + FVector(0, 0, 50), f.end - f.start, true);
		splineComponents.Add(s);
	}

}

TArray<FPolygon> ASpawner::getBuildingPolygons(TArray<FRoadSegment> segments) {
	TArray<FPolygon> shapes;
	TSet<FPolygon> unConnected;

	for (FRoadSegment f : segments) {
		// two collision segments for every road
		FVector tangent = f.end - f.start;
		FVector extraLength = tangent / 16;
		tangent.Normalize();
		FVector sideOffset = FRotator(0, 90, 0).RotateVector(tangent)*(standardWidth/2*f.width + 100);
		FPolygon left;
		left.points.Add(f.start + sideOffset - extraLength);
		left.points.Add(f.end + sideOffset + extraLength);
		FPolygon right;
		right.points.Add(f.start - sideOffset - extraLength);
		right.points.Add(f.end - sideOffset + extraLength);

		float halfSquared = FMath::Pow(stepLength.Size() / 2, 2);

		FVector middle = (left.points[1] - left.points[0]) / 2 + left.points[0];

		for (int i = 0; i < shapes.Num(); i++) {
			FPolygon &pol = shapes[i];
			// check lines
			for (int k = 1; k < pol.points.Num(); k++) {
				FVector res = intersection(left.points[0], left.points[1], pol.points[k - 1], pol.points[k]);
				if (res.X != 0 && FVector::DistSquared(middle, res) < halfSquared){
					// intersection
					pol.points.Append(left.points);
					goto outOfLoop1;

				}
			}
		}
		shapes.Add(left);
		outOfLoop1:

		middle = (right.points[1] - right.points[0]) / 2 + right.points[0];

		for (int i = 0; i < shapes.Num(); i++) {
			FPolygon &pol = shapes[i];
			// check lines
			for (int k = 1; k < pol.points.Num(); k++) {
				FVector res = intersection(right.points[0], right.points[1], pol.points[k - 1], pol.points[k]);
				if (res.X != 0 && FVector::DistSquared(middle, res) < halfSquared) {
					// intersection
					pol.points.Append(right.points);
					goto outOfLoop2;

				}
			}
		}
		shapes.Add(right);
	outOfLoop2:
		;
		//unConnected
	}
	return shapes;


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

