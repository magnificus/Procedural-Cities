// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "stdlib.h"
#include <queue>
#include "GameFramework/Actor.h"
#include "Components/SplineMeshComponent.h"
#include "City.h"
#include "Algo/Reverse.h"
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

UENUM(BlueprintType)
enum class RoomType : uint8
{
	office 	UMETA(DisplayName = "Office"),
	apartment UMETA(DisplayName = "Apartment"),
	store UMETA(DisplayName = "Store")
};

UENUM(BlueprintType)
enum class PolygonType : uint8
{
	interior 	UMETA(DisplayName = "Interior"),
	exterior UMETA(DisplayName = "Exterior"),
	floor UMETA(DisplayName = "Floor"),
	window UMETA(DisplayName = "Window"),
	roof UMETA(DisplayName = "Roof")
};



FVector intersection(FVector p1, FVector p2, FVector p3, FVector p4);

struct SplitStruct {
	int min;
	int max;
	FVector p1;
	FVector p2;
};

static FVector middle(FVector p1, FVector p2) {
	return (p2 - p1) * 0.5 + p1;
}

static TArray<int32> getIntList(int32 min, int32 max) {
	TArray<int32> ints;
	for (int32 i = min; i < max; i++) {
		ints.Add(i);
	}
	return ints;
}

static FVector getNormal(FVector p1, FVector p2, bool left) {
	return FRotator(0, left ? 90 : 270, 0).RotateVector(p2 - p1);
}

USTRUCT(BlueprintType)
struct FPolygon
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadWrite)
		TArray<FVector> points;

	FVector getCenter() {
		FVector center = FVector(0, 0, 0);
		double totLen = 0;
		for (int i = 1; i < points.Num(); i++) {
			float len = (points[i] - points[i - 1]).Size();
			center += ((points[i] - points[i - 1])/2 + points[i-1])*len;
			totLen += len;
		}
		center /= totLen;
		return center;
	}

	// only cares about dimensions X and Y, not Z
	double getArea() {

	if (points.Num() < 3)
		return 0.0;

	double tot = 0;

	for (int i = 0; i < points.Num() - 1; i++) {
		if (!points.IsValidIndex(i) || !points.IsValidIndex(i+1)) {
			return 0.0f;
		}
		tot += 0.0001*(points[i].X * points[i + 1].Y);
		tot -= 0.0001*(points[i].Y * points[i+1].X);
	}
	tot /= 2;
	return std::abs(tot);
	}

	void offset(FVector offset) {
		for (FVector &f : points) {
			f += offset;
		}
	};





	// this method merges polygon sides when possible, and combines points
	void decreaseEdges() {
		float dirDiffAllowed = 0.07f;
		float distDiffAllowed = 200;

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

	// assumes at least 3 points in polygon
	FVector getDirection() {
		FVector res = FVector::CrossProduct(points[1] - points[0], points[2] - points[0]);
		res.Normalize();
		return res;
	}

	void reverse() {
		Algo::Reverse(points);
	}

	SplitStruct getSplitProposal(bool buildLeft, float approxRatio) {
			
		if (points.Num() < 3) {
			return SplitStruct{ 0, 0, FVector(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 0.0f) };
		}
		if (FVector::Dist(points[0], points[points.Num() - 1]) > 0.1f) {
			UE_LOG(LogTemp, Warning, TEXT("END AND BEGINNING NOT CONNECTED IN SPLITSTRUCT, dist is: %f"), FVector::Dist(points[0], points[points.Num() - 1]));
			FVector first = points[0];
			points.Add(first);
		}

			int longest = 1;

			float longestLen = 0.0f;

			FVector curr;
			for (int i = 1; i < points.Num(); i++) {
				float dist = FVector::DistSquared(points[i], points[i - 1]);
				if (dist > longestLen) {
					longestLen = dist;
					longest = i;
				}
			}
			curr = points[longest] - points[longest - 1];
			curr.Normalize();

			FVector middle = (points[longest] - points[longest - 1]) * approxRatio + points[longest - 1];
			FVector p1 = middle;
			int split = 0;
			FVector p2 = FVector(0.0f, 0.0f, 0.0f);
			FVector tangent = FRotator(0, buildLeft ? 90 : 270, 0).RotateVector(curr);
			//tangent.Normalize();
			float closest = 10000000.0f;
			for (int i = 1; i < points.Num(); i++) {
				if (i == longest) {
					continue;
				}
				curr = intersection(middle, middle + tangent * 100000, points[i - 1], points[i]);
				if (curr.X != 0.0f && FVector::Dist(curr, middle) < closest) {
					closest = FVector::Dist(curr, middle);
					split = i;
					p2 = curr;
					
				}
			}

			if (p2.X == 0.0f || p1.X == 0.0f) {
				UE_LOG(LogTemp, Warning, TEXT("UNABLE TO SPLIT"));
				// cant split, no target, this shouldn't happen unless the polygons are poorly constructed
				return SplitStruct{ 0, 0, FVector(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 0.0f) };

			}

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

			return SplitStruct{ min, max, p1, p2 };
	}


};


