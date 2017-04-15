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

FVector intersection(FVector p1, FVector p2, FVector p3, FVector p4) {
	float x1 = p1.X, x2 = p2.X, x3 = p3.X, x4 = p4.X;
	float y1 = p1.Y, y2 = p2.Y, y3 = p3.Y, y4 = p4.Y;

	float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	// If d is zero, there is no intersection
	if (d == 0) return FVector{ 0,0,0 };

	// Get the x and y
	float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
	float x = (pre * (x3 - x4) - (x1 - x2) * post) / d;
	float y = (pre * (y3 - y4) - (y1 - y2) * post) / d;

	// Check if the x and y coordinates are within both lines
	if (x < std::min(x1, x2) || x > std::max(x1, x2) ||
		x < std::min(x3, x4) || x > std::max(x3, x4)) return FVector{ 0,0,0 };
	if (y < std::min(y1, y2) || y > std::max(y1, y2) ||
		y < std::min(y3, y4) || y > std::max(y3, y4)) return FVector{ 0,0,0 };

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
		if (std::max(min1, min2) >= std::min(max1, max2) - collisionLeniency) {
			return false;
		}
	}
	return true;
}



float randFloat() {
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

FVector getCenter(FPolygon p) {
	FVector center = FVector(0, 0, 0);
	for (FVector f : p.points) {
		center += f;
	}
	center /= p.points.Num();
	return center;
}

FVector NearestPointOnLine(FVector linePnt, FVector lineDir, FVector pnt)
{
	lineDir.Normalize();//this needs to be a unit vector
	FVector v = pnt - linePnt;
	float d = FVector::DotProduct(v, lineDir);
	return linePnt + lineDir * d;
}
