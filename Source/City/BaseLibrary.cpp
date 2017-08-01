// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "BaseLibrary.h"

BaseLibrary::BaseLibrary()
{
}

BaseLibrary::~BaseLibrary()
{
}

FPolygon getTinyPolygon(FVector point) {
	FPolygon temp;
	temp.points.Add(point);
	temp.points.Add(point + FVector(1, 1, 0));
	temp.points.Add(point + FVector(0, 1, 0));
	temp.points.Add(point + FVector(1, 0, 0));
	return temp;
}

void getMinMax(float &min, float &max, FVector tangent, TArray<FVector> points) {
	if (points.Num() == 0)
		return;

	min = FVector::DotProduct(tangent, points[0]);
	max = FVector::DotProduct(tangent, points[0]);

	for (int i = 1; i < points.Num(); i++) {
		float res = FVector::DotProduct(tangent, points[i]);
		min = std::min(min, res);
		max = std::max(max, res);
	}
}

// finds the first intersection point (if any) between a polygon and a list of potentially colliding polygons
FVector intersection(FPolygon &p1, TArray<FPolygon> &p2) {
	for (FPolygon &f : p2) {
		FVector res = intersection(p1, f);
		if (res.X != 0.0f) {
			return res;
		}
	}
	return FVector(0.0f, 0.0f, 0.0f);
}

// finds the first intersection point (if any) between two polygons via brute force (n^2)
FVector intersection(FPolygon &p1, FPolygon &p2) {
	for (int i = 1; i < p1.points.Num()+1; i++) {
		for (int j = 1; j < p2.points.Num()+1; j++) {
			FVector res = intersection(p1.points[i - 1], p1.points[i%p1.points.Num()], p2.points[j - 1], p2.points[j%p2.points.Num()]);
			if (res.X != 0.0f) {
				return res;
			}
		}
	}
	return FVector(0.0f, 0.0f, 0.0f);
}

// a pretty inefficient method for checking whether any of the lines in the polygon intersects another (n^2)
bool selfIntersection(FPolygon &p) {
	for (int i = 1; i < p.points.Num()+1; i++) {
		for (int j = i+2; j < p.points.Num()+1; j++) {
			FVector tan1 = p.points[i%p.points.Num()] - p.points[i - 1];
			tan1.Normalize();
			FVector tan2 = p.points[j%p.points.Num()] - p.points[j - 1];
			tan2.Normalize();
			if (intersection(p.points[i - 1] + tan1, p.points[i%p.points.Num()] - tan1, p.points[j - 1] + tan2, p.points[j%p.points.Num()] - tan2).X != 0.0f)
				return true;
		}
	}
	return false;
}

FVector intersection(FVector p1, FVector p2, FVector p3, FVector p4) {
	float p0_x = p1.X;
	float p0_y = p1.Y;
	float p1_x = p2.X;
	float p1_y = p2.Y;
	float p2_x = p3.X;
	float p2_y = p3.Y;
	float p3_x = p4.X;
	float p3_y = p4.Y;

	float s1_x, s1_y, s2_x, s2_y;
	s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
	s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

	float s, t;
	s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
	t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		// Collision detected
		return FVector(p0_x + (t * s1_x), p0_y + (t * s1_y), 0);
	}

	return FVector(0.0f, 0.0f, 0.0f); // No collision
}

FVector intersection(FVector p1, FVector p2, FPolygon p) {
	for (int i = 1; i < p.points.Num(); i++) {
		FVector res = intersection(p1, p2, p.points[i - 1], p.points[i]);
		if (res.X != 0.0f) {
			return res;
		}
	}
	return FVector(0.0f, 0.0f, 0.0f);
}



bool testAxis(FVector axis, FPolygon &p1, FPolygon &p2, float leniency) {
	float min1;
	float max1;
	float min2;
	float max2;

	getMinMax(min1, max1, axis, p1.points);
	getMinMax(min2, max2, axis, p2.points);
	if (std::max(min1, min2) >= std::min(max1, max2) - leniency) {
		return false;
	}
	else {
		return true;
	}
}

