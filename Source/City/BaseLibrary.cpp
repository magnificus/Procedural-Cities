// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "BaseLibrary.h"

BaseLibrary::BaseLibrary()
{
}

BaseLibrary::~BaseLibrary()
{
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
	for (int i = 1; i < p1.points.Num(); i++) {
		for (int j = 1; j < p2.points.Num(); j++) {
			FVector res = intersection(p1.points[i - 1], p1.points[i], p2.points[j - 1], p2.points[j]);
			if (res.X != 0.0f) {
				return res;
			}
		}
	}
	return FVector(0.0f, 0.0f, 0.0f);
}

// a pretty inefficient method for checking whether any of the lines in the polygon intersects another
bool selfIntersection(FPolygon &p) {
	for (int i = 1; i < p.points.Num(); i++) {
		for (int j = i+1; j < p.points.Num(); j++) {
			FVector tan1 = p.points[i] - p.points[i - 1];
			tan1.Normalize();
			FVector tan2 = p.points[j] - p.points[j - 1];
			tan2.Normalize();
			if (intersection(p.points[i - 1] + tan1, p.points[i] - tan1, p.points[j - 1] + tan2, p.points[j] - tan2).X != 0.0f)
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
	for (int i = 1; i < p1.points.Num(); i++) {
		if (!testAxis(getNormal(p1.points[i], p1.points[i-1], true), p1, p2, leniency)) {
			return false;
		}
	}
	for (int i = 1; i < p2.points.Num(); i++) {
		if (!testAxis(getNormal(p2.points[i], p2.points[i-1], true), p1, p2, leniency)){//FRotator(0, 90, 0).RotateVector(p2.points[i] - p2.points[i-1]), p1, p2, leniency)) {
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
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX - 100);
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

void decidePolygonFate(TArray<FLine> &segments, TArray<FLine> &blocking, LinkedLine* &inLine, TArray<LinkedLine*> &lines, bool allowSplit, float extraRoadLen, float width, float middleOffset)
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
	FVector v3 = inLine->line.p2 + width * tangent2;
	FVector v4 = inLine->line.p2 - width * tangent2;
	lineVertices.Add(v1);
	lineVertices.Add(v2);
	lineVertices.Add(v3);
	lineVertices.Add(v4);

	for (FLine f : blocking) {
		FVector tangent = f.p2 - f.p1;
		tangent.Normalize();
		FVector intSec = intersection(f.p1 - tangent*extraRoadLen, f.p2 + tangent*extraRoadLen, inLine->line.p1, inLine->line.p2);
		int counter = 0;
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
			intSec = intersection(f.p1 - tangent * extraRoadLen, f.p2 + tangent * extraRoadLen, inLine->line.p1, inLine->line.p2);
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
		lineVertices2.Add(pol->line.p1 - tangent4 * width);
		lineVertices2.Add(pol->line.p2 + tangent4 * width);
		lineVertices2.Add(pol->line.p2 - tangent4 * width);



		FVector res = intersection(pol->line.p1, pol->line.p2, inLine->line.p1, inLine->line.p2);
		//if (testCollision(tangents, lineVertices, lineVertices2, 0)) {

			//if (std::abs(res.X) < 0.1f) {
			//	res = FVector::Dist(pol->line.p1, middle(inLine->line.p1, inLine->line.p2)) < FVector::Dist(pol->line.p2, middle(inLine->line.p1, inLine->line.p2)) ? pol->line.p1 : pol->line.p2;
			//}
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
TArray<FMetaPolygon> BaseLibrary::getSurroundingPolygons(TArray<FLine> &segments, TArray<FLine> &blocking, float stdWidth, float extraLen, float extraRoadLen, float width, float middleOffset) {

	TArray<LinkedLine*> lines;
	// get coherent polygons
	for (FLine f : segments) {
		// two collision segments for every road
		FVector tangent = f.p2 - f.p1;
		tangent.Normalize();
		FVector extraLength = tangent * extraLen;

		FVector sideOffset = FRotator(0, 90, 0).RotateVector(tangent)*(stdWidth/2 * f.width);
		LinkedLine* left = new LinkedLine();
		left->line.p1 = f.p1 + sideOffset - extraLength;
		left->line.p2 = f.p2 + sideOffset + extraLength;
		left->buildLeft = true;
		decidePolygonFate(segments, blocking, left, lines, true, extraRoadLen, width, middleOffset);

		if (f.width != 0.0f) {
			LinkedLine* right = new LinkedLine();
			right->line.p1 = f.p1 - sideOffset - extraLength;
			right->line.p2 = f.p2 - sideOffset + extraLength;
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
			//FVector res = intersection(curr->line.p1, curr->line.p2, curr->child->line.p1, curr->child->line.p2);
			f.points.Add(curr->point.X != 0.0f ? curr->point : curr->line.p2);
			remaining.Remove(curr);
			taken.Add(curr);
		}
		if (curr->child && taken.Contains(curr->child)) {
			// closed polygon since last point continues into first
			FVector res = intersection(curr->line.p1, curr->line.p2, curr->child->line.p1, curr->child->line.p2);
			if (true || res.X != 0.0f) {
				f.points.RemoveAt(0);
				f.points.EmplaceAt(0, res);
				f.points.Add(res);
			}
			else {
				FVector first = f.points[0];
				f.points.Add(first);
			}

			f.open = false;
		}
		else {
			f.open = true;


		}

		polygons.Add(f);

	}

	//// delete impossible polygons if they exist, they shouldn't 
	//for (int i = 0; i < polygons.Num(); i++) {
	//	FMetaPolygon f = polygons[i];
	//	if (f.points.Num() < 3) {
	//		polygons.RemoveAt(i);
	//		i--;
	//	}
	//}

	// zip together open polygons where end and beginning are really close together, and check orientation
	for (int i = 0; i < polygons.Num(); i++) {
		FMetaPolygon &f = polygons[i];
		if (f.open && FVector::Dist(f.points[0], f.points[f.points.Num()-1]) < 5000) {
			f.points.Add(FVector(f.points[0]));
			f.open = false;
		}
		f.checkOrientation();
		//if (f.getArea() < 500) {
		//	polygons.RemoveAt(i);
		//	i--;
		//}
	}


	for (LinkedLine* l : lines) {
		delete (l);
	}
	return polygons;


}
