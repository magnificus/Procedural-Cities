// Fill out your copyright notice in the Description page of Project Settings.

#pragma once



#include "stdlib.h"
#include <queue>
#include "GameFramework/Actor.h"
#include "Components/SplineMeshComponent.h"
#include "City.h"
#include "Algo/Reverse.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

#include "BaseLibrary.generated.h"

/**
 * 
 */
struct SplitStruct {
	int min;
	int max;
	FVector p1;
	FVector p2;
};

static float noiseXOffset = 0;
static float noiseYOffset = 0;

struct FPolygon;
struct FMaterialPolygon;

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
	store UMETA(DisplayName = "Store"),
	restaurant UMETA(DisplayName = "Restaurant")
};

UENUM(BlueprintType)
enum class PolygonType : uint8
{
	interior 	UMETA(DisplayName = "Interior"),
	exterior UMETA(DisplayName = "Exterior"),
	exteriorSnd UMETA(DisplayName = "Exterior Secondary"),
	floor UMETA(DisplayName = "Floor"),
	window UMETA(DisplayName = "Window"),
	windowFrame UMETA(DisplayName = "Window Frame"),
	occlusionWindow UMETA(DisplayName = "Occlusion Window"),
	roof UMETA(DisplayName = "Roof"),
	green UMETA(DisplayName = "Green"),
	concrete UMETA(DisplayName = "Concrete"),
	roadMiddle UMETA(DisplayName = "Middle of road line")
};

void getMinMax(float &min, float &max, FVector tangent, TArray<FVector> points);

//void getMinMax(float &min, float &max, FVector tangent, FVector v1, FVector v2, FVector v3, FVector v4);
FVector intersection(FPolygon &p1, TArray<FPolygon> &p2);
FVector intersection(FPolygon &p1, FPolygon &p2);
FVector intersection(FVector p1, FVector p2, FVector p3, FVector p4);
FVector intersection(FVector p1, FVector p2, FPolygon p);
bool selfIntersection(FPolygon &p1);
bool testCollision(FPolygon &, TArray<FPolygon> &, float leniency, FPolygon &);
bool testCollision(FPolygon &, FPolygon &, float leniency);
bool testCollision(TArray<FVector> tangents, TArray<FVector> vertices1, TArray<FVector> vertices2, float collisionLeniency);
float randFloat();
FVector NearestPointOnLine(FVector linePnt, FVector lineDir, FVector pnt);
TArray<FMaterialPolygon> getSidesOfPolygon(FPolygon p, PolygonType type, float width);
TArray<FMaterialPolygon> fillOutPolygons(TArray<FMaterialPolygon> &first);
FVector intersection(FVector p1, FVector p2, FVector p3, FVector p4);




static FVector getNormal(FVector p1, FVector p2, bool left) {
	return FRotator(0, left ? 90 : 270, 0).RotateVector(p2 - p1);
}

FPolygon getPolygon(FRotator rot, FVector pos, FString name, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map);


USTRUCT(BlueprintType)
struct FPolygon
{
	GENERATED_USTRUCT_BODY();