// check whether two polygons overlap with potential collision leniency
bool testCollision(FPolygon &p1, FPolygon &p2, float leniency) {
	for (int i = 1; i < p1.points.Num()+1; i++) {
		if (!testAxis(getNormal(p1.points[i%p1.points.Num()], p1.points[i-1], true), p1, p2, leniency)) {
			return false;
		}
	}
	for (int i = 1; i < p2.points.Num()+1; i++) {
		if (!testAxis(getNormal(p2.points[i%p2.points.Num()], p2.points[i-1], true), p1, p2, leniency)){//FRotator(0, 90, 0).RotateVector(p2.points[i] - p2.points[i-1]), p1, p2, leniency)) {
			return false;
		}
	}
	return true;
}

// check that a polygon doesn't collide with any other polygon and is inside of the surrounding polygon
bool testCollision(FPolygon &in, TArray<FPolygon> &others, float leniency, FPolygon &surrounding) {
	for (FPolygon &other : others) {
		if (testCollision(other, in, leniency)) {
			return true;
		}
	}
	return intersection(in, surrounding).X != 0.0f || !testCollision(in, surrounding, leniency);
}

// returns true if colliding
bool testCollision(TArray<FVector> tangents, TArray<FVector> vertices1, TArray<FVector> vertices2, float collisionLeniency) {
	float min1;
	float max1;
	float min2;
	float max2;
	for (FVector t : tangents) {
		getMinMax(min1, max1, t, vertices1);
		getMinMax(min2, max2, t, vertices2);
		if (std::max(min1, min2) >= (std::min(max1, max2) - collisionLeniency)) {
			return false;
		}
	}
	return true;
}



float randFloat() {
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX - 1);
}


FVector NearestPointOnLine(FVector linePnt, FVector lineDir, FVector pnt)
{
	lineDir.Normalize();//this needs to be a unit vector
	FVector v = pnt - linePnt;
	float d = FVector::DotProduct(v, lineDir);
	return linePnt + lineDir * d;
}

struct LinkedLine {
	FLine line;
	bool buildLeft;
	FVector point = FVector(0.0f, 0.0f, 0.0f);
	LinkedLine* parent;
	LinkedLine* child;
};


// change direction of line hierarchy, me and the parents
void invertAndParents(LinkedLine* line) {
	//return;

	TSet<LinkedLine*> taken;
	LinkedLine* prev = NULL;
	while (line && !taken.Contains(line)) {
		taken.Add(line);
		FVector temp = line->line.p1;
		line->line.p1 = line->line.p2;
		line->line.p2 = temp;
		
		if (line->parent)
			line->point = line->parent->point;
		else 
			line->point = FVector(0, 0, 0);
		line->child = line->parent;	
		if (prev) {
			line->parent = prev;
		}
		temp = line->point;
		prev = line;
		line->buildLeft = !line->buildLeft;

		line = line->child;
	}
}

// change direction of line hierarchy, me and the children
void invertAndChildren(LinkedLine* line) {
	//return;
	TSet<LinkedLine*> taken;
	LinkedLine* prev = NULL;
	FVector prevPoint = FVector(0,0,0);
	while (line && !taken.Contains(line)) {
		taken.Add(line);

		FVector temp = line->line.p1;
		line->line.p1 = line->line.p2;
		line->line.p2 = temp;

		line->parent = line->child;
		if (prev) {
			line->child = prev;
		}
		temp = line->point;
		line->point = prevPoint;
		prevPoint = temp;
		prev = line;
		line->buildLeft = !line->buildLeft;

		line = line->parent;
	}
}


FVector getProperIntersection(FVector p1, FVector p2, FVector p3, FVector p4) {
	FVector res = intersection(p1, p2, p3, p4);
	if (res.X == 0.0f) {
		// check if they collide into each other from the back
		FVector otherTangent = p2 - p1;
		otherTangent = FRotator(0, 90, 0).RotateVector(otherTangent);
		otherTangent.Normalize();

		res = intersection(p1 + otherTangent * 150, p1 - otherTangent * 150, p3, p4);
		if (res.X == 0.0f) {
			return intersection(p2 + otherTangent * 150, p2 - otherTangent * 150, p3, p4);
		}
	}
	return res;
}