USTRUCT(BlueprintType)
struct FMaterialPolygon : public FPolygon {
	GENERATED_USTRUCT_BODY();

	PolygonType type = PolygonType::exterior;
	float width = 0;
};


USTRUCT(BlueprintType)
struct FMetaPolygon : public FPolygon
{

	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool open = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool buildLeft;

	void checkOrientation() {
		FVector tangent = points[1] - points[0];
		tangent = FRotator(0, buildLeft ? 90 : 270, 0).RotateVector(tangent);
		tangent.Normalize();
		FVector middle = (points[1] - points[0]) / 2 + points[0];
		for (int i = 2; i < points.Num(); i++) {
			if (intersection(middle, middle + tangent * 100000, points[i - 1], points[i]).X != 0.0f)
				return;
		}
		buildLeft = !buildLeft;
	}

};

UENUM(BlueprintType)
enum class SubRoomType : uint8
{
	meeting UMETA(DisplayName = "Meeting Room"),
	work UMETA(DisplayName = "Work Room"),
	bath  UMETA(DisplayName = "Bathroom"),
	empty  UMETA(DisplayName = "Empty"),
	corridor UMETA(DisplayName = "Corridor"),
	bed UMETA(DisplayName = "Bedroom"),
	kitchen UMETA(DisplayName = "Kitchen"),
	living UMETA(DIsplayName = "Living Room"),
	closet UMETA(DIsplayName = "Closet")

};


struct RoomSpecification {
	float minArea;
	float maxArea;
	SubRoomType type;
};

struct RoomBlueprint {
	TArray<RoomSpecification> needed;
	TArray<RoomSpecification> optional;
};

//static getOfficeSpecifications

struct FRoomPolygon : public FPolygon
{
	TSet<int32> windows;
	TSet<int32> entrances;
	TSet<int32> toIgnore;

	bool canRefine = true;
	SubRoomType type = SubRoomType::empty;

	//bool operator==(const FRoomPolygon& rhs)
	//{
	//	return points == rhs.points && windows == rhs.windows;
	//}

