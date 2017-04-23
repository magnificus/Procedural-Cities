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

	current->segment->end += tangent1 * maxAttachDistance;
	addVertices(current->segment);
	TArray<FVector> vert1;
	vert1.Add(current->segment->v1);
	vert1.Add(current->segment->v2);
	vert1.Add(current->segment->v3);
	vert1.Add(current->segment->v4);

	bool hadCollision = false;
	for (FRoadSegment* f : segments){

		TArray<FVector> tangents;

		// can't be too close to another segment
		if (FVector::Dist((f->end - f->start) / 2 + f->start, (current->segment->end - current->segment->start) / 2 + current->segment->start) < 7000) {
			return false;
		}


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
			hadCollision = true;
			FVector newE = intersection(current->segment->start, current->segment->end, f->start, f->end);
			current->time = 100000;
			//return false;
			if (newE.X == 0) {
				// the lines themselves are not colliding, its an edge case
				FVector closest = NearestPointOnLine(f->start, f->end - f->start, current->segment->end);
				if (FVector::Dist(closest, current->segment->end) < maxAttachDistance) {
					current->segment->end = closest;
					addVertices(current->segment);
				}
				continue;
			}
			current->segment->end = newE;
			addVertices(current->segment);
			// new road cant be too short
			if (FVector::Dist(current->segment->start, current->segment->end) < primaryStepLength.Size()/5) {
				return false;
			}

		}

		//FVector closestPoint = NearestPointOnLine(f->start, f->end - f->start, current->segment->end);
		//bool isOnInterval = std::abs(FVector::Dist(f->start, closestPoint) + FVector::Dist(f->end, closestPoint) - FVector::Dist(f->start, f->end)) < 0.01;
		//if (!isOnInterval) {
		//	closestPoint = FVector::DistSquared(f->start, current->segment->end) < FVector::DistSquared(f->end, current->segment->end) ? f->start : f->end;
		//}
		//if (FVector::Dist(closestPoint, current->segment->end) < maxAttachDistance) {
		//	current->segment->end = closestPoint;
		//	addVertices(current->segment);
		//}


	}
	if (!hadCollision) {
		current->segment->end -= maxAttachDistance*tangent1;
		addVertices(current->segment);
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
			if (current->previous && FVector::Dist(current->previous->segment->end, current->segment->start) < 100)
				current->previous->segment->roadInFront = true;
			addRoadToMap(segmentsOrganized, current->segment, primaryStepLength.Size() / 2);

			//UE_LOG(LogTemp, Warning, TEXT("CURRENT SEGMENT START X %f"), current->segment->start.X);
			addExtensions(queue, current, allSegments);
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

struct Line {
	FVector p1;
	FVector p2;
};

struct LinkedLine {
	Line line;
	bool buildLeft;
	FVector point = FVector(0.0f, 0.0f, 0.0f);
	LinkedLine* parent;
	LinkedLine* child;
};

void invertAndParents(LinkedLine* line) {
	TSet<LinkedLine*> taken;
	while (line && !taken.Contains(line)) {
		taken.Add(line);
		//if (line->parent)
		//	line->point = line->parent->point;
		//else
			line->point = FVector(0.0f, 0.0f, 0.0f);
		FVector temp = line->line.p1;
		line->line.p1 = line->line.p2;
		line->line.p2 = temp;
		LinkedLine* prevC = line->child;
		line->child = line->parent;
		line->parent = prevC;
		line->buildLeft = !line->buildLeft;

		line = line->child;

	}
}

void invertAndChildren(LinkedLine* line) {
	TSet<LinkedLine*> taken;
	while (line && !taken.Contains(line)) {
		taken.Add(line);
		//if (line->child)
		//	line->point = line->child->point;
		//else
			line->point = FVector(0.0f, 0.0f, 0.0f);

		FVector temp = line->line.p1;
		line->line.p1 = line->line.p2;
		line->line.p2 = temp;
		LinkedLine* prevC = line->child;
		line->child = line->parent;
		line->parent = prevC;
		line->buildLeft = !line->buildLeft;

		line = line->parent;
	}
}

void decidePolygonFate(TArray<FRoadSegment> &segments, LinkedLine* &inLine, TArray<LinkedLine*> &lines, bool allowSplit, float extraRoadLen)
{
	float len = FVector::Dist(inLine->line.p1, inLine->line.p2);
	float middleOffset = 600;
	float width = 1000;

	if (len < 1000) {
		delete inLine;
		return;
	}

	// split lines blocking roads

	FVector tangent1 = inLine->line.p2 - inLine->line.p1;
	tangent1.Normalize();
	FVector tangent2 = FRotator(0, 90, 0).RotateVector(tangent1);
	TArray<FVector> lineVertices;
	FVector v1 = inLine->line.p1 + width * tangent2;
	FVector v2 = inLine->line.p1 - width * tangent2;
	FVector v3 = inLine->line.p2 + width * tangent2;
	FVector v4 = inLine->line.p2 - width * tangent2;
	lineVertices.Add(v1);
	lineVertices.Add(v2);
	lineVertices.Add(v3);
	lineVertices.Add(v4);

	for (FRoadSegment f : segments) {
		FVector tangent = (f.end - f.start);
		tangent.Normalize();
		FVector intSec = intersection(f.start - tangent*extraRoadLen, f.end + tangent*extraRoadLen, inLine->line.p1, inLine->line.p2);
		int counter = 0;
		while (intSec.X != 0.0f && counter++ < 5) {
			if (!allowSplit) {
				delete inLine;
				return;
			}

			FVector altTangent = FRotator(0, 90, 0).RotateVector(tangent);
			LinkedLine* newP = new LinkedLine();
			if (FVector::DistSquared(intSec, inLine->line.p1) > FVector::DistSquared(intSec + altTangent * middleOffset, inLine->line.p1)) {
				// 1 got closer, so use 1
				newP->line.p1 = inLine->line.p1;
				newP->line.p2 = intSec + altTangent * middleOffset;
				inLine->line.p1 = intSec - altTangent * middleOffset;
			}
			else {
				// 1 got further away, use 2 instead
				newP->line.p1 = intSec + altTangent * middleOffset;
				newP->line.p2 = inLine->line.p2;
				inLine->line.p2 = intSec - altTangent * middleOffset;
			}
			decidePolygonFate(segments, newP,lines, true, extraRoadLen);
			intSec = intersection(f.start - tangent * extraRoadLen, f.end + tangent * extraRoadLen, inLine->line.p1, inLine->line.p2);
			//return;
		}
	}
	len = FVector::Dist(inLine->line.p1, inLine->line.p2);
	if (len < 1000) {
		delete inLine;
		return;
	}

	for (int i = 0; i < lines.Num(); i++) {
		LinkedLine *pol = lines[i];
		// check lines

			TArray<FVector> tangents;
			FVector tangent3 = pol->line.p2 - pol->line.p1;
			tangent3.Normalize();

			FVector tangent4 = FRotator(0, 90, 0).RotateVector(tangent3);

			tangents.Add(tangent1);
			tangents.Add(tangent2);
			tangents.Add(tangent3);
			tangents.Add(tangent4);

			TArray<FVector> lineVertices2;
			lineVertices2.Add(pol->line.p1 + tangent4 * width);
			lineVertices2.Add(pol->line.p1- tangent4 * width);
			lineVertices2.Add(pol->line.p2 + tangent4 * width);
			lineVertices2.Add(pol->line.p2 - tangent4 * width);

			if (testCollision(tangents, lineVertices, lineVertices2, 0)) {

				FVector res = intersection(pol->line.p1, pol->line.p2, inLine->line.p1, inLine->line.p2);
				if (res.X != 0.0f) {
					// on the previous line, is the collision close to the end?
					if (FVector::Dist(pol->line.p1, res) > FVector::Dist(pol->line.p2, res)) {
						// on the new line, collision end?
						if (FVector::Dist(inLine->line.p1, res) > FVector::Dist(inLine->line.p2, res)) {
							// then flip
							invertAndParents(inLine);
						}
						else {

						}
						inLine->parent = pol;
						pol->child = inLine;
						pol->point = res;
						inLine->buildLeft = pol->buildLeft;

					} 	
					// so the new line is maybe the master
					else{
						// on inLine, collision end?
						if (FVector::Dist(inLine->line.p1, res) > FVector::Dist(inLine->line.p2, res)) {
							pol->parent = inLine;
							inLine->child = pol;

						}
						else {
							// otherwise flip me
							invertAndChildren(inLine);
							inLine->child = pol;
							pol->parent = inLine;
						}
						pol->buildLeft = inLine->buildLeft;
						inLine->point = res;

					}
				}else {
					// continous road

					//if (FVector::Dist(pol->line.p1, res) > FVector::Dist(pol->line.p2, res)) {
					//	inLine->parent = pol;
					//	pol->child = inLine;
					//	inLine->point = res;
					//}
					//else {
					//	pol->parent = inLine;
					//	inLine->child = pol;
					//	pol->point = res;
					//}
					
				}



			}
	}
	lines.Add(inLine);
	return;

}

struct PolygonPoint {
	FPolygon* pol;
	FVector point;
};

TArray<FMetaPolygon> ASpawner::getBuildingPolygons(TArray<FRoadSegment> segments) {

	TArray<LinkedLine*> lines;
	// get coherent polygons
	for (FRoadSegment f : segments) {
		// two collision segments for every road
		FVector tangent = f.end - f.start;
		tangent.Normalize();
		FVector extraLength = tangent * 950;
		FVector sideOffset = FRotator(0, 90, 0).RotateVector(tangent)*(standardWidth / 2 * f.width);
		LinkedLine* left = new LinkedLine();
		left->line.p1 = f.start + sideOffset - extraLength;
		left->line.p2 = f.end + sideOffset + extraLength;
		left->buildLeft = true;
		LinkedLine* right = new LinkedLine();
		right->line.p1 = f.start - sideOffset - extraLength;
		right->line.p2 = f.end - sideOffset + extraLength;
		right->buildLeft = false;


		float extraRoadLen = 950;


		decidePolygonFate(segments, left, lines, true, extraRoadLen);
		//if (!f.roadInFront) {
		//	LinkedLine* inFront = new LinkedLine();
		//	inFront->line.p1 = f.end + sideOffset*1.5;
		//	inFront->line.p2 = f.end - sideOffset*1.5;
		//	decidePolygonFate(segments, inFront, lines, false, 0);
		//}
		decidePolygonFate(segments, right, lines, true, extraRoadLen);
	}

	TSet<LinkedLine*> remaining;
	remaining.Append(lines);

	TArray<FMetaPolygon> polygons;
	int count = 0;
	// build the actual polýgons from the linked structures
	while (remaining.Num() > 0){
		TSet<LinkedLine*> taken;
		UE_LOG(LogTemp, Log, TEXT("remaining: %i"), remaining.Num());
		auto it = remaining.CreateIterator();
		LinkedLine* curr = *it;
		taken.Add(curr);
		while (curr->parent && remaining.Contains(curr->parent) && !taken.Contains(curr->parent)) {
			curr = curr->parent;
			taken.Add(curr);
		}
		// now curr is top dog
		FMetaPolygon f;
		f.buildLeft = curr->buildLeft;
		f.points.Add(curr->line.p1);
		f.points.Add(curr->point.X != 0.0f ? curr->point : curr->line.p2);

		taken.Empty();
		taken.Add(curr);
		remaining.Remove(curr);
		while (curr->child && !taken.Contains(curr->child)) {
			curr = curr->child;
			f.points.Add(curr->point.X != 0.0f ? curr->point : curr->line.p2);
			remaining.Remove(curr);
			taken.Add(curr);
		}
		if (curr->child && taken.Contains(curr->child)) {
			f.points.RemoveAt(0);
			f.points.EmplaceAt(0, intersection(curr->line.p1, curr->line.p2, curr->child->line.p1, curr->child->line.p2));
			//f.points.Add(intersection(curr->line.p1, curr->line.p2, curr->child->line.p1, curr->child->line.p2));
			f.open = false;
		}
		polygons.Add(f);

	}

	// these roads shouldn't exist, so this is mainly to highlight errors
	for (int i = 0; i < polygons.Num(); i++) {
		FMetaPolygon f = polygons[i];
		if (f.points.Num() < 3) {
			polygons.RemoveAt(i);
			i--;
		}
	}

	// split polygons into habitable blocks
	float maxArea = 2000.0f;
	float minArea = -200.0f;
	
	TArray<FMetaPolygon> refinedPolygons;
	for (FMetaPolygon &p : polygons) {
		refinedPolygons.Append(p.refine(maxArea, minArea));
	}
	return refinedPolygons;


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