TArray<FPolygon> getBlockingEntrances(TArray<FVector> points, TSet<int32> entrances, TMap<int32, FVector> specificEntrances, float entranceWidth, float blockingLength) {
	TArray<FPolygon> blocking;
	for (int i : entrances) {
		FPolygon entranceBlock;

		FVector inMiddle = specificEntrances.Contains(i) ? specificEntrances[i] : middle(points[i%points.Num()], points[i - 1]);
		FVector tangent = points[i%points.Num()] - points[i - 1];
		tangent.Normalize();
		FVector altTangent = FRotator(0, 90, 0).RotateVector(tangent);
		entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 + altTangent*blockingLength);
		entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 - altTangent*blockingLength);
		entranceBlock.points.Add(inMiddle + tangent * entranceWidth*0.5 - altTangent*blockingLength);
		entranceBlock.points.Add(inMiddle + tangent * entranceWidth*0.5 + altTangent*blockingLength);
		blocking.Add(entranceBlock);

	}
	return blocking;
}

void decidePolygonFate(TArray<FRoadSegment> &segments, TArray<FRoadSegment> &blocking, LinkedLine* &inLine, TArray<LinkedLine*> &lines, bool allowSplit, float extraRoadLen, float width, float middleOffset)
{
	float len = FVector::Dist(inLine->line.p1, inLine->line.p2);

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
	FVector v4 = inLine->line.p2 - width * tangent2;
	FVector v3 = inLine->line.p2 + width * tangent2;
	lineVertices.Add(v1);
	lineVertices.Add(v2);
	lineVertices.Add(v3);
	lineVertices.Add(v4);

	for (FRoadSegment f : blocking) {
		FVector tangent = f.p2 - f.p1;
		tangent.Normalize();
		FVector intSec = getProperIntersection(f.p1 - tangent*extraRoadLen, f.p2 + tangent*extraRoadLen, inLine->line.p1, inLine->line.p2);
		if (intSec.X != 0.0f) {
			if (!allowSplit) {
				delete inLine;
				return;
			}

			FVector altTangent = FRotator(0, 90, 0).RotateVector(tangent);
			LinkedLine* newP = new LinkedLine();
			float diff1 = FVector::DistSquared(intSec, inLine->line.p1) - FVector::DistSquared(intSec + altTangent * middleOffset, inLine->line.p1);
			float diff2 = FVector::DistSquared(intSec, inLine->line.p2) - FVector::DistSquared(intSec + altTangent * middleOffset, inLine->line.p2);

			if (diff1 > diff2) {
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

			newP->buildLeft = inLine->buildLeft;
			decidePolygonFate(segments, blocking, newP, lines, true, extraRoadLen, width, middleOffset);
			intSec = getProperIntersection(f.p1 - tangent * extraRoadLen, f.p2 + tangent * extraRoadLen, inLine->line.p1, inLine->line.p2);
			//return;
		}
	}
	len = FVector::Dist(inLine->line.p1, inLine->line.p2);
	if (len < 2000) {
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
		lineVertices2.Add(pol->line.p1 - tangent4 * width);
		lineVertices2.Add(pol->line.p2 - tangent4 * width);
		lineVertices2.Add(pol->line.p2 + tangent4 * width);



		FVector res = getProperIntersection(pol->line.p1, pol->line.p2, inLine->line.p1, inLine->line.p2);
			if (res.X != 0.0f) {
				// on the previous line, is the collision close to the end? if so, old pol is master
				if (FVector::Dist(pol->line.p1, res) > FVector::Dist(pol->line.p2, res)) {
					// on the new line, collision end?
					if (FVector::Dist(inLine->line.p1, res) > FVector::Dist(inLine->line.p2, res)) {
						// then flip
						invertAndParents(inLine);
					}
					inLine->parent = pol;
					pol->child = inLine;
					pol->point = res;

				}
				// so the new line is maybe the master
				else {
					// on inLine, collision end?
					if (FVector::Dist(inLine->line.p1, res) < FVector::Dist(inLine->line.p2, res)) {
						invertAndChildren(inLine);
					}
					pol->parent = inLine;
					inLine->child = pol;
					inLine->point = res;

				}
			//}
		}
	}
	lines.Add(inLine);
	return;

}

struct PolygonPoint {
	FPolygon* pol;
	FVector point;
};

// get polygons describing the shapes between all of the lines in segments
TArray<FMetaPolygon> BaseLibrary::getSurroundingPolygons(TArray<FRoadSegment> &segments, TArray<FRoadSegment> &blocking, float stdWidth, float extraLen, float extraRoadLen, float width, float middleOffset) {

	TArray<LinkedLine*> lines;
	// get coherent polygons
	for (FRoadSegment f : segments) {
		// two collision segments for every road
		FVector tangent = f.p2 - f.p1;
		tangent.Normalize();
		FVector extraLength = tangent * extraLen;
		FVector beginNorm = f.beginTangent;
		beginNorm.Normalize();
		FVector endNorm = f.endTangent;
		endNorm.Normalize();
		FVector sideOffsetBegin = FRotator(0, 90, 0).RotateVector(beginNorm)*(stdWidth/2 * f.width);
		FVector sideOffsetEnd = FRotator(0, 90, 0).RotateVector(tangent)*(stdWidth / 2 * f.width);

		LinkedLine* left = new LinkedLine();
		left->line.p1 = f.p1 + sideOffsetBegin - extraLength;
		left->line.p2 = f.p2 + sideOffsetEnd + extraLength;
		left->buildLeft = true;
		decidePolygonFate(segments, blocking, left, lines, true, extraRoadLen, width, middleOffset);

		//if (!f.roadInFront) {
		//	LinkedLine* top = new LinkedLine();
		//	top->line.p1 = f.p2 + (extraRoadLen+5)*tangent - sideOffsetEnd*1.1;
		//	top->line.p2 = f.p2 + (extraRoadLen+5)*tangent + sideOffsetEnd*1.1;
		//	top->buildLeft = true;
		//	decidePolygonFate(segments, blocking, top, lines, true, extraRoadLen, width, middleOffset);
		//}


		if (f.width != 0.0f) {
			LinkedLine* right = new LinkedLine();
			right->line.p1 = f.p1 - sideOffsetBegin - extraLength;
			right->line.p2 = f.p2 - sideOffsetEnd + extraLength;
			right->buildLeft = false;
			decidePolygonFate(segments, blocking, right, lines, true, extraRoadLen, width, middleOffset);
		}
	}

	TSet<LinkedLine*> remaining;
	remaining.Append(lines);

	TArray<FMetaPolygon> polygons;
	int count = 0;
	// build the actual polýgons from the linked structures
	while (remaining.Num() > 0) {
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
		f.points.Add(curr->point.X != 0.0f ? curr->point: curr->line.p2);

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
			// closed polygon since last point continues into first
			FVector res = getProperIntersection(curr->line.p1, curr->line.p2, curr->child->line.p1, curr->child->line.p2);
			//f.points.EmplaceAt(0, res);
			f.points.RemoveAt(0);
			f.points.Add(res);

			f.open = false;
		}
		else {
			f.open = true;


		}

		polygons.Add(f);

	}
	float maxConnect = 4500;

	for (int i = 0; i < polygons.Num(); i++) {
		FMetaPolygon &f = polygons[i];
		if (f.open && FVector::Dist(f.points[0], f.points[f.points.Num() - 1]) < maxConnect) {
			f.open = false;
		}
		f.checkOrientation();
	}

	// combine open polygons that are close together
	TArray<FMetaPolygon> prevOpen;
	for (int i = 0; i < polygons.Num(); i++) {
		FMetaPolygon p = polygons[i];
		if (p.open) {
			bool added = false;
			for (FMetaPolygon &p2 : prevOpen) {
				if (FVector::Dist(p2.points[0], p.points[0]) < maxConnect) {
					p2.reverse();
					p2.points.Append(p.points);
					added = true;
					break;
				}
				else if (FVector::Dist(p2.points[p2.points.Num()-1], p.points[0]) < maxConnect) {
					p2.points.Append(p.points);
					added = true;
					break;

				}
				else if (FVector::Dist(p2.points[0], p.points[p.points.Num() - 1]) < maxConnect) {
					p2.reverse();
					p.reverse();
					p2.points.Append(p.points);
					added = true;
					break;

				}
				else if (FVector::Dist(p2.points[p2.points.Num() - 1], p.points[p.points.Num() - 1]) < maxConnect) {
					p.reverse();
					p2.points.Append(p.points);
					added = true;
					break;
				}
			}
			if (!added) {
				prevOpen.Add(p);
			}
			polygons.RemoveAt(i);
			i--;
		}

	}
	polygons.Append(prevOpen);
	// remove redundant points, zip together open polygons where end and beginning are really close together, and check orientation, and remove very pointy edges
	for (int i = 0; i < polygons.Num(); i++) {
		FMetaPolygon &f = polygons[i];
		while (f.points.Num() > 1 && FVector::Dist(f.points[f.points.Num() - 2], f.points[f.points.Num() - 1]) < 10.0f) {
			f.points.RemoveAt(f.points.Num() - 1);
		}
		if (f.open && FVector::Dist(f.points[0], f.points[f.points.Num() - 1]) < maxConnect) {
			//f.points.Add(FVector(f.points[0]));
			f.open = false;
		}
		f.checkOrientation();

		f.clipEdges(-0.9f);
		if (f.points.Num() < 3) {
			polygons.RemoveAt(i);
			i--;
		}
	}


	for (LinkedLine* l : lines) {
		delete (l);
	}
	return polygons;


}