	FRoomPolygon splitAlongMax(float approxRatio) {
		SplitStruct p = getSplitProposal(false, approxRatio);
		if (p.p1.X == 0.0f) {
			return FRoomPolygon();
		}
		FRoomPolygon newP;
		newP.points.Add(p.p1);

		if (entrances.Contains(p.min)) {
			newP.entrances.Add(newP.points.Num());
		}
		if (windows.Contains(p.min)) {
			newP.windows.Add(newP.points.Num());
		}
		if (toIgnore.Contains(p.min)) {
			newP.toIgnore.Add(newP.points.Num());
		}
		newP.points.Add(points[p.min]);

		for (int i = p.min + 1; i < p.max; i++) {
			if (entrances.Contains(i)) {
				entrances.Remove(i);
				newP.entrances.Add(newP.points.Num());
			}
			if (windows.Contains(i)) {
				windows.Remove(i);
				newP.windows.Add(newP.points.Num());
			}
			if (toIgnore.Contains(i)) {
				toIgnore.Remove(i);
				newP.toIgnore.Add(newP.points.Num());
			}
			newP.points.Add(points[i]);
		}
		if (entrances.Contains(p.max)) {
			newP.entrances.Add(newP.points.Num());
		}
		if (windows.Contains(p.max)) {
			newP.windows.Add(newP.points.Num());
		}
		if (toIgnore.Contains(p.max)) {
			newP.toIgnore.Add(newP.points.Num());
		}



		std::vector<int32> toRemove;
		for (int32 i : entrances) {
			if (i >= p.max)
				toRemove.push_back(i);
		}
		for (int32 i : toRemove) {
			entrances.Remove(i);
			entrances.Add(i - (p.max - p.min) + 2);
		}

		toRemove.clear();
		for (int32 i : windows) {
			if (i >= p.max)
				toRemove.push_back(i);
		}
		for (int32 i : toRemove) {
			windows.Remove(i);
			windows.Add(i - (p.max - p.min) + 2);
		}


		toRemove.clear();
		for (int32 i : toIgnore) {
			if (i >= p.max)
				toRemove.push_back(i);
		}
		for (int32 i : toRemove) {
			toIgnore.Remove(i);
			toIgnore.Add(i - (p.max - p.min) + 2);
		}


		newP.points.Add(p.p2);
		// dont place the wall twice
		newP.toIgnore.Add(newP.points.Num());
		newP.points.Add(p.p1);

		// entrance to next room
		entrances.Add(p.min + 1);

		points.RemoveAt(p.min, p.max - p.min);
		points.EmplaceAt(p.min, p.p1);
		points.EmplaceAt(p.min + 1, p.p2);

		if (FVector::DistSquared(points[0], points[points.Num() - 1]) > 100) {
			UE_LOG(LogTemp, Warning, TEXT("RESULTING SELF NOT CONNECTED"));
			FVector first = points[0];
			points.Add(first);
		}
		return newP;
	}

	TArray<FRoomPolygon> recursiveSplit(float maxArea, float minArea, int depth) {
		double area = getArea();
		//UE_LOG(LogTemp, Warning, TEXT("area of new room: %f"), area);
		TArray<FRoomPolygon> tot;

		if (points.Num() < 3 || area < minArea) {
			//tot.Add(*this);
			return tot;
		}
		//else if (depth > 3) {
		//	tot.Add(*this);
		//	return tot;
		//}
		else if (area > maxArea) {
			FRoomPolygon newP = splitAlongMax(0.5);
			if (newP.points.Num() > 2) {
				tot = newP.recursiveSplit(maxArea, minArea, depth + 1);
			}
			tot.Append(recursiveSplit(maxArea, minArea, depth + 1));

		}
		else {
			tot.Add(*this);
		}
		return tot;
	}


	TArray<FRoomPolygon> fitSpecificationOnRooms(TArray<RoomSpecification> specs, TArray<FRoomPolygon> &remaining, bool repeating) {
		TArray<FRoomPolygon> toReturn;

		float area;
		float minPctSplit = 0.25f;
		bool couldPlace = false;
		int c1 = 0;
		do {
			couldPlace = false;
			for (RoomSpecification r : specs) {
				bool found = false;
				bool smaller = false;
				for (int i = 0; i < remaining.Num(); i++) {
					FRoomPolygon &p = remaining[i];
					area = p.getArea();
					smaller = smaller || (area > r.maxArea);
					if (area <= r.maxArea && area >= r.minArea) {
						// found fitting room
						p.type = r.type;
						toReturn.Add(p);
						remaining.RemoveAt(i);
						found = true;
						couldPlace = true;
						break;
					}
				}
				// could not find a fitting room since all remaining are too big, cut them down to size
				if (!found && smaller) {
					FRoomPolygon target;
					int targetNum = 0;
					float scale = 0.0f;
					for (int i = 0; i < remaining.Num(); i++) {
						target = remaining[i];
						targetNum = i;
						scale = r.minArea / target.getArea();
						if (scale < 1.0f) {
							break;
						}
					}
					remaining.RemoveAt(targetNum);
					//FRoomPolygon &target = remaining[0];
					while (scale < minPctSplit) {
						FRoomPolygon newP = target.splitAlongMax(0.4);
						remaining.Add(newP);
						scale = r.minArea / target.getArea();

					}
					if (target.getArea() <= r.maxArea && target.getArea() >= r.minArea) {
						target.type = r.type;
						toReturn.Add(target);
						//remaining.RemoveAt(targetNum);

					}
					else {
						float ideal = (r.maxArea - r.minArea) / 2 + r.minArea;
						FRoomPolygon newR = target.splitAlongMax(r.minArea / target.getArea());
						newR.type = r.type;
						toReturn.Add(newR);
						remaining.Add(target);

					}
					couldPlace = true;

				}
			}
		} while (repeating && couldPlace && c1++ < 5);
		return toReturn;
	}

