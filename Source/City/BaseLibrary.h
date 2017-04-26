// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "stdlib.h"
#include <queue>
#include "GameFramework/Actor.h"
#include "Components/SplineMeshComponent.h"
#include "City.h"
#include "BaseLibrary.generated.h"
/**
 * 
 */


UENUM(BlueprintType)
enum class RoadType : uint8
{
	main 	UMETA(DisplayName = "Main Road"),
	secondary UMETA(DisplayName = "Secondary Road")
};


FVector intersection(FVector p1, FVector p2, FVector p3, FVector p4);


USTRUCT(BlueprintType)
struct FPolygon
{
	//GENERATED_BODY();
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FVector> points;

	FVector getCenter() {
		FVector center = FVector(0, 0, 0);
		for (FVector f : points) {
			center += f;
		}
		center /= points.Num();
		return center;
	}

	// only cares about dimensions X and Y, not Z
	double getArea() {

	double tot = 0;
	FPolygon newP = *this;

	// this makes sure we dont overflow because of our location in the world
	newP.offset(-(points[0]));

	for (int i = 0; i < points.Num() - 1; i++) {
		tot += 0.0001*(newP.points[i].X * newP.points[i + 1].Y - newP.points[i].Y - newP.points[i+1].X);
	}
	tot /= 2;
	return std::abs(tot);
	// must be even
	//if (points.Num() % 2 != 0) {
	//	FVector toAdd = points[0];
	//	points.Add(toAdd);
	//}
	//	float area = 0;
	//	int nPoints = points.Num();
	//	for (int i = 0; i < nPoints - 2; i += 2)
	//		area += (points[i + 1].X * (points[i + 2].Y - points[i].Y) + points[i + 1].Y * (points[i].X - points[i + 2].X)) * 0.000001;
	//	// the last point binds together beginning and end
	//	//area += points[nPoints - 1].X * (points[0].Y - points[nPoints - 2].Y) + points[nPoints - 1].Y * (points[nPoints - 2].X - points[0].X) * 0.00000001;
	//	return area / 2;
	}

	void offset(FVector offset) {
		for (FVector &f : points) {
			f += offset;
		}
	};





	// this method merges polygon sides when possible, and combines points
	void decreaseEdges() {
		float dirDiffAllowed = 0.07f;
		float distDiffAllowed = 300;

		for (int i = 1; i < points.Num(); i++) {
			if (FVector::Dist(points[i - 1], points[i]) < distDiffAllowed) {
				points.RemoveAt(i-1);
				i--;
			}
		}

		for (int i = 2; i < points.Num(); i++) {
			FVector prev = points[i - 1] - points[i - 2];
			prev.Normalize();
			FVector curr = points[i] - points[i - 1];
			curr.Normalize();
			//UE_LOG(LogTemp, Warning, TEXT("DIST: %f"), FVector::Dist(curr, prev));
			if (FVector::Dist(curr, prev) < dirDiffAllowed) {
				points.RemoveAt(i-1);
				i--;
			}
		}
	}


};

USTRUCT(BlueprintType)
struct FMetaPolygon : public FPolygon
{

	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool open = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool buildLeft;

