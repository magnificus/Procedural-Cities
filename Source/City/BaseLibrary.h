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

 // would be much prettier with a 3-bit solution but that doesn't work with Blueprints... :(
UENUM(BlueprintType)
enum class Direction : uint8
{
	L 	UMETA(DisplayName = "Left"),
	F 	UMETA(DisplayName = "Forward"),
	R	UMETA(DisplayName = "Right"),
	LF  UMETA(DisplayName = "Left Front"),
	LR  UMETA(DisplayName = "Left Right"),
	FR  UMETA(DisplayName = "Forward Right"),
	LFR UMETA(DisplayName = "All Directions")
};

UENUM(BlueprintType)
enum class RoadType : uint8
{
	main 	UMETA(DisplayName = "Main Road"),
	secondary UMETA(DisplayName = "Secondary Road")
};



USTRUCT(BlueprintType)
struct FPolygon
{
	//GENERATED_BODY();
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FVector> points;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool open = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool buildLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector center;
};

USTRUCT(BlueprintType)
struct FHousePolygon {

	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPolygon polygon;

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
		Direction dir;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		Direction out;
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
	FPolygon f;
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
bool testCollision(TArray<FVector> tangents, TArray<FVector> vertices1, TArray<FVector> vertices2, float collisionLeniency);
float randFloat();
FVector NearestPointOnLine(FVector linePnt, FVector lineDir, FVector pnt);

FVector getCenter(FPolygon p);

class CITY_API BaseLibrary
{
public:
	BaseLibrary();
	~BaseLibrary();


};