	TArray<FRoomPolygon> getRooms(RoomBlueprint blueprint) {
		TArray<FRoomPolygon> rooms;
		TArray<FRoomPolygon> remaining;
		remaining.Add(*this);

		rooms.Append(fitSpecificationOnRooms(blueprint.needed, remaining, false));
		rooms.Append(fitSpecificationOnRooms(blueprint.optional, remaining, true));
		for (FRoomPolygon &p : remaining) {
			rooms.Add(p);
		}
		return rooms;
	}


	TArray<FRoomPolygon> refine(float maxArea, float minArea) {
		return recursiveSplit(maxArea, minArea, 0);
	}




	FVector getRoomDirection() {
		if (points.Num() < 3) {
			return FVector(0.0f, 0.0f, 0.0f);
		}
		return getNormal(points[1], points[0], true);
	}

};






USTRUCT(BlueprintType)
struct FHousePolygon : public FMetaPolygon {

	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector housePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float height;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float population;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	RoomType type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSet<int32> entrances;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSet<int32> windows;

	void removePoint(int place) {
		std::vector<int> toRemove;
		for (int i : windows) {
			if (i > place) {
				toRemove.push_back(i);
			}
		}
		for (int i : toRemove) {
			windows.Remove(i);
			windows.Add(i - 1);
		}
		toRemove.clear();
		for (int i : entrances) {
			if (i > place) {
				toRemove.push_back(i);
			}
		}
		for (int i : toRemove) {
			entrances.Remove(i);
			entrances.Add(i - 1);
		}
		points.RemoveAt(place);
	}
	void addPoint(int place, FVector point) {
		std::vector<int> toRemove;
		for (int i : windows) {
			if (i > place) {
				toRemove.push_back(i);
			}
		}
		for (int i : toRemove) {
			windows.Remove(i);
			windows.Add(i + 1);
		}
		toRemove.clear();
		for (int i : entrances) {
			if (i > place) {
				toRemove.push_back(i);
			}
		}
		for (int i : toRemove) {
			entrances.Remove(i);
			entrances.Add(i + 1);
		}
		points.EmplaceAt(place, point);
		if (windows.Contains(place - 1) || windows.Contains(place + 1)) {
			windows.Add(place);
		}
	}

