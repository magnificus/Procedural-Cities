// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "BaseLibrary.h"

BaseLibrary::BaseLibrary()
{
}

BaseLibrary::~BaseLibrary()
{
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
}

FVector intersection(FPolygon p1, FPolygon p2) {
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

FVector intersection(FVector p1, FVector p2, FVector p3, FVector p4) {
	float x1 = p1.X, x2 = p2.X, x3 = p3.X, x4 = p4.X;
	float y1 = p1.Y, y2 = p2.Y, y3 = p3.Y, y4 = p4.Y;

	float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	// If d is zero, there is no intersection
	if (d == 0) return FVector{ 0.0f,0.0f,0.0f };

	// Get the x and y
	float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
	float x = (pre * (x3 - x4) - (x1 - x2) * post) / d;
	float y = (pre * (y3 - y4) - (y1 - y2) * post) / d;

	// Check if the x and y coordinates are within both lines
	if (x < std::min(x1, x2) || x > std::max(x1, x2) ||
		x < std::min(x3, x4) || x > std::max(x3, x4)) return FVector{ 0.0f,0.0f,0.0f };
	if (y < std::min(y1, y2) || y > std::max(y1, y2) ||
		y < std::min(y3, y4) || y > std::max(y3, y4)) return FVector{ 0.0f,0.0f,0.0f };

	// Return the point of intersection
	FVector ret{ x,y,0 };
	return ret;
}

// returns true if colliding
bool testCollision(TArray<FVector> tangents, TArray<FVector> vertices1, TArray<FVector> vertices2, float collisionLeniency) {
	// assume rectangles
	float min1;
	float max1;
	float min2;
	float max2;

	for (FVector t : tangents) {
		getMinMax(min1, max1, t, vertices1[0], vertices1[1], vertices1[2], vertices1[3]);
		getMinMax(min2, max2, t, vertices2[0], vertices2[1], vertices2[2], vertices2[3]);
		if (std::max(min1, min2) >= (std::min(max1, max2) - collisionLeniency)) {
			return false;
		}
	}
	return true;
}



float randFloat() {
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
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

void decidePolygonFate(TArray<FLine> &segments, LinkedLine* &inLine, TArray<LinkedLine*> &lines, bool allowSplit, float extraRoadLen)
{
	float len = FVector::Dist(inLine->line.p1, inLine->line.p2);
	float middleOffset = 600;
	float width = 500;

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

	for (FLine f : segments) {
		FVector tangent = (f.p2 - f.p1);
		tangent.Normalize();
		FVector intSec = intersection(f.p1 - tangent*extraRoadLen, f.p2 + tangent*extraRoadLen, inLine->line.p1, inLine->line.p2);
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
			newP->buildLeft = inLine->buildLeft;
			decidePolygonFate(segments, newP, lines, true, extraRoadLen);
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

				}
				// so the new line is maybe the master
				else {
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
					inLine->point = res;

				}
			}
			else {
				// continous road
				//if (FVector::Dist(pol->line.p1, inLine->line.getMiddle()) > FVector::Dist(pol->line.p2, inLine->line.getMiddle())) {
				//	// on the new line, collision end?
				//	if (FVector::Dist(inLine->line.p1, pol->line.getMiddle()) > FVector::Dist(inLine->line.p2, pol->line.getMiddle())) {
				//		// then flip
				//		invertAndParents(inLine);
				//	}
				//	else {

				//	}
				//	inLine->parent = pol;
				//	pol->child = inLine;
				//	//pol->point = res;

				//}
				//// so the new line is maybe the master
				//else {
				//	// on inLine, collision end?
				//	if (FVector::Dist(inLine->line.p1, pol->line.getMiddle()) > FVector::Dist(inLine->line.p2, pol->line.getMiddle())) {
				//		pol->parent = inLine;
				//		inLine->child = pol;

				//	}
				//	else {
				//		// otherwise flip me
				//		invertAndChildren(inLine);
				//		inLine->child = pol;
				//		pol->parent = inLine;
				//	}
				//	inLine->point = res;

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

TArray<FMetaPolygon> BaseLibrary::getSurroundingPolygons(TArray<FLine> segments, float stdWidth) {

	TArray<LinkedLine*> lines;
	// get coherent polygons
	for (FLine f : segments) {
		//if (f.type == RoadType::main)
		//	continue;
		// two collision segments for every road
		FVector tangent = f.p2 - f.p1;
		tangent.Normalize();
		FVector extraLength = tangent * 700;
		FVector sideOffset = FRotator(0, 90, 0).RotateVector(tangent)*(stdWidth/2 * f.width);
		LinkedLine* left = new LinkedLine();
		left->line.p1 = f.p1 + sideOffset - extraLength;
		left->line.p2 = f.p2 + sideOffset + extraLength;
		left->buildLeft = true;
		LinkedLine* right = new LinkedLine();
		right->line.p1 = f.p1 - sideOffset - extraLength;
		right->line.p2 = f.p2 - sideOffset + extraLength;
		right->buildLeft = false;


		float extraRoadLen = 900;


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
			FVector res = intersection(curr->line.p1, curr->line.p2, curr->child->line.p1, curr->child->line.p2);
			if (res.X != 0.0f) {
				f.points.RemoveAt(0);
				f.points.EmplaceAt(0, res);
			}

			f.open = false;
		}
		polygons.Add(f);

	}

	// these roads generally shouldn't exist, so this is mainly to highlight errors
	//for (int i = 0; i < polygons.Num(); i++) {
	//	FMetaPolygon f = polygons[i];
	//	if (f.points.Num() < 3) {
	//		polygons.RemoveAt(i);
	//		i--;
	//	}
	//}



	// split polygons into habitable blocks

	TArray<FMetaPolygon> refinedPolygons;


	for (LinkedLine* l : lines) {
		delete (l);
	}
	return polygons;


}
