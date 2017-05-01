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




bool ASpawner::placementCheck(TArray<FRoadSegment*> &segments, logicRoadSegment* current, TMap <int, TArray<FRoadSegment*>*> &map){

	// use SAT collision between all roads
	FVector tangent1 = current->segment->p2 - current->segment->p1;
	tangent1.Normalize();
	FVector tangent2 = FRotator(0, 90, 0).RotateVector(tangent1);

	current->segment->p2 += tangent1 * maxAttachDistance;
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
		if (FVector::Dist((f->p2 - f->p1) / 2 + f->p1, (current->segment->p2 - current->segment->p1) / 2 + current->segment->p1) < 7000) {
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
			hadCollision = true;
			FVector newE = intersection(current->segment->p1, current->segment->p2, f->p1, f->p2);
			current->time = 100000;
			if (newE.X == 0) {
				// the lines themselves are not colliding, its an edge case
				FVector closest = NearestPointOnLine(f->p1, f->p2 - f->p1, current->segment->p2);
				if (FVector::Dist(closest, current->segment->p2) < maxAttachDistance) {
					current->segment->p2 = closest;
					addVertices(current->segment);
				}
				continue;
			}
			else {
				FVector tangent = newE - current->segment->p1;
				tangent.Normalize();
				float len = FVector::Dist(newE, current->segment->p2);
				current->segment->p2 += (len - standardWidth / 2) * tangent;
				current->segment->p2 = newE;
				addVertices(current->segment);
				current->segment->roadInFront = true;
				// new road cant be too short
				if (FVector::Dist(current->segment->p1, current->segment->p2) < primaryStepLength.Size() / 6) {
					return false;
				}
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
		current->segment->p2 -= maxAttachDistance*tangent1;
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

	newRoad->p1 = prevSeg->p2;
	newRoad->p2 = newRoad->p1 + newRoadL->firstDegreeRot.RotateVector(stepLength);
	newRoad->beginTangent = prevSeg->p2 - prevSeg->p1;
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
	newRoad->p1 = /*prevSeg->end - (standardWidth * (prevSeg->end - prevSeg->start).Normalize()/2) + startOffset; */prevSeg->p1 + (prevSeg->p2 - prevSeg->p1) / 2 + startOffset;
	newRoad->p2 = newRoad->p1 + newRoadL->firstDegreeRot.RotateVector(stepLength);
	newRoad->beginTangent = newRoad->p2 - newRoad->p1;
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
	FVector tangent = current->segment->p2 - current->segment->p1;
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
		if (current->roadLength < maxSecondaryRoadLength) {
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
	startR->p1 = FVector(0, 0, 0);
	startR->p2 = startR->p1 + primaryStepLength;
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
			if (current->previous && FVector::Dist(current->previous->segment->p2, current->segment->p1) < 100)
				current->previous->segment->roadInFront = true;
			addRoadToMap(segmentsOrganized, current->segment, primaryStepLength.Size() / 2);

			//UE_LOG(LogTemp, Warning, TEXT("CURRENT SEGMENT START X %f"), current->segment->start.X);
			addExtensions(queue, current, allSegments);
		}
	}



	//for (FRoadSegment* f2 : determinedSegments) {
	//	//if (f2->roadInFront)
	//	//	continue;
	//	FVector closestYet;
	//	float closestD = 1000000000.0f;
	//	for (FRoadSegment* f : determinedSegments) {
	//		if (f == f2)
	//			continue;

	//		FVector closestPoint = NearestPointOnLine(f->start, f->end - f->start, f2->end);
	//		bool isOnInterval = std::abs(FVector::Dist(f->start, closestPoint) + FVector::Dist(f->end, closestPoint) - FVector::Dist(f->start, f->end)) < 0.01f;
	//		if (!isOnInterval) {
	//			closestPoint = FVector::DistSquared(f->start, f2->end) < FVector::DistSquared(f->end, f2->end) ? f->start : f->end;
	//		}
	//		if (FVector::Dist(closestPoint, f2->end) < closestD) {
	//			closestYet = closestPoint;
	//			closestD = FVector::Dist(closestPoint, f2->end);
	//		//break;
	//		}
	//	}

	//	if (closestD < maxAttachDistance){
	//		UE_LOG(LogTemp, Warning, TEXT("attaching road to road ahead"));
	//		f2->end = closestYet;
	//		addVertices(f2);
	//	}



	//}



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


//struct LinkedLine {
//	FLine line;
//	bool buildLeft;
//	FVector point = FVector(0.0f, 0.0f, 0.0f);
//	LinkedLine* parent;
//	LinkedLine* child;
//};
//
//void invertAndParents(LinkedLine* line) {
//	TSet<LinkedLine*> taken;
//	while (line && !taken.Contains(line)) {
//		taken.Add(line);
//		//if (line->parent)
//		//	line->point = line->parent->point;
//		//else
//			line->point = FVector(0.0f, 0.0f, 0.0f);
//		FVector temp = line->line.start;
//		line->line.start = line->line.end;
//		line->line.end = temp;
//		LinkedLine* prevC = line->child;
//		line->child = line->parent;
//		line->parent = prevC;
//		line->buildLeft = !line->buildLeft;
//
//		line = line->child;
//
//	}
//}
//
//void invertAndChildren(LinkedLine* line) {
//	TSet<LinkedLine*> taken;
//	while (line && !taken.Contains(line)) {
//		taken.Add(line);
//		//if (line->child)
//		//	line->point = line->child->point;
//		//else
//			line->point = FVector(0.0f, 0.0f, 0.0f);
//
//		FVector temp = line->line.start;
//		line->line.start = line->line.end;
//		line->line.end = temp;
//		LinkedLine* prevC = line->child;
//		line->child = line->parent;
//		line->parent = prevC;
//		line->buildLeft = !line->buildLeft;
//
//		line = line->parent;
//	}
//}
//
//void decidePolygonFate(TArray<FLine> &segments, LinkedLine* &inLine, TArray<LinkedLine*> &lines, bool allowSplit, float extraRoadLen)
//{
//	float len = FVector::Dist(inLine->line.start, inLine->line.end);
//	float middleOffset = 600;
//	float width = 500;
//
//	if (len < 1000) {
//		delete inLine;
//		return;
//	}
//
//	// split lines blocking roads
//
//	FVector tangent1 = inLine->line.end - inLine->line.start;
//	tangent1.Normalize();
//	FVector tangent2 = FRotator(0, 90, 0).RotateVector(tangent1);
//	TArray<FVector> lineVertices;
//	FVector v1 = inLine->line.start + width * tangent2;
//	FVector v2 = inLine->line.start - width * tangent2;
//	FVector v3 = inLine->line.end + width * tangent2;
//	FVector v4 = inLine->line.end - width * tangent2;
//	lineVertices.Add(v1);
//	lineVertices.Add(v2);
//	lineVertices.Add(v3);
//	lineVertices.Add(v4);
//
//	for (FLine f : segments) {
//		FVector tangent = (f.end - f.start);
//		tangent.Normalize();
//		FVector intSec = intersection(f.start - tangent*extraRoadLen, f.end + tangent*extraRoadLen, inLine->line.start, inLine->line.end);
//		int counter = 0;
//		while (intSec.X != 0.0f && counter++ < 5) {
//			if (!allowSplit) {
//				delete inLine;
//				return;
//			}
//
//			FVector altTangent = FRotator(0, 90, 0).RotateVector(tangent);
//			LinkedLine* newP = new LinkedLine();
//			if (FVector::DistSquared(intSec, inLine->line.start) > FVector::DistSquared(intSec + altTangent * middleOffset, inLine->line.start)) {
//				// 1 got closer, so use 1
//				newP->line.start = inLine->line.start;
//				newP->line.end = intSec + altTangent * middleOffset;
//				inLine->line.start = intSec - altTangent * middleOffset;
//			}
//			else {
//				// 1 got further away, use 2 instead
//				newP->line.start = intSec + altTangent * middleOffset;
//				newP->line.end = inLine->line.start;
//				inLine->line.end = intSec - altTangent * middleOffset;
//			}
//			newP->buildLeft = inLine->buildLeft;
//			decidePolygonFate(segments, newP,lines, true, extraRoadLen);
//			intSec = intersection(f.start - tangent * extraRoadLen, f.end + tangent * extraRoadLen, inLine->line.start, inLine->line.end);
//			//return;
//		}
//	}
//	len = FVector::Dist(inLine->line.start, inLine->line.end);
//	if (len < 1000) {
//		delete inLine;
//		return;
//	}
//
//	for (int i = 0; i < lines.Num(); i++) {
//		LinkedLine *pol = lines[i];
//		// check lines
//
//			TArray<FVector> tangents;
//			FVector tangent3 = pol->line.end - pol->line.start;
//			tangent3.Normalize();
//
//			FVector tangent4 = FRotator(0, 90, 0).RotateVector(tangent3);
//
//			tangents.Add(tangent1);
//			tangents.Add(tangent2);
//			tangents.Add(tangent3);
//			tangents.Add(tangent4);
//
//			TArray<FVector> lineVertices2;
//			lineVertices2.Add(pol->line.start + tangent4 * width);
//			lineVertices2.Add(pol->line.start- tangent4 * width);
//			lineVertices2.Add(pol->line.end + tangent4 * width);
//			lineVertices2.Add(pol->line.end - tangent4 * width);
//
//			if (testCollision(tangents, lineVertices, lineVertices2, 0)) {
//
//				FVector res = intersection(pol->line.start, pol->line.end, inLine->line.start, inLine->line.end);
//				if (res.X != 0.0f) {
//					// on the previous line, is the collision close to the end?
//					if (FVector::Dist(pol->line.start, res) > FVector::Dist(pol->line.end, res)) {
//						// on the new line, collision end?
//						if (FVector::Dist(inLine->line.start, res) > FVector::Dist(inLine->line.end, res)) {
//							// then flip
//							invertAndParents(inLine);
//						}
//						else {
//
//						}
//						inLine->parent = pol;
//						pol->child = inLine;
//						pol->point = res;
//
//					} 	
//					// so the new line is maybe the master
//					else{
//						// on inLine, collision end?
//						if (FVector::Dist(inLine->line.start, res) > FVector::Dist(inLine->line.end, res)) {
//							pol->parent = inLine;
//							inLine->child = pol;
//
//						}
//						else {
//							// otherwise flip me
//							invertAndChildren(inLine);
//							inLine->child = pol;
//							pol->parent = inLine;
//						}
//						inLine->point = res;
//
//					}
//				}else {
//					// continous road
//
//					//if (FVector::Dist(pol->line.p1, res) > FVector::Dist(pol->line.p2, res)) {
//					//	inLine->parent = pol;
//					//	pol->child = inLine;
//					//	inLine->point = res;
//					//}
//					//else {
//					//	pol->parent = inLine;
//					//	inLine->child = pol;
//					//	pol->point = res;
//					//}
//					
//				}
//
//
//
//			}
//	}
//	lines.Add(inLine);
//	return;
//
//}
//
//struct PolygonPoint {
//	FPolygon* pol;
//	FVector point;
//};
//
//TArray<FMetaPolygon> ASpawner::getSurroundingPolygons(TArray<FLine> segments) {
//
//	TArray<LinkedLine*> lines;
//	// get coherent polygons
//	for (FLine f : segments) {
//		//if (f.type == RoadType::main)
//		//	continue;
//		// two collision segments for every road
//		FVector tangent = f.end - f.start;
//		tangent.Normalize();
//		FVector extraLength = tangent * 700;
//		FVector sideOffset = FRotator(0, 90, 0).RotateVector(tangent)*(standardWidth / 2 * f.width);
//		LinkedLine* left = new LinkedLine();
//		left->line.start = f.start + sideOffset - extraLength;
//		left->line.end = f.end + sideOffset + extraLength;
//		left->buildLeft = true;
//		LinkedLine* right = new LinkedLine();
//		right->line.start = f.start - sideOffset - extraLength;
//		right->line.end = f.end - sideOffset + extraLength;
//		right->buildLeft = false;
//
//
//		float extraRoadLen = 900;
//
//
//		decidePolygonFate(segments, left, lines, true, extraRoadLen);
//		//if (!f.roadInFront) {
//		//	LinkedLine* inFront = new LinkedLine();
//		//	inFront->line.p1 = f.end + sideOffset*1.5;
//		//	inFront->line.p2 = f.end - sideOffset*1.5;
//		//	decidePolygonFate(segments, inFront, lines, false, 0);
//		//}
//		decidePolygonFate(segments, right, lines, true, extraRoadLen);
//	}
//
//	TSet<LinkedLine*> remaining;
//	remaining.Append(lines);
//
//	TArray<FMetaPolygon> polygons;
//	int count = 0;
//	// build the actual polygons from the linked structures
//	while (remaining.Num() > 0){
//		TSet<LinkedLine*> taken;
//		UE_LOG(LogTemp, Log, TEXT("remaining: %i"), remaining.Num());
//		auto it = remaining.CreateIterator();
//		LinkedLine* curr = *it;
//		taken.Add(curr);
//		while (curr->parent && remaining.Contains(curr->parent) && !taken.Contains(curr->parent)) {
//			curr = curr->parent;
//			taken.Add(curr);
//		}
//		// now curr is top dog
//		FMetaPolygon f;
//		f.buildLeft = curr->buildLeft;
//		f.points.Add(curr->line.start);
//		f.points.Add(curr->point.X != 0.0f ? curr->point : curr->line.end);
//
//		taken.Empty();
//		taken.Add(curr);
//		remaining.Remove(curr);
//		while (curr->child && !taken.Contains(curr->child)) {
//			curr = curr->child;
//			f.points.Add(curr->point.X != 0.0f ? curr->point : curr->line.end);
//			remaining.Remove(curr);
//			taken.Add(curr);
//		}
//		if (curr->child && taken.Contains(curr->child)) {
//			f.points.RemoveAt(0);
//			f.points.EmplaceAt(0, intersection(curr->line.start, curr->line.end, curr->child->line.start, curr->child->line.end));
//			//f.points.Add(intersection(curr->line.p1, curr->line.p2, curr->child->line.p1, curr->child->line.p2));
//			f.open = false;
//		}
//		polygons.Add(f);
//
//	}
//
//	// these roads shouldn't exist, so this is mainly to highlight errors
//	for (int i = 0; i < polygons.Num(); i++) {
//		FMetaPolygon f = polygons[i];
//		if (f.points.Num() < 3) {
//			polygons.RemoveAt(i);
//			i--;
//		}
//	}
//
//
//
//	// split polygons into habitable blocks
//	
//	TArray<FMetaPolygon> refinedPolygons;
//
//
//	for (LinkedLine* l : lines) {
//		delete (l);
//	}
//	return polygons;
//
//
//}



TArray<FMetaPolygon> ASpawner::getSurroundingPolygons(TArray<FLine> segments)
{
	return BaseLibrary::getSurroundingPolygons(segments, segments, standardWidth, 700, 900, 500, 100);
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