	FHousePolygon splitAlongMax() {


		if (FVector::Dist(points[0], points[points.Num() - 1]) > 0.1f) {
			UE_LOG(LogTemp, Warning, TEXT("END AND BEGINNING NOT CONNECTED IN splitAlongMax, dist is: %f"), FVector::Dist(points[0], points[points.Num() - 1]));
		}

		SplitStruct p = getSplitProposal(buildLeft, 0.5);
		if (p.p1.X == 0.0f) {
			height = 50;
			return FHousePolygon();
		}

		FHousePolygon newP;
		newP.open = open;
		newP.buildLeft = buildLeft;
		newP.population = population;
		newP.type = type;

		FVector offset = p.p2 - p.p1;
		offset = FRotator(0, 90, 0).RotateVector(offset);
		offset.Normalize();

		newP.points.Add(p.p1);
		if (entrances.Contains(p.min)) {
			newP.entrances.Add(newP.points.Num());
		}
		if (windows.Contains(p.min)) {
			newP.windows.Add(newP.points.Num());
		}
		newP.points.Add(points[p.min]);

		for (int i = p.min + 1; i < p.max; i++) {
			if (entrances.Contains(i)) {
				entrances.Remove(i);
				newP.entrances.Add(newP.points.Num());
			}
			if (windows.Contains(i)) {
				windows.Remove(i);
				newP.windows.Add(newP.points.Num());
			}
			newP.points.Add(points[i]);
		}

		if (entrances.Contains(p.max)) {
			newP.entrances.Add(newP.points.Num());
		}
		if (windows.Contains(p.max)) {
			newP.windows.Add(newP.points.Num());
		}

		std::vector<int32> toRemove;
		for (int32 i : entrances) {
			if (i > p.min)
				toRemove.push_back(i);
		}
		for (int32 i : toRemove) {
			entrances.Remove(i);
			entrances.Add(i - (p.max - p.min) + 2);
		}


		toRemove.clear();
		for (int32 i : windows) {
			if (i > p.min)
				toRemove.push_back(i);
		}
		for (int32 i : toRemove) {
			windows.Remove(i);
			windows.Add(i - (p.max - p.min) + 2);
		}

		newP.points.Add(p.p2);
		newP.points.Add(p.p1);
		
		points.RemoveAt(p.min, p.max - p.min);
		points.EmplaceAt(p.min, p.p1);
		points.EmplaceAt(p.min + 1, p.p2);


		return newP;

	}

	TArray<FHousePolygon> recursiveSplit(float maxArea, float minArea, int depth) {

		double area = getArea();
		//UE_LOG(LogTemp, Warning, TEXT("area of new house: %f"), area);

		TArray<FHousePolygon> tot;

		if (points.Num() < 3 || area <= minArea) {
			//tot.Add(*this);
			return tot;
		}
		else if (depth > 3) {
			tot.Add(*this);
			return tot;
		}
		else if (area > maxArea) {
			FHousePolygon newP = splitAlongMax();
			if (newP.points.Num() > 2) {
				tot = newP.recursiveSplit(maxArea, minArea, depth + 1);
			}
			tot.Append(recursiveSplit(maxArea, minArea, depth + 1));

		}
		else {
			tot.Add(*this);
		}
		return tot;
	}

	TArray<FHousePolygon> refine(float maxArea, float minArea) {


		if (FVector::Dist(points[0], points[points.Num() - 1]) > 0.1f) {
			UE_LOG(LogTemp, Warning, TEXT("END AND BEGINNING NOT CONNECTED IN refine, dist is: %f"), FVector::Dist(points[0], points[points.Num() - 1]));
		}

		decreaseEdges();
		TArray<FHousePolygon> toReturn;
		if (!open) {
			toReturn.Append(recursiveSplit(maxArea, minArea, 0));
		}
		else 
			toReturn.Add(*this);
		return toReturn;
	}
};


USTRUCT(BlueprintType)
struct FLine {
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector p1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector p2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float width;

	FVector getMiddle() {
		return (p2 - p1) / 2 + p1;
	}
};

USTRUCT(BlueprintType)
struct FRoadSegment : public FLine
{
	//GENERATED_BODY();

	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector beginTangent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector endTangent = FVector(0.0f, 0.0f, 0.0f);
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
	RoomType type;

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
FVector intersection(FPolygon &p1, TArray<FPolygon> &p2);
FVector intersection(FPolygon &p1, FPolygon &p2);
FVector intersection(FVector p1, FVector p2, FVector p3, FVector p4);
FVector intersection(FVector p1, FVector p2, FPolygon p);
bool testCollision(TArray<FVector> tangents, TArray<FVector> vertices1, TArray<FVector> vertices2, float collisionLeniency);
float randFloat();
FVector NearestPointOnLine(FVector linePnt, FVector lineDir, FVector pnt);

class CITY_API BaseLibrary
{
public:
	BaseLibrary();
	~BaseLibrary();


	static TArray<FMetaPolygon> getSurroundingPolygons(TArray<FLine> &segments, TArray<FLine> &blocking, float stdWidth, float extraLen, float extraRoadLen, float width, float middleOffset);


};