TArray<FMaterialPolygon> getSidesOfPolygon(FPolygon p, PolygonType type, float width) {
	TArray<FMaterialPolygon> toReturn;
	for (int i = 1; i < p.points.Num()+1; i++) {
		FMaterialPolygon side;
		side.type = type;
		side.points.Add(p.points[i - 1]);
		side.points.Add(p.points[i - 1] - FVector(0, 0, width));
		side.points.Add(p.points[i%p.points.Num()] - FVector(0, 0, width));
		side.points.Add(p.points[i%p.points.Num()]);
		toReturn.Add(side);
	}
	return toReturn;
}

TArray <FMaterialPolygon> fillOutPolygons(TArray<FMaterialPolygon> &inPols) {
	TArray<FMaterialPolygon> otherSides;
	for (FMaterialPolygon &p : inPols) {
		otherSides.Add(p);
		FMaterialPolygon other = p;
		bool polygonSides = true;
		// exterior walls are interiors on the inside
		if (p.type == PolygonType::exterior || p.type == PolygonType::exteriorSnd) {
			other.type = PolygonType::interior;
			//polygonSides = false;
		}
		if (p.type == PolygonType::floor)// || p.type == PolygonType::roof)
			polygonSides = false;

		other.offset(p.getDirection() * p.width);
		if (polygonSides) {
			for (int i = 1; i < p.points.Num()+1; i++) {
				FMaterialPolygon newP1;
				newP1.type = p.type;
				newP1.points.Add(other.points[i - 1]);
				newP1.points.Add(p.points[i%p.points.Num()]);
				newP1.points.Add(p.points[i - 1]);
				otherSides.Add(newP1);

				FMaterialPolygon newP2;
				newP2.type = p.type;
				newP2.points.Add(other.points[i - 1]);
				newP2.points.Add(other.points[i%other.points.Num()]);
				newP2.points.Add(p.points[i%p.points.Num()]);
				otherSides.Add(newP2);

			}
		}
		other.normal = -p.normal;

		other.reverse();
		otherSides.Add(other);

	}
	return otherSides;
}

