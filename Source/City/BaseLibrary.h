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


USTRUCT(BlueprintType)
struct FMetaPolygon : FPolygon
{
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool open = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool buildLeft;

	FMetaPolygon splitAlongMax() {

		int longest = 0;
		int sndLongest = 0;

		float longestLen = 0;
		float sndLongestLen = 0;

		for (int i = 1; i < points.Num(); i++) {
			FVector p = points[i] - points[i - 1];
			float curr = p.Size();
			if (curr > sndLongestLen) {
				if (curr > longestLen) {
					sndLongestLen = longestLen;
					sndLongest = longest;
					longestLen = curr;
					longest = i;
				}
				else {
					sndLongestLen = curr;
					sndLongest = i;
				}
			}
		}

		int min = std::min(longest, sndLongest);
		int max = std::max(sndLongest, longest);

		FVector newCutoff1 = points[min] - points[min - 1];
		FVector newCutoff2 = points[max] - points[max - 1];

		FVector firstJump = newCutoff1 / 2;
		FVector sndJump = newCutoff2 / 2;



		FPolygon newP;
		newP.open = false;

		FVector firstCut = points[min - 1] + firstJump;
		FVector sndCut = points[max - 1] + sndJump;

		newP.points.Add(firstCut);
		for (int i = min; i < max; i++) {
			newP.points.Add(points[i]);
		}
		newP.points.Add(sndCut);
		newP.points.Add(firstCut);
		int lenToRemove = max - min;
		points.RemoveAt(min, lenToRemove);
		points.EmplaceAt(min, firstCut);
		points.EmplaceAt(min + 1, sndCut);


		return newP;

	}
};


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
	float getArea() {
		float area = 0;
		int nPoints = points.Num();
		for (int i = 0; i < nPoints - 2; i += 2)
			area += points[i + 1].X * (points[i + 2].Y - points[i].Y) + points[i + 1].Y * (points[i].X - points[i + 2].X);
		// the last point binds together beginning and end
		area += points[nPoints - 1].X * (points[0].Y - points[nPoints - 2].Y) + points[nPoints - 1].Y * (points[nPoints - 2].X - points[0].X);
		return std::abs(area / 2);
	}



	TArray<FPolygon> recursiveSplit(float maxArea, float minArea) {
		float area = getArea();
		if (area < minArea) {
			return TArray<FPolygon>();
		}
		if (area > maxArea){
			FPolygon newP = splitAlongMax();
			TArray<FPolygon> tot = newP.recursiveSplit(maxArea, minArea);
			tot.Add(newP);
			tot.Append(recursiveSplit(maxArea, minArea));
			//tot.Add(*this);
			return tot;
		}
		else {
			TArray<FPolygon> toReturn;
			toReturn.Add(*this);
			return toReturn;
		}
	}

	// this method merges polygon sides when possible, and combines points
	void decreaseEdges() {
		float dirDiffAllowed = 0.01f;
		float distDiffAllowed = 400;

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
			UE_LOG(LogTemp, Warning, TEXT("DIST: %f"), FVector::Dist(curr, prev));
			if (FVector::Dist(curr, prev) < dirDiffAllowed) {
				points.RemoveAt(i-1);
				i--;
			}
		}
	}

	TArray<FPolygon> refine(float maxArea, float minArea) {
		
		//decreaseEdges();
		//if (!open) {
		//	return recursiveSplit(maxArea, minArea);
		//}
		TArray<FPolygon> toReturn;
		toReturn.Add(*this);
		return toReturn;
	}
};





USTRUCT(BlueprintType)
struct FHousePolygon {

	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPolygon polygon;

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
struct FPlotPolygon {
	GENERATED_USTRUCT_BODY();


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMetaPolygon f;
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
FVector intersection(FVector p1, FVector p2, FVector p3, FVector p4);
FVector intersection(FPolygon p1, FPolygon p2);
bool testCollision(TArray<FVector> tangents, TArray<FVector> vertices1, TArray<FVector> vertices2, float collisionLeniency);
float randFloat();
FVector NearestPointOnLine(FVector linePnt, FVector lineDir, FVector pnt);

//FVector getCenter(FPolygon p);

class CITY_API BaseLibrary
{
public:
	BaseLibrary();
	~BaseLibrary();


};
