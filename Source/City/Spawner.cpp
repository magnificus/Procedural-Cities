// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
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


void ASpawner::addVertices(FRoadSegment* road) {
	road->v1 = road->start + FRotator(0, 90, 0).RotateVector(road->beginTangent) * standardWidth*road->width / 2;
	road->v2 = road->start - FRotator(0, 90, 0).RotateVector(road->beginTangent) * standardWidth*road->width / 2;
	FVector endTangent = road->end - road->start;
	endTangent.Normalize();
	road->v3 = road->end + FRotator(0, 90, 0).RotateVector(endTangent) * standardWidth*road->width / 2;
	road->v4 = road->end - FRotator(0, 90, 0).RotateVector(endTangent) * standardWidth*road->width / 2;

}




bool ASpawner::placementCheck(TArray<FRoadSegment*> &segments, logicRoadSegment* current, TMap <int, TArray<FRoadSegment*>*> &map){

	// use SAT collision between all roads
	FVector tangent1 = current->segment->end - current->segment->start;
	tangent1.Normalize();
	FVector tangent2 = FRotator(0, 90, 0).RotateVector(tangent1);

	
	TArray<FVector> vert1;
	vert1.Add(current->segment->v1);
	vert1.Add(current->segment->v2);
	vert1.Add(current->segment->v3);
	vert1.Add(current->segment->v4);

	float maxAttachDistanceSquared = FMath::Pow(maxAttachDistance, 2);
	//for (TArray<FRoadSegment*>* s : relevant) {
	//	for (FRoadSegment* f : (*s)) {
	for (FRoadSegment* f : segments){

		TArray<FVector> tangents;

		// can't be too close to another segment
		//if (FVector::Dist((f->end - f->start) / 2 + f->start, (current->segment->end - current->segment->start) / 2 + current->segment->start) < 1000) {
		//	return false;
		//}

		FVector tangent3 = f->end - f->start;
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
			FVector newE = intersection(current->segment->start, current->segment->end, f->start, f->end);
			//return false;
			if (newE.X == 0) {
				//continue;
				return false;
			}
			current->segment->end = newE;
			addVertices(current->segment);
			if (FVector::Dist(current->segment->start, current->segment->end) < 5000) {
				return false;
			}

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
	FVector stepLength = prevSeg->type == RoadType::main ? primaryStepLength : secondaryStepLength;

	newRoad->start = prevSeg->end;
	newRoad->end = newRoad->start + newRoadL->firstDegreeRot.RotateVector(stepLength);
	newRoad->beginTangent = prevSeg->end - prevSeg->start;
	newRoad->beginTangent.Normalize();
	newRoad->width = prevSeg->width;
	newRoad->type = prevSeg->type;
	newRoadL->segment = newRoad;
	newRoadL->time = previous->segment->type != RoadType::main ? previous->time + FMath::Rand() % 1 : previous->time;
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

	FVector stepLength = newType == RoadType::main ? primaryStepLength : secondaryStepLength;
	FVector startOffset = newRoadL->firstDegreeRot.RotateVector(FVector(standardWidth*previous->segment->width / 2, 0, 0));
	newRoad->start = /*prevSeg->end - (standardWidth * (prevSeg->end - prevSeg->start).Normalize()/2) + startOffset; */prevSeg->start + (prevSeg->end - prevSeg->start) / 2 + startOffset;
	newRoad->end = newRoad->start + newRoadL->firstDegreeRot.RotateVector(stepLength);
	newRoad->beginTangent = newRoad->end - newRoad->start;
	newRoad->beginTangent.Normalize();
	newRoad->width = width;
	newRoad->type = newType;
	newRoadL->segment = newRoad;
	// every side track has less priority
	newRoad->dir = left ? Direction::L : Direction::R;
	newRoad->out = Direction::F;

	newRoadL->time = previous->segment->type != RoadType::main ? previous->time +FMath::Rand() % 1: (previous->time + 1);
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
					addRoadSide(queue, current, true, 2.0f, allsegments, RoadType::secondary);
				}
				if (randFloat() < secondaryRoadBranchChance) {
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

void ASpawner::generate() {
	for (auto It = splineComponents.CreateIterator(); It; It++)
	{
		(*It)->DestroyComponent();
	}
	for (APlotBuilder *pb : plots) {
		pb->Destroy();
	}
	splineComponents.Empty();
	plots.Empty();
	
	TArray<FRoadSegment> roadSegments = determineRoadSegments();
	buildRoads(roadSegments);
	TArray<FPolygon> polygons = getBuildingPolygons(roadSegments);
	//buildPolygons(polygons);
//	buildPlots(polygons);
	GetWorld()->GetGameViewport()->GetEngineShowFlags()->SetSplines(true);
	
}

//void ASpawner::OnConstruction(const FTransform & Transform) {
//	for (auto It = splineComponents.CreateIterator(); It; It++)
//	{
//		(*It)->DestroyComponent();
//	}
//	for (APlotBuilder *pb : plots) {
//		pb->Destroy();
//	}
//	splineComponents.Empty();
//	plots.Empty();
//
//	TArray<FRoadSegment> roadSegments = determineRoadSegments();
//	buildRoads(roadSegments);
//	TArray<FPolygon> polygons = getBuildingPolygons(roadSegments);
//	//buildPolygons(polygons);
//	buildPlots(polygons);
//	//GetWorld()->GetGameViewport()->GetEngineShowFlags()->SetSplines(true);
//
//}

//void ASpawner::BeginDestroy()
//{
//	for (auto It = splineComponents.CreateIterator(); It; It++)
//	{
//		if (*It)
//			(*It)->DestroyComponent();
//	}
//	for (auto It = plots.CreateIterator(); It; It++)
//	{
//		if (*It)
//			(*It)->Destroy();
//	}
//	Super::BeginDestroy();
//
//}

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
	startR->beginTangent = primaryStepLength;
	startR->start = FVector(0, 0, 0);
	startR->end = startR->start + primaryStepLength;
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
		for (auto It = splineComponents.CreateIterator(); It; It++)
		{
			if (*It)
				(*It)->DestroyComponent();
		}
		for (auto It = plots.CreateIterator(); It; It++)
		{
			if (*It)
				(*It)->Destroy();
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

void ASpawner::buildPolygons(TArray<FPolygon> polygons) {

		for (FPolygon p : polygons) {
			for (int i = 1; i < p.points.Num(); i++) {
				USplineMeshComponent *s = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
				s->SetStaticMesh(meshPolygon);
				s->SetCastShadow(false);
				s->SetStartAndEnd(p.points[i-1], p.points[i] - p.points[i-1], p.points[i] , p.points[i] - p.points[i - 1], true);
				splineComponents.Add(s);
			}

		}

}

void decidePolygonFate(TArray<FRoadSegment> &segments, FPolygon* inPol, TArray<FPolygon*> &shapes, TMap<FPolygon*, TArray<FPolygon*>*> &map)
{
	float len = FVector::Dist(inPol->points[1], inPol->points[0]);
	float middleOffset = 100;
	float extraRoadLen = 100;
	if (len < 1000) {
		return;
	}

	// split lines blocking roads

	float width = 100;
	FVector tangent1 = inPol->points[1] - inPol->points[0];
	tangent1.Normalize();
	FVector tangent2 = FRotator(0, 90, 0).RotateVector(tangent1);
	TArray<FVector> lineVertices;
	FVector v1 = inPol->points[0] + width * tangent2;
	FVector v2 = inPol->points[0] - width * tangent2;
	FVector v3 = inPol->points[1] + width * tangent2;
	FVector v4 = inPol->points[1] - width * tangent2;
	lineVertices.Add(v1);
	lineVertices.Add(v2);
	lineVertices.Add(v3);
	lineVertices.Add(v4);

	for (FRoadSegment f : segments) {
		FVector tangent = (f.end - f.start);
		tangent.Normalize();
		FVector intSec = intersection(f.start - tangent*extraRoadLen, f.end + tangent*extraRoadLen, inPol->points[0], inPol->points[1]);
		while (intSec.X != 0.0f) {

			FVector altTangent = FRotator(0, 90, 0).RotateVector(tangent);
			FPolygon* newP = new FPolygon();
			if (FVector::DistSquared(intSec, inPol->points[0]) > FVector::DistSquared(intSec + altTangent * middleOffset, inPol->points[0])) {
				// 0 got closer, so use 0
				newP->points.Add(inPol->points[0]);
				newP->points.Add(intSec + altTangent * middleOffset);
				inPol->points[0] = intSec - altTangent * middleOffset;
			}
			else {
				// 0 got further away, use 1 instead
				newP->points.Add(intSec + altTangent * middleOffset);
				newP->points.Add(inPol->points[1]);
				inPol->points[1] = intSec - altTangent * middleOffset;
			}
			decidePolygonFate(segments, newP,shapes, map);
			intSec = intersection(f.start - tangent * extraRoadLen, f.end + tangent * extraRoadLen, inPol->points[0], inPol->points[1]);
			//return;
		}
	}
	len = FVector::Dist(inPol->points[1], inPol->points[0]);
	if (len < 1000) {
		return;
	}
	bool shallAdd = true;
	TArray<FPolygon*> toRemove;
	//TArra

	bool hasPlaced = false;
	for (int i = 0; i < shapes.Num(); i++) {
		FPolygon *pol = shapes[i];
		// check lines
		for (int k = 1; k < pol->points.Num(); k++){

			TArray<FVector> tangents;
			FVector tangent3 = pol->points[k] - pol->points[k - 1];
			tangent3.Normalize();

			FVector tangent4 = FRotator(0, 90, 0).RotateVector(tangent3);

			tangents.Add(tangent1);
			tangents.Add(tangent2);
			tangents.Add(tangent3);
			tangents.Add(tangent4);

			TArray<FVector> lineVertices2;
			lineVertices2.Add(pol->points[k - 1] + tangent4 * width);
			lineVertices2.Add(pol->points[k - 1] - tangent4 * width);
			lineVertices2.Add(pol->points[k] + tangent4 * width);
			lineVertices2.Add(pol->points[k] - tangent4 * width);

			
			//FVector res = intersection(inPol->points[0], inPol->points[1], pol->points[k - 1], pol->points[k]);
			//FVector middlePol = (pol->points[k] - pol->points[k - 1]) / 2 + pol->points[k - 1];
			//float polLen = FVector::Dist(pol->points[k - 1], pol->points[k]) / 2;
			if (testCollision(tangents, lineVertices, lineVertices2, 0)){
				UE_LOG(LogTemp, Log, TEXT("testcollision: true"));

				if (!hasPlaced) {
					TArray<FPolygon*> *array = map[pol];
					array->Add(inPol);
					map.Add(inPol, array);
					hasPlaced = true;
				}
				else {
					auto prev = map[pol];
					TArray<FPolygon*> *array = map[inPol];
					array->Append(*prev);
					TArray<FPolygon*> *array2 = map[pol];
					for (FPolygon* f : *array2) {
						map->Add(f,inPol);
					}
					delete(prev);
				}

				//FVector res = intersection(inPol->points[0], inPol->points[1], pol->points[k - 1], pol->points[k]);
				//if (res.X != 0.0f) {
				//	FVector furthest = FVector::DistSquared(res, inPol->points[0]) > FVector::DistSquared(res, inPol->points[1]) ? inPol->points[0] : inPol->points[1];
				//	pol->points.Add(res);
				//	pol->points.Add(furthest);
				//}
				//else {
				//	FVector furthest = FVector::DistSquared(pol->points[k], inPol->points[0]) > FVector::DistSquared(pol->points[k], inPol->points[1]) ? inPol->points[0] : inPol->points[1];
				//	pol->points.Add(furthest);
				//	return;
				//	//int place = FVector::DistSquared(furthest, pol->points[0]) < FVector::DistSquared(furthest, pol->points[1]) < 
				//	//if ()
				//}

				//pol->points.Add(inPol->points[0]);
				//pol->points.Add(inPol->points[1]);
				//goto loopDone;
			//if (res.X != 0.0f) {
				// both polygons will retain at least one and lose at least one point
				// intersection
				//UE_LOG(LogTemp, Log, TEXT("collision! %s"), *res.ToString());


				// add extra point
				//if (toAppend == -1) {
				//	FVector toEmplace = FVector::DistSquared(inPol->points[0], res) < FVector::DistSquared(inPol->points[1], res) ? inPol->points[1] : inPol->points[0];

				//	if (FVector::DistSquared(pol->points[k - 1], res) < FVector::DistSquared(pol->points[k], res)) {
				//		pol->points.RemoveAt(k - 1);
				//		pol->points.EmplaceAt(k - 1, res);
				//		pol->points.EmplaceAt(k - 1, toEmplace);
				//		inFront = true;
				//	} else {
				//		pol->points.RemoveAt(k);
				//		pol->points.EmplaceAt(k, res);
				//		pol->points.EmplaceAt(k + 1, toEmplace);
				//		inFront = false;
				//	}
				//	toAppend = i;
				//	shallAdd = false;
				//}
				//else {
				//	// TODO take care of the order as well ;);););););;););)
				//	if (inFront) {
				//		pol->points.RemoveAt(pol->points.Num() - 1);
				//		pol->points.Add(res);
				//		shapes[toAppend]->points.RemoveAt(0);
				//		pol->points.Append(shapes[toAppend]->points);

				//		toRemove.Add(shapes[toAppend]);
				//		toAppend = i;

				//	}
				//	else {
				//		shapes[toAppend]->points.RemoveAt(shapes[toAppend]->points.Num() - 1);
				//		shapes[toAppend]->points.Add(res);
				//		pol->points.RemoveAt(0);
				//		shapes[toAppend]->points.Append(pol->points);

				//		toRemove.Add(pol);
				//	}

				//}

				//break;
				//return;

			}
			else {
				UE_LOG(LogTemp, Log, TEXT("testcollision: false"));

			}
		}
	}
//loopDone:
//	for (FPolygon* f : toRemove) {
//		shapes.Remove(f);
//		delete(f);
//	}
	//if (shallAdd) {
	//	shapes.Add(inPol);
	//}
	shapes.Add(inPol);
	if (!hasPlaced) {
		map.Add(inPol, new TArray<FPolygon*>());
		map[inPol]->Add(inPol);
	}
	return;

}

struct PolygonPoint {
	FPolygon* pol;
	FVector point;
};

FPolygon* beautify(TArray<FPolygon*> polygons) {

	TMap<FPolygon*, PolygonPoint> successors;
	TMap<FPolygon*, FPolygon*> parents;
	for (FPolygon* f : polygons){
		for (FPolygon* f2 : polygons) {
			UE_LOG(LogTemp, Log, TEXT("assembling..."));

			if (f == f2)
				continue;

			FVector res = intersection(f->points[0], f->points[1], f2->points[0], f2->points[1]);
			if (res.X != 0.0f) {
				successors.Add(f, PolygonPoint{ f2, res });
				parents.Add(f2, f);
				//break;
			}
			else {
				if (FVector::Dist(f2->points[0], f->points[1]) < 700) {
					successors.Add(f, PolygonPoint{ f2, f2->points[0] });
					parents.Add(f2, f);
					//break;
				}
				else if (FVector::Dist(f2->points[1], f->points[1]) < 700) {
					successors.Add(f, PolygonPoint{ f2, f2->points[0] });
					parents.Add(f2, f);
					//break;
				}
			}

				
		}
	}
	FPolygon* current = polygons[0];
	FPolygon** next = &current;
	while (next) {
		UE_LOG(LogTemp, Log, TEXT("here1"));

		current = *next;
		next = parents.Find(current);
	}
	next = &current;

	FPolygon* newPolygon = new FPolygon();
	newPolygon->buildLeft = polygons[0]->buildLeft;
	newPolygon->center = FVector(0, 0, 0);
	newPolygon->points.Add(current->points[0]);

	PolygonPoint *nextPP = successors.Find(current);
	while (nextPP) {
		UE_LOG(LogTemp, Log, TEXT("here2"));

		current = nextPP->pol;
		newPolygon->points.Add(nextPP->point);
		nextPP = successors.Find(current);
	}
	return newPolygon;
}

//void beautify(FPolygon *f) {
	// if two points are too close together, combine them using the one closest to the middle
	//return;
	//FVector center;
	//for (FVector v : f->points) {
	//	center += v;
	//}
	//center /= f->points.Num();

	//for (int i = 0; i < f->points.Num(); i++) {
	//	for (int j = i+1; j < f->points.Num(); j++) {
	//		if (FVector::Dist(f->points[i], f->points[j]) < 1000) {

	//			if (FVector::DistSquared(f->points[i], center) < FVector::DistSquared(f->points[j], center)) {
	//				f->points[j] = f->points[i];
	//			}
	//			else {
	//				f->points[i] = f->points[j];
	//			}
	//			int min = std::min(i, j);
	//			int max = std::max(i, j);
	//			// if the last and first point are very close together, we're dealing with a closed polygon
	//			if (max == f->points.Num() - 1 && min == 0) {
	//				UE_LOG(LogTemp, Log, TEXT("Closed polygon detected"));

	//				f->open = false;
	//			}
	//		}
	//	}
	//}
//}

TArray<FPolygon> ASpawner::getBuildingPolygons(TArray<FRoadSegment> segments) {
	TArray<FPolygon*> shapes;

	TMap<FPolygon*, TArray<FPolygon*>*> map;
	// get coherent polygons
	for (FRoadSegment f : segments) {
		// two collision segments for every road
		FVector tangent = f.end - f.start;
		tangent.Normalize();
		FVector extraLength = tangent * 700;
		FVector sideOffset = FRotator(0, 90, 0).RotateVector(tangent)*(standardWidth / 2 * f.width);
		FPolygon* left = new FPolygon();
		left->points.Add(f.start + sideOffset - extraLength);
		left->points.Add(f.end + sideOffset + extraLength);
		left->buildLeft = true;
		FPolygon* right = new FPolygon();;
		right->points.Add(f.start - sideOffset - extraLength);
		right->points.Add(f.end - sideOffset + extraLength);
		right->buildLeft = false;

		decidePolygonFate(segments, left, shapes, map);
		decidePolygonFate(segments, right, shapes, map);

	}

	// zip em together neatly
	auto it = map.CreateIterator();
	while (it) {



		TArray<FPolygon*> *pols = it->Value;




		FPolygon* res = beautify(*pols);
		shapes.Add(res);
		++it;
		UE_LOG(LogTemp, Log, TEXT("pollegögg!"));

	}

	//for (FPolygon *f : shapes) {
	//	beautify(f);
	//}
	//for (int i = 0; i < shapes.Num(); i++) {
	//	FPolygon* f = shapes[i];
	//	if (f->points.Num() < 3) {
	//		shapes.RemoveAt(i);
	//		i--;
	//		delete(f);
	//	}

	//}
	TArray<FPolygon> toReturn;
	for (FPolygon* f : shapes) {
		toReturn.Add(*f);
		delete(f);
	}
	return toReturn;


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

//void ASpawner::buildPlots(TArray<FPolygon> polygons) {
//	for (FPolygon p : polygons) {
//			// one house per segment
//			FActorSpawnParameters spawnInfo;
//			APlotBuilder* pb = GetWorld()->SpawnActor<APlotBuilder>(FVector(0,0,0), FRotator(0, 0, 0), spawnInfo);
//			FPlotPolygon ph;
//			ph.f = p;
//			//pb->pl
//			ph.population = 1;
//
//			pb->BuildPlot(ph);
//			plots.Add(pb);
//			//fh.
//			//h->placeHouse()
//		
//	}
//}