TArray<FMaterialPolygon> BaseLibrary::getSimplePlotPolygons(TArray<FSimplePlot> plots) {
	TArray<FMaterialPolygon> toReturn;
	PolygonType type;
	if (plots.Num() > 0)
		type = plots[0].type == SimplePlotType::asphalt ? PolygonType::concrete : PolygonType::green;
	else
		return toReturn;
	for (FSimplePlot p : plots) {
		FMaterialPolygon newP;
		newP.points = p.pol.points;
		newP.reverse();
		newP.type = type;
		toReturn.Add(newP);

	}
	return toReturn;
}




FTransform FRoomPolygon::attemptGetPosition(TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes, bool windowAllowed, int testsPerSide, FString string, FRotator offsetRot, FVector offsetPos, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map, bool onWall) {
	for (int i = 1; i < points.Num(); i++) {
		if (windows.Contains(i) && !windowAllowed) {
			continue;
		}
		int place = i;
		for (int j = 0; j < testsPerSide; j++) {
			FVector dir = getNormal(points[place], points[place - 1], true);
			FVector tangent = points[place] - points[place - 1];
			float sideLen = tangent.Size();
			tangent.Normalize();
			dir.Normalize();
			FVector origin = points[place - 1] + tangent * (FMath::FRand() * (sideLen - 100.0f) + 100.0f);
			FVector pos = origin + dir + offsetPos;// *verticalOffset + offsetPos;
			FRotator rot = dir.Rotation() + offsetRot;
			FPolygon pol = getPolygon(rot, pos, string, map);

			// fit the polygon properly if possible
			float lenToMove = 0;
			for (int k = 1; k < pol.points.Num(); k++) {
				FVector res = intersection(pol.points[k - 1], pol.points[k], points[place], points[place - 1]);
				if (res.X != 0.0f) {
					float currentToMove = FVector::Dist(pol.points[k - 1], pol.points[k]) - FVector::Dist(pol.points[k - 1], res);
					lenToMove = std::max(lenToMove, currentToMove);
				}
			}
			if (lenToMove != 0) {
				pos += (lenToMove + 30)*dir;
				pol = getPolygon(rot, pos, string, map);
			}
			if (onWall) {
				// at least two points has to be next to the wall
				int count = 0;
				for (int k = 1; k < pol.points.Num(); k++) {
					FVector curr = NearestPointOnLine(points[i - 1], points[i] - points[i - 1], pol.points[k]);
					float dist = FMath::Pow(curr.X - pol.points[k].X, 2) + FMath::Pow(curr.Y - pol.points[k].Y, 2);
					if (dist < 1500) { // pick a number that seems reasonable
						count++;
					}
				}
				if (count < 2) {
					continue;
				}
			}

			if (!testCollision(pol, placed, 0, *this)) {
				return FTransform(rot, pos, FVector(1.0f, 1.0f, 1.0f));
			}
		}


	}
	return FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0));
}