	FMetaPolygon splitAlongMax() {



		int longest = 1;

		float longestLen = 0;

		FVector curr;
		for (int i = 1; i < points.Num(); i++) {
			curr = points[i] - points[i - 1];
			if (curr.Size() > longestLen) {
				longestLen = curr.Size();
				longest = i;
			}
		}

		FVector middle = (points[longest] - points[longest - 1]) / 2 + points[longest - 1];
		FVector tangent = FRotator(0, buildLeft ? 270 : 90, 0).RotateVector(curr);
		FVector p1 = middle;
		int split = 0;
		FVector p2;
		// find first intersecting line, if several, find the one which tangent is at greatest angle, makes sense if you think about it
		//float greatestDot;
		for (int i = 1; i < points.Num(); i++) {
			if (i == longest) {
				continue;
			}
			p2 = intersection(middle + tangent*0.1, middle + tangent*100, points[i - 1], points[i]);
			if (p2.X != 0.0f) {
				// split this
				split = i;
				break;
			}
		}
		//if (p2.X == 0.0f) {
		//	tangent = FRotator(0, buildLeft ? 90 : 270, 0).RotateVector(curr);
		//	for (int i = 1; i < points.Num(); i++) {
		//		if (i == longest) {
		//			continue;
		//		}
		//		p2 = intersection(middle, middle + tangent * 100, points[i - 1], points[i]);
		//		if (p2.X != 0.0f ) {
		//			// split this
		//			split = i;
		//			break;
		//		}
		//	}
		//}
		if (p2.X == 0.0f) {
			// cant split, no target
			return FMetaPolygon();
		}



		FMetaPolygon newP;
		newP.open = open;
		int min = longest;
		int max = split;

		// rearrange if split comes before longest in the array
		if (longest > split) {
			FVector temp = p1;
			p1 = p2;
			p2 = temp;
			min = split;
			max = longest;
		}
		newP.points.Add(p1);
		for (int i = min; i < max; i++) {
			newP.points.Add(points[i]);
		}
		newP.points.Add(p2);
		newP.points.Add(p1);

		points.RemoveAt(min, max - min);
		points.EmplaceAt(min, p1);
		points.EmplaceAt(min + 1, p2);

		//for (int i = 1; i < points.Num(); i++) {
		//	FVector p = points[i] - points[i - 1];
		//	float curr = p.Size();
		//	if (curr > sndLongestLen) {
		//		if (curr > longestLen) {
		//			sndLongestLen = longestLen;
		//			sndLongest = longest;
		//			longestLen = curr;
		//			longest = i;
		//		}
		//		else {
		//			sndLongestLen = curr;
		//			sndLongest = i;
		//		}
		//	}
		//}

		//int min = std::min(longest, sndLongest);
		//int max = std::max(sndLongest, longest);

		//FVector newCutoff1 = points[min] - points[min - 1];
		//FVector newCutoff2 = points[max] - points[max - 1];

		//FMetaPolygon newP;
		//newP.buildLeft = buildLeft;
		//newP.open = open;

		//FVector firstCut = points[min - 1] + newCutoff1 / 2;
		//FVector sndCut = points[max - 1] + newCutoff2 / 2;

		//newP.points.Add(firstCut);
		//for (int i = min; i < max; i++) {
		//	newP.points.Add(points[i]);
		//}
		//newP.points.Add(sndCut);
		//newP.points.Add(firstCut);
		//int lenToRemove = max - min;
		//points.RemoveAt(min, lenToRemove);
		//points.EmplaceAt(min, firstCut);
		//points.EmplaceAt(min+1, sndCut);	


		return newP;

	}

	TArray<FMetaPolygon> recursiveSplit(float maxArea, float minArea, int depth) {
		double area = getArea();
		if (area < minArea || points.Num() < 3 || depth > 3) {
			return TArray<FMetaPolygon>();
		}
		if (area > maxArea) {
			TArray<FMetaPolygon> tot;
			FMetaPolygon newP = splitAlongMax();
			if (newP.points.Num() > 2) {
				tot = newP.recursiveSplit(maxArea, minArea, depth+1);
				tot.Append(recursiveSplit(maxArea, minArea, depth + 1));
				//tot.Add(*this);
				return tot;
			}
			else {
				tot.Add(*this);
				return tot;
			}
		}
		else {
			TArray<FMetaPolygon> toReturn;
			toReturn.Add(*this);
			return toReturn;
		}
	}

	TArray<FMetaPolygon> refine(float maxArea, float minArea) {

		decreaseEdges();
		TArray<FMetaPolygon> toReturn;
		//FMetaPolygon other = splitAlongMax();
		//toReturn.Add(other);
		if (!open) {
			toReturn.Append(recursiveSplit(maxArea, minArea, 0));
		}
		else {
			toReturn.Add(*this);
		}
		return toReturn;
	}
};




USTRUCT(BlueprintType)
struct FHousePolygon : public FPolygon {

	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector housePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float height;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float population;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int type;
};


USTRUCT(BlueprintType)
struct FRoadSegment
{
	//GENERATED_BODY();

	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector start;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector end;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float width;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector beginTangent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		RoadType type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector v1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector v2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector v3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector v4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool roadInFront;

};



struct logicRoadSegment {
	int time;
	logicRoadSegment* previous;
	FRoadSegment* segment;
	FRotator firstDegreeRot;
	FRotator secondDegreeRot;
	int roadLength;
};

struct roadComparator {
	bool operator() (logicRoadSegment* arg1, logicRoadSegment* arg2) {
		return arg1->time > arg2->time;
	}
};


USTRUCT(BlueprintType)
struct FPlotPolygon : public FMetaPolygon{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float population;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int type;

};


struct Point {
	float x;
	float y;
};
/*
Get min and max value projected on FVector tangent, used in SAT collision detection for rectangles
*/


/*
Calculate whether two lines intersect and where
*/


void getMinMax(float &min, float &max, FVector tangent, FVector v1, FVector v2, FVector v3, FVector v4);
FVector intersection(FPolygon p1, FPolygon p2);
bool testCollision(TArray<FVector> tangents, TArray<FVector> vertices1, TArray<FVector> vertices2, float collisionLeniency);
float randFloat();
FVector NearestPointOnLine(FVector linePnt, FVector lineDir, FVector pnt);
//FVector project()

//FVector getCenter(FPolygon p);

class CITY_API BaseLibrary
{
public:
	BaseLibrary();
	~BaseLibrary();


};