	FVector normal = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite)
		TArray<FVector> points;

	bool getIsClockwise() {
		float tot = 0;
		FVector first = points[0];
		//offset(-first);
		for (int i = 1; i < points.Num() + 1; i++) {
			tot += (points[i-1].X*0.01 * points[i%points.Num()].Y*0.01 - points[i%points.Num()].X*0.01 * points[i-1].Y*0.01);
		}
		//offset(first);
			UE_LOG(LogTemp, Warning, TEXT("getisclockwise res : %f"), tot);
		return tot > 0;
	}

	FVector& operator[] (int index) {
		return points[index];
	}

	void operator+= (FVector toAdd) {
		points.Add(toAdd);
	}

	FVector getCenter() {
		FVector center = FVector(0, 0, 0);
		double totLen = 0;
		for (int i = 1; i < points.Num() + 1; i++) {
			float len = (points[i%points.Num()] - points[i - 1]).Size();
			center += ((points[i%points.Num()] - points[i - 1]) / 2 + points[i - 1])*len;
			totLen += len;
		}
		center /= totLen;
		return center;
	}

	// not totally random, favors placement closer to the sides a bit, but good enough
	FVector getRandomPoint(bool left, float minDist) {
		int place = FMath::RandRange(1, points.Num());
		FVector tangent = (points[place%points.Num()] - points[place - 1]);
		FVector beginPlace = FMath::FRand() * tangent + points[place - 1];
		tangent.Normalize();
		FVector pointNormal = FRotator(0, left ? 90 : 270, 0).RotateVector(tangent);
		int a;
		FVector target;
		getSplitCorrespondingPoint(place, beginPlace, tangent, pointNormal, a, target);
		if (target.X == 0.0f || FVector::Dist(beginPlace, target) < minDist * 2)
			return FVector(0, 0, 0);
		FVector point = FMath::FRandRange(minDist, FVector::Dist(beginPlace, target) - minDist) * pointNormal + beginPlace;
		return point;
	}

	// only cares about dimensions X and Y, not Z
	double getArea() {
		double tot = 0;

		for (int i = 0; i < points.Num(); i++) {
			tot += 0.0001*(points[i].X * points[(i + 1) % points.Num()].Y);
			tot -= 0.0001*(points[i].Y * points[(i + 1) % points.Num()].X);
		}

		tot *= 0.5;
		return std::abs(tot);
	}

	void offset(FVector offset) {
		for (FVector &f : points) {
			f += offset;
		}
	};

	void rotate(FRotator rotation) {
		FVector center = getCenter();
		for (FVector &f : points) {
			f = rotation.RotateVector(f - center) + center;
		}
	}

	// removes corners that stick out in an ugly way
	void clipEdges(float maxDot) {
		bool changed = true;
		while (changed) {
			changed = decreaseEdges();
			for (int i = 1; i < points.Num() - 1; i++) {
				FVector tan1 = points[i] - points[i - 1];
				FVector tan2 = points[i + 1] - points[i];
				tan1.Normalize();
				tan2.Normalize();
				float dist = FVector::DotProduct(tan1, tan2);
				if (dist < maxDot) {
					points.RemoveAt(i);
					i--;
					changed = true;
					break;
				}

			}
		}

		// untangle

		for (int i = 1; i < points.Num(); i++) {
			FVector tan1 = points[i] - points[i - 1];
			tan1.Normalize();
			for (int j = i + 2; j < points.Num() + 1; j++) {
				FVector tan2 = points[j%points.Num()] - points[j-1];
				tan2.Normalize();
				FVector res = intersection(points[i - 1], points[i], points[j - 1], points[j%points.Num()] - tan2*10);
				if (res.X != 0.0f) {
					TArray<FVector> newPoints;
					for (int k = 0; k < i; k++) {
						newPoints.Add(points[k]);
					}
					newPoints.Add(res);
					for (int k = j; k < points.Num(); k++) {
						newPoints.Add(points[k]);
					}
					points = newPoints;
					i = 0;
					break;

				}
			}
		}
	}





	// this method merges polygon sides when possible, and combines points
	bool decreaseEdges() {
		float distDiffAllowed = 40000;
		bool hasModified = false;
		for (int i = 1; i < points.Num(); i++) {
			if (FVector::DistSquared(points[i - 1], points[i]) < distDiffAllowed) {
				points.RemoveAt(i);
				hasModified = true;
				i--;
			}
		}

		return hasModified;
	}

	// assumes at least 3 points in polygon
	FVector getDirection() {
		FVector res = normal.Size() < 1.0f ? FVector::CrossProduct(points[1] - points[0], points[points.Num()-1] - points[0]) : normal;
		res.Normalize();
		return res;
	}

	void reverse() {
		Algo::Reverse(points);
	}

	void getSplitCorrespondingPoint(int begin, FVector point, FVector tangent, FVector inNormal, int &split, FVector &p2) {
		float closest = 10000000.0f;
		for (int i = 1; i < points.Num()+1; i++) {
			if (i == begin) {
				continue;
			}
			FVector curr = intersection(point, point + inNormal * 100000, points[i - 1], points[i%points.Num()]);
			if (curr.X != 0.0f && FVector::Dist(curr, point) < closest) {
				closest = FVector::Dist(curr, point);
				split = i;
				p2 = curr;
			}
		}

	}

	SplitStruct getSplitProposal(bool buildLeft, float approxRatio) {

		if (points.Num() < 3) {
			return SplitStruct{ 0, 0, FVector(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 0.0f) };
		}
		int longest = -1;

		float longestLen = 0.0f;

		FVector curr;
		for (int i = 1; i < points.Num()+1; i++) {
			float dist = FVector::DistSquared(points[i%points.Num()], points[i - 1]);
			if (dist > longestLen) {
				longestLen = dist;
				longest = i;
			}
		}
		if (longest == -1) {
			return SplitStruct{ 0, 0, FVector(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 0.0f) };
		}
		curr = points[longest%points.Num()] - points[longest - 1];
		curr.Normalize();

		FVector middle = (points[longest%points.Num()] - points[longest - 1]) * approxRatio + points[longest - 1];
		FVector p1 = middle;
		int split = 0;
		FVector p2 = FVector(0.0f, 0.0f, 0.0f);
		FVector tangent = FRotator(0, buildLeft ? 90 : 270, 0).RotateVector(curr);
		tangent.Normalize();

		getSplitCorrespondingPoint(longest, middle, curr, tangent, split, p2);

		if (p2.X == 0.0f || p1.X == 0.0f) {
			UE_LOG(LogTemp, Warning, TEXT("UNABLE TO SPLIT, NO CORRESPONDING SPLIT POINT FOR POLYGON"));
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

	FVector getPointDirection(int place, bool left) {
		int prev = place == 0 ? points.Num() - 1 : place - 1;
		//FVector dir1 = points[place] - points[prev];
		//FVector dir2 = points[(place + 1)%points.Num()] - points[place];

		FVector dir1 = getNormal(points[place], points[prev], left);
		FVector dir2 = getNormal(points[(place + 1) % points.Num()], points[place] , left);

		dir1.Normalize();
		dir2.Normalize();
		//dir1 = FRotator(0, left ? 90 : 270, 0).RotateVector(dir1);
		//dir2 = FRotator(0, left ? 90 : 270, 0).RotateVector(dir2);

		FVector totDir = dir1 + dir2;
		//totDir.Normalize();

		totDir.X = std::min(totDir.X, 1.0f);
		totDir.Y = std::min(totDir.Y, 1.0f);
		totDir.Z = std::min(totDir.Z, 1.0f);
		totDir.X = std::max(totDir.X, -1.0f);
		totDir.Y = std::max(totDir.Y, -1.0f);
		totDir.Z = std::max(totDir.Z, -1.0f);

		//totDir.Normalize();
		return totDir;
	//}

	}

	void symmetricShrink(float length, bool left) {
		for (int i = 0; i < points.Num(); i++) {
			//float distToCenter = FVector::Dist(getCenter(), points[i]);
			points[i] += getPointDirection(i, left)*length;
		}
	}

};





USTRUCT(BlueprintType)
struct FMaterialPolygon : public FPolygon {
	GENERATED_USTRUCT_BODY();

	PolygonType type = PolygonType::exterior;
	float width = 20;
};

USTRUCT(BlueprintType)
struct FMeshInfo {
	GENERATED_USTRUCT_BODY();

	FMeshInfo() :
		description(""),
		transform(FTransform()),
		instanced(true) {}
	FMeshInfo(const FString &description,const FTransform &transform, const bool &instanced) :
		description(description),
		transform(transform),
		instanced(instanced) {}
	FMeshInfo(const FString &description, const FTransform &transform) :
		description(description),
		transform(transform),
		instanced(true) {}


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FTransform transform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool instanced;
};

UENUM(BlueprintType)
enum class WindowType : uint8
{
	rectangular 	UMETA(DisplayName = "Rectangular"),
	cross UMETA(DisplayName = "Cross"),
	rectangularHorizontalBigger UMETA(DisplayName = "Rectangular Horizontal Bigger"),
	verticalLines UMETA(DisplayName = "Vertical Lines")
};


USTRUCT(BlueprintType)
struct FRoomInfo {

	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FMaterialPolygon> pols;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FMeshInfo> meshes;


	void offset(FVector offset) {
		for (FPolygon &p : pols)
			p.offset(offset);
		for (FMeshInfo &f : meshes)
			f.transform.SetTranslation(f.transform.GetTranslation() + offset);
	}
};


UENUM(BlueprintType)
enum class SimplePlotType : uint8
{
	undecided UMETA(DisplayName = "Undecided"),
	asphalt UMETA(DisplayName = "Asphalt"),
	green UMETA(DisplayName = "Green")
};

USTRUCT(BlueprintType)
struct FSimplePlot {
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPolygon pol;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMeshInfo> meshes;

	SimplePlotType type = SimplePlotType::undecided;


	void decorate() {
		decorate(TArray<FPolygon>());
	}


	void decorate(TArray<FPolygon> blocking) {
		float area = pol.getArea();
		switch (type) {
		case SimplePlotType::undecided:
		case SimplePlotType::green: {
			float treeAreaRatio = 0.01;
			for (int i = 0; i < treeAreaRatio*area; i++) {
				FVector point = pol.getRandomPoint(true, 150);
				if (point.X != 0.0f) {
					FPolygon temp;
					temp.points.Add(point);
					temp.points.Add(point + FVector(1, 1, 0));
					temp.points.Add(point + FVector(0, 1, 0));
					temp.points.Add(point + FVector(1, 0, 0));
					bool collision = false;
					for (FPolygon &p : blocking) {
						if (testCollision(p, temp, 0)) {
							collision = true;
							break;
						}
					}
					if (!collision) {
						FMeshInfo toAdd;
						toAdd.description = "tree";
						toAdd.transform = FTransform(point);
						toAdd.instanced = false;
						meshes.Add(toAdd);
					}
				}
			}
			break;
		}
		case SimplePlotType::asphalt: {
			//attemptPlace()
			break;
		}
		}
	}
};
USTRUCT(BlueprintType)
struct FHouseInfo {
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRoomInfo roomInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSimplePlot> remainingPlots;

};






static FVector middle(FVector p1, FVector p2) {
	return (p2 + p1) / 2;
}

static TArray<int32> getIntList(int32 min, int32 max) {
	TArray<int32> ints;
	for (int32 i = min; i < max; i++) {
		ints.Add(i);
	}
	return ints;
}


static void removeAllButOne(TSet<int32> &entries) {
	TArray<int> numbers;
	if (entries.Num() < 2) {
		return;
	}
	for (int32 i : entries) {
		numbers.Add(i);
	}
	int place = FMath::Rand() % numbers.Num();
	numbers.RemoveAt(place);
	for (int32 i : numbers) {
		entries.Remove(i);
	}
}





USTRUCT(BlueprintType)
struct FMetaPolygon : public FPolygon
{

	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool open = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool buildLeft;

	void checkOrientation() {
		if (!getIsClockwise())
			reverse();
		buildLeft = true;

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
	living UMETA(DisplayName = "Living Room"),
	closet UMETA(DisplayName = "Closet"),
	hallway UMETA(DisplayName = "Hallway"),
	storeFront UMETA(DisplayName = "Store Front"),
	storeBack UMETA(DisplayName = "Store Back"),
	restaurant UMETA(DisplayName = "Restaurant")


};


static bool splitableType(SubRoomType type) {
	switch (type){
	case SubRoomType::meeting: return false;
	case SubRoomType::work: return true;
	case SubRoomType::bath: return false;
	case SubRoomType::empty: return true;
	case SubRoomType::corridor: return true;
	case SubRoomType::bed: return false;
	case SubRoomType::kitchen: return true;
	case SubRoomType::living: return true;
	case SubRoomType::closet: return false;
	case SubRoomType::hallway: return true;
	case SubRoomType::storeBack: return true;
	case SubRoomType::storeFront: return true;
	}
	return true;
}

static bool isOnLine(FVector point, FVector p1, FVector p2) {
	return std::abs(FVector::Dist(point, p1) + FVector::Dist(point, p2) - FVector::Dist(p1, p2)) < 1.0f;
}


struct RoomSpecification {
	float minArea;
	float maxArea;
	SubRoomType type;
};

struct RoomBlueprint {
	TArray<RoomSpecification> needed;
	TArray<RoomSpecification> optional;
};





struct FRoomPolygon : public FPolygon
{
	// sides of the polygon where windows are allowed
	TSet<int32> windows;
	// sides of the polygon with entrances from my end
	TSet<int32> entrances;
	// sides of polygons that are towards the outside
	TSet<int32> exteriorWalls;
	// a subset of entrances where the entrance is not in the middle but in a specified position
	TMap<int32, FVector> specificEntrances;
	TSet<int32> toIgnore;
	// members of toIgnore that correspond to an entrance from the neighboring polygon
	TMap<int32, TSet<FRoomPolygon*>> passiveConnections;
	// pointers to recievers of entrances from my end
	TMap<FRoomPolygon*, int32> activeConnections;

	WindowType windowType;

	bool canRefine = true;
	SubRoomType type = SubRoomType::empty;

	void updateConnections(int num, FVector &inPoint, FRoomPolygon* newP, bool first, int passiveNum) {
		TArray<FRoomPolygon*> toRemove;

		TSet<FRoomPolygon*> childPassive;

		// move collisions to the left
		if (specificEntrances.Contains(num)) {
			//FVector entrancePoint = specificEntrances.Contains(num) ? specificEntrances[num] : middle(points[num], points[num - 1]);
			if (FVector::Dist(inPoint, specificEntrances[num]) < 100) {
				FVector tangent = points[num%points.Num()] - points[num - 1];
				tangent.Normalize();
				float toMove = FVector::Dist(inPoint, specificEntrances[num] - tangent * 100);
				inPoint -= toMove * tangent;
			}
		}

		if (!passiveConnections.Contains(num)) {
			return;
		}
		for (FRoomPolygon* p1 : passiveConnections[num]) {
			int loc = p1->activeConnections[this];
			// this is the entrance, we're not allowed to place a wall that collides with this, try to fit newP on the other side of the entrance if possible, making it a single-entry room
			FVector point = p1->specificEntrances[loc]; //middle(p1->points[loc], p1->points[loc - 1]);
			float dist = FVector::Dist(point, inPoint);
			if (dist < 100) {
				FVector tangent = points[num%points.Num()] - points[num - 1];
				tangent.Normalize();
				float toMove = FVector::Dist(inPoint, point - tangent * 100);
				inPoint -= toMove * tangent;
				// our child is now free from our master, but we're still slaves
			}
			else {
				// new cut doesn't interfere overlap with the entrance, now to see who gets it
				if (first && isOnLine(point, points[num - 1], inPoint) || !first && !isOnLine(point, points[num - 1], inPoint)) {
					// i got it, no change
				}
				else {
					// my child got it
					childPassive.Add(p1);
					p1->activeConnections.Remove(this);
					p1->activeConnections.Add(newP, loc);
					toRemove.Add(p1);
					newP->toIgnore.Add(passiveNum);
				}
			}
		}
		if (childPassive.Num() > 0)
			newP->passiveConnections.Add(passiveNum, childPassive);

		for (FRoomPolygon* t : toRemove) {
			passiveConnections[num].Remove(t);
		}
	}


	FRoomPolygon* splitAlongMax(float approxRatio, bool entranceBetween) {
		SplitStruct p = getSplitProposal(false, approxRatio);
		if (p.p1.X == 0.0f) {
			return NULL;
		}
		FRoomPolygon* newP = new FRoomPolygon();
		updateConnections(p.min, p.p1, newP, true, 1);

		if (entrances.Contains(p.min)){
			// potentially add responsibility of child
			FVector entrancePoint = specificEntrances.Contains(p.min) ? specificEntrances[p.min] : middle(p.p1, points[p.min-1]);
			if (isOnLine(entrancePoint, p.p1, points[p.min])) {
				if (specificEntrances.Contains(p.min))
					newP->specificEntrances.Add(1, entrancePoint);
				newP->entrances.Add(1);
				TArray<FRoomPolygon*> toRemove;
				for (auto &pair : activeConnections) {
					if (pair.Value == p.min) {
						for (auto &pair2 : pair.Key->passiveConnections) {
							if (pair2.Value.Contains(this)) {
								pair.Key->passiveConnections[pair2.Key].Add(newP);
								pair.Key->passiveConnections[pair2.Key].Remove(this);
								break;
							}
						}
						newP->activeConnections.Add(pair.Key, 1);
						toRemove.Add(pair.Key);
						break;
					}
				}
				specificEntrances.Remove(p.min);
				entrances.Remove(p.min);
				for (FRoomPolygon* p2 : toRemove) {
					activeConnections.Remove(p2);
				}
			}

		}


		if (windows.Contains(p.min)) {
			newP->windows.Add(1);
		}
		if (exteriorWalls.Contains(p.min)) {
			newP->exteriorWalls.Add(1);
		}

		// move intersection to make more sense if possible
		FVector res = intersection(p.p1, p.p1 + FRotator(0, 270, 0).RotateVector(points[p.min] - points[p.min - 1]) * 50, points[p.max%points.Num()], points[p.max - 1]);
		if (res.X != 0.0f)
			p.p2 = res;
		if (toIgnore.Contains(p.min)) {
			newP->toIgnore.Add(1);
		}
		newP->points.Add(p.p1);

		newP->points.Add(points[p.min]);

		for (int i = p.min + 1; i < p.max; i++) {
			if (entrances.Contains(i)) {

				if (specificEntrances.Contains(i))
					newP->specificEntrances.Add(newP->points.Num(), specificEntrances[i]);
				newP->entrances.Add(newP->points.Num());

				entrances.Remove(i);
				specificEntrances.Remove(i);
				TArray<FRoomPolygon*> toRemove;
				for (auto &pair : activeConnections) {
					if (pair.Value == i) {
						FRoomPolygon *p1 = pair.Key;
						for (int j = 1; j < p1->points.Num(); j++) {
							if (p1->passiveConnections.Contains(j) && p1->passiveConnections[j].Contains(this)) {
								toRemove.Add(p1);
								//activeConnections.Remove(p1);
								p1->passiveConnections[j].Remove(this);
								p1->passiveConnections[j].Add(newP);
								newP->activeConnections.Add(p1, newP->points.Num());
								break;
							}
						}
					}
				}
				for (FRoomPolygon *f : toRemove) {
					activeConnections.Remove(f);
				}
	

			}
			if (windows.Contains(i)) {
				windows.Remove(i);
				newP->windows.Add(newP->points.Num());
			}
			if (exteriorWalls.Contains(i)) {
				exteriorWalls.Remove(i);
				newP->exteriorWalls.Add(newP->points.Num());
			}
			if (toIgnore.Contains(i)) {
				toIgnore.Remove(i);
				newP->toIgnore.Add(newP->points.Num());
				if (passiveConnections.Contains(i)) {
					TSet<FRoomPolygon*> pols = passiveConnections[i];
					newP->passiveConnections.Add(newP->points.Num(), pols);
					passiveConnections.Remove(i);
					for (FRoomPolygon *p1 : pols) {
						p1->activeConnections.Add(newP, p1->activeConnections[this]);
						p1->activeConnections.Remove(this);
					}
				}
			}
			newP->points.Add(points[i]);
		}

		updateConnections(p.max, p.p2, newP, false, newP->points.Num());


		if (entrances.Contains(p.max)){
			FVector entrancePoint = specificEntrances.Contains(p.max) ? specificEntrances[p.max] : middle(p.p2, points[p.max%points.Num()]);
			if (isOnLine(entrancePoint, p.p2, points[p.max-1])) {
				if (specificEntrances.Contains(p.max))
					newP->specificEntrances.Add(newP->points.Num(), entrancePoint);

				newP->entrances.Add(newP->points.Num());
				TArray<FRoomPolygon*> toRemove;
				for (auto &pair : activeConnections) {
					if (pair.Value == p.max) {
						for (auto &pair2 : pair.Key->passiveConnections) {
							if (pair2.Value.Contains(this)) {
								pair.Key->passiveConnections[pair2.Key].Add(newP);
								pair.Key->passiveConnections[pair2.Key].Remove(this);
								break;
							}
						}
						newP->activeConnections.Add(pair.Key, newP->points.Num());
						toRemove.Add(pair.Key);
						break;
					}
				}
				specificEntrances.Remove(p.max);
				entrances.Remove(p.max);
				for (FRoomPolygon* p2 : toRemove) {
					activeConnections.Remove(p2);
				}
			}
		}
		if (windows.Contains(p.max)) {
			newP->windows.Add(newP->points.Num());
		}
		if (exteriorWalls.Contains(p.max)) {
			newP->exteriorWalls.Add(newP->points.Num());
		}


		if (toIgnore.Contains(p.max)) {
			newP->toIgnore.Add(newP->points.Num());
		}



		TSet<int32> newList;
		TMap<int32, FVector> newSpecificList;
		for (int32 i : entrances) {
			if (i >= p.max){
				newList.Add(i - (p.max - p.min) + 2);
				if (specificEntrances.Contains(i))
					newSpecificList.Add(i - (p.max - p.min) + 2, specificEntrances[i]);
			}
			else {
				newList.Add(i);
				if (specificEntrances.Contains(i))
					newSpecificList.Add(i, specificEntrances[i]);
			}
		}
		entrances = newList;
		specificEntrances = newSpecificList;

		newList.Empty();
		for (int32 i : windows) {
			if (i >= p.max)
				newList.Add(i - (p.max - p.min) + 2);
			else
				newList.Add(i);
		}
		windows = newList;

		newList.Empty();
		for (int32 i : exteriorWalls) {
			if (i >= p.max)
				newList.Add(i - (p.max - p.min) + 2);
			else
				newList.Add(i);
		}
		exteriorWalls = newList;

		newList.Empty();
		for (int32 i : toIgnore) {
			if (i >= p.max)
				newList.Add(i - (p.max - p.min) + 2);
			else
				newList.Add(i);
		}
		toIgnore = newList;



		TMap<FRoomPolygon*, int32> newActive;
		for (auto &pair : activeConnections) {
			if (pair.Value >= p.max) {
				newActive.Add(pair.Key, pair.Value - (p.max - p.min) + 2);
			}
			else {
				newActive.Add(pair.Key, pair.Value);
			}
		}
		activeConnections = newActive;

		TMap<int32, TSet<FRoomPolygon*> > newPassive;
		for (auto &pair : passiveConnections) {
			if (pair.Key >= p.max) {
				newPassive.Add(pair.Key - (p.max - p.min) + 2, pair.Value );
			}
			else {
				newPassive.Add(pair.Key, pair.Value);
			}
		}
		passiveConnections = newPassive;

		newP->points.Add(p.p2);

		//// dont place the wall twice
		toIgnore.Add(p.min+1);
		//	// entrance to next room
		if (entranceBetween){
			TSet<FRoomPolygon*> passive = newP->passiveConnections.Contains(newP->points.Num()) ? newP->passiveConnections[newP->points.Num()] : TSet<FRoomPolygon*>();
			passive.Add(newP);
			passiveConnections.Add(p.min + 1, passive);
			newP->specificEntrances.Add(newP->points.Num(), middle(p.p1, p.p2));
			newP->entrances.Add(newP->points.Num());
			newP->activeConnections.Add(this, newP->points.Num());
		}

		//newP->points.Add(p.p1);


		points.RemoveAt(p.min, p.max - p.min);
		points.EmplaceAt(p.min, p.p1);
		//if (p.max != points.Num())
			points.EmplaceAt(p.min + 1, p.p2);

		return newP;
	}

	FTransform attemptGetPosition(TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes, bool windowAllowed, int testsPerSide, FString string, FRotator offsetRot, FVector offsetPos, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map, bool onWall);
	bool attemptPlace(TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes, bool windowAllowed, int testsPerSide, FString string, FRotator offsetRot, FVector offsetPos, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map, bool onWall);


	TArray<FRoomPolygon*> fitSpecificationOnRooms(TArray<RoomSpecification> specs, TArray<FRoomPolygon*> &remaining, bool repeating, bool useMin) {
		TArray<FRoomPolygon*> toReturn;
		float area;
		float minPctSplit = 0.25f;
		bool couldPlace = false;
		int c1 = 0;
		do {
			couldPlace = false;
			for (RoomSpecification r : specs) {
				bool found = false;
				bool smaller = false;
				float maxAreaAllowed = useMin ? (r.maxArea + r.minArea) / 2 : r.maxArea;
				for (int i = 0; i < remaining.Num(); i++) {
					FRoomPolygon *p = remaining[i];
					area = p->getArea();
					smaller = smaller || (area > maxAreaAllowed);
					if (area <= maxAreaAllowed && area >= r.minArea){// && p->type != SubRoomType::hallway) {
						// found fitting room
						p->type = r.type;
						toReturn.Add(p);
						remaining.RemoveAt(i);
						found = true;
						couldPlace = true;
						break;
					}
				}
				// could not find a fitting room since all remaining are too big, cut them down to size
				if (!found && smaller) {
					FRoomPolygon *target = remaining[0];
					int targetNum = 0;
					float scale = 0.0f;
					for (int i = 0; i < remaining.Num(); i++) {
						target = remaining[i];
						targetNum = i;
						scale = r.minArea / target->getArea();
						if (scale < 1.0f) {
							break;
						}
					}
					remaining.RemoveAt(targetNum);
					int count = 0;
					while (scale < minPctSplit && count++ < 5) {
						FRoomPolygon* newP = target->splitAlongMax(0.6, true);
						if (newP == nullptr) {
							break;
						}
						remaining.EmplaceAt(0, newP);
						scale = r.minArea / target->getArea();

					}
					if (target->getArea() <= maxAreaAllowed && target->getArea() >= r.minArea){// && target->type != SubRoomType::hallway) {
						target->type = r.type;
						//remaining.RemoveAt(targetNum);
						toReturn.Add(target);
						couldPlace = true;

					}
					else if (scale > minPctSplit) {
						FRoomPolygon* newP = target->splitAlongMax(r.minArea / target->getArea(), true);
						if (newP == nullptr) {
							couldPlace = false;
						}
						else {
							newP->type = r.type;
							toReturn.Add(newP);
							couldPlace = true;
						}
						remaining.EmplaceAt(0, target);
					}
					else {
						remaining.EmplaceAt(0, target);
					}
						
				}
			}
		} while (repeating && couldPlace && c1++ < 5);

		return toReturn;


	}

	int getTotalConnections() {
		int totPassive = 0;
		for (auto &list : passiveConnections) {
			totPassive += list.Value.Num();
		}
		return entrances.Num()	 + totPassive;
	}


	// post placement part of algorithm, makes sure the required rooms are there by any means neccesary
	void postFit(TArray<FRoomPolygon*> &rooms, TArray<RoomSpecification> neededRooms, TArray<RoomSpecification> optionalRooms){

		if (neededRooms.Num() > rooms.Num()) {
			return;
		}
		//TMap<RoomSpecification, int32> required;
		TMap<SubRoomType, int32> need;
		for (RoomSpecification r : neededRooms) {
			if (need.Contains(r.type))
				need[r.type] ++;
			else
				need.Add(r.type, 1);
		}
		for (FRoomPolygon *p : rooms) {
			if (p->entrances.Num() > p->specificEntrances.Num()) {
				p->type = SubRoomType::hallway;
			}
			else if (!splitableType(p->type) && p->getTotalConnections() > 1) {
				p->type = SubRoomType::corridor;
			}
			if (need.Contains(p->type))
				need[p->type] --;
		}

		TArray<SubRoomType> remaining;
		for (auto &spec : need) {
			for (int i = 0; i < spec.Value; i++) {
				remaining.Add(spec.Key);
			}
		}

		while (remaining.Num() > 0) {
			SubRoomType current = remaining[remaining.Num() - 1];

			bool found = false;
			for (FRoomPolygon *p : rooms) {
				// first pick non-needed rooms
				if (!need.Contains(p->type) && (splitableType(current) || p->getTotalConnections() < 2)) {
					p->type = current;
					found = true;
					break;
				}
			}
			if (found) {
				remaining.RemoveAt(remaining.Num() - 1);
				need[current] --;
				continue;
			}
			else {
				if (!splitableType(current)) {
					// consider picking rooms that already contains an essential room
					for (FRoomPolygon *p : rooms) {
						if (p->getTotalConnections() < 2) {
							SubRoomType prevType = p->type;
							remaining.RemoveAt(remaining.Num() - 1);
							need[current]--;
							if (++need[prevType] > 0) {
								remaining.Add(prevType);
							}
							p->type = current;
							found = true;
							break;
						}
					}
					// if this fails, there are no possible places for this room, TODO create a new room somewhere
					need[current] --;
					remaining.RemoveAt(remaining.Num() - 1);
				}
				else {
					// if this fails, there are no possible places for this room, TODO create a new room somewhere
					need[current] --;
					remaining.RemoveAt(remaining.Num() - 1);
				}

			}

		}
	}

	TArray<FRoomPolygon*> getRooms(RoomBlueprint blueprint) {
		TArray<FRoomPolygon*> rooms;
		TArray<FRoomPolygon*> remaining;
		FRoomPolygon* thisP = new FRoomPolygon();
		*thisP = *this;
		thisP->type = SubRoomType::hallway;
		remaining.Add(thisP);
		removeAllButOne(remaining[0]->entrances);

		float standardAreaRequired = 0;
		// if the sum of the average room areas for neccesary rooms is greater than the total area of the apartment, use rooms as small as possible
		for (RoomSpecification r : blueprint.needed) {
			standardAreaRequired += (r.maxArea + r.minArea) / 2;
		}
		bool minimizeRoomSizes = standardAreaRequired > getArea();
		rooms.Append(fitSpecificationOnRooms(blueprint.needed, remaining, false, minimizeRoomSizes));
		rooms.Append(fitSpecificationOnRooms(blueprint.optional, remaining, true, false));

		rooms.Append(remaining);
		TArray<SubRoomType> neededTypes;
		for (auto &a : blueprint.needed) {
			neededTypes.Add(a.type);
		}
		postFit(rooms, blueprint.needed, blueprint.optional);



		return rooms;
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
	SimplePlotType simplePlotType;

	bool canBeModified = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSet<int32> entrances;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSet<int32> windows;

	void removePoint(int place) {
		if (points.Num() <= place) {
			return;
		}
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
			if (i >= place) {
				toRemove.push_back(i);
			}
		}
		for (int i : toRemove) {
			windows.Remove(i);
			windows.Add(i + 1);
		}
		toRemove.clear();
		for (int i : entrances) {
			if (i >= place) {
				toRemove.push_back(i);
			}
		}
		for (int i : toRemove) {
			entrances.Remove(i);
			entrances.Add(i + 1);
		}
		points.EmplaceAt(place, point);
	}

	FHousePolygon splitAlongMax(float spaceBetween) {

		SplitStruct p = getSplitProposal(true, 0.5);
		if (p.p1.X == 0.0f) {
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
		//newP.points.Add(p.p1);
		
		points.RemoveAt(p.min, p.max - p.min);
		points.EmplaceAt(p.min, p.p1);
		points.EmplaceAt(p.min + 1, p.p2);

		//newP.checkOrientation();
		return newP;

	}

	TArray<FHousePolygon> recursiveSplit(float maxArea, float minArea, int depth, float spaceBetween) {

		double area = getArea();
		//UE_LOG(LogTemp, Warning, TEXT("area of new house: %f"), area);

		TArray<FHousePolygon> tot;

		if (points.Num() < 3 || area <= minArea) {
			//tot.Add(*this);
			return tot;
		}
		else if (depth > 2) {
			tot.Add(*this);
			return tot;
		}
		else if (area > maxArea) {
			FHousePolygon newP = splitAlongMax(spaceBetween);
			if (newP.points.Num() > 2) {
				tot = newP.recursiveSplit(maxArea, minArea, depth + 1, spaceBetween);
			}
			tot.Append(recursiveSplit(maxArea, minArea, depth + 1, spaceBetween));

		}
		else {
			tot.Add(*this);
		}
		return tot;
	}

	TArray<FHousePolygon> refine(float maxArea, float minArea, float spaceBetween) {

		decreaseEdges();
		TArray<FHousePolygon> toReturn;
		if (!open) {
			toReturn.Append(recursiveSplit(maxArea, minArea, 0, spaceBetween));
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
		return (p1 + p2) / 2;//(p2 - p1) / 2 + p1;
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
	float time;
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
struct FCityDecoration {
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMaterialPolygon> polygons;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FMeshInfo> meshes;

};

USTRUCT(BlueprintType)
struct FPlotInfo {
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FHousePolygon> houses;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSimplePlot> leftovers;
};

USTRUCT(BlueprintType)
struct FPlotPolygon : public FMetaPolygon{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float population;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	RoomType type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	SimplePlotType simplePlotType;

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



class CITY_API BaseLibrary
{
public:
	BaseLibrary();
	~BaseLibrary();

	UFUNCTION(BlueprintCallable, Category = conversion)
	static TArray<FMaterialPolygon> getSimplePlotPolygons(TArray<FSimplePlot> plots);
	static TArray<FMetaPolygon> getSurroundingPolygons(TArray<FRoadSegment> &segments, TArray<FRoadSegment> &blocking, float stdWidth, float extraLen, float extraRoadLen, float width, float middleOffset);


};