/**
A simplified method for trying to find a wall in a room to place the mesh
@param r2 the room to place mesh in
@param placed other placed polygons, to check for collision
@param meshes the array to which append the mesh if sucessful
@param windowAllowed whether to allow the mesh to be placed in front of windows or not
@param testsPerSide number of tries for placement on each side of the room
@param string name of mesh in instanced mesh map
@param offsetRot offset rotation for object
@param offsetPos offset position for object'
@param map the instanced mesh map for finding the bounding box for the object
@param onwall whether the object is required to be strictly next to the wall (and not sticking out)
@return whether the placement was successful or not

*/
bool FRoomPolygon::attemptPlace(TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes, bool windowAllowed, int testsPerSide, FString string, FRotator offsetRot, FVector offsetPos, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map, bool onWall) {
	FTransform pos = attemptGetPosition(placed, meshes, windowAllowed, testsPerSide, string, offsetRot, offsetPos, map, onWall);
	if (pos.GetLocation().X != 0.0f) {
		FPolygon pol = getPolygon(pos.Rotator(), pos.GetLocation(), string, map);
		placed.Add(pol);
		meshes.Add(FMeshInfo{ string, pos });
		return true;
	}
	return false;
}

FPolygon getPolygon(FRotator rot, FVector pos, FString name, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map) {
	FVector min;
	FVector max;
	FPolygon pol;

	if (map.Contains(name))
		map[name]->GetLocalBounds(min, max);
	else
		return pol;

	pol.points.Add(rot.RotateVector(FVector(min.X, min.Y, 0.0f)) + pos);
	pol.points.Add(rot.RotateVector(FVector(max.X, min.Y, 0.0f)) + pos);
	pol.points.Add(rot.RotateVector(FVector(max.X, max.Y, 0.0f)) + pos);
	pol.points.Add(rot.RotateVector(FVector(min.X, max.Y, 0.0f)) + pos);
	return pol;
}
