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
	double tot = 0;

	for (int i = 0; i < points.Num() - 1; i++) {
		tot += 0.0001*(points[i].X * points[i + 1].Y);
		tot -= 0.0001*(points[i].Y * points[i+1].X);
	}
	tot *= 0.5;
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

			int longest = -1;

			float longestLen = 0.0f;

			FVector curr;
			for (int i = 1; i < points.Num(); i++) {
				float dist = FVector::DistSquared(points[i], points[i - 1]);
				if (dist > longestLen) {
					longestLen = dist;
					longest = i;
				}
			}
			if (longest == -1) {
				return SplitStruct{ 0, 0, FVector(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 0.0f) };
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
	living UMETA(DisplayName = "Living Room"),
	closet UMETA(DisplayName = "Closet"),
	hallway UMETA(DisplayName = "Hallway")

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

//static getOfficeSpecifications




struct FRoomPolygon : public FPolygon
{
	TSet<int32> windows;
	TSet<int32> entrances;
	TSet<int32> nonDuplicatingEntrances;
	TSet<int32> toIgnore;
	TMap<int32, TSet<FRoomPolygon*>> passiveConnections;
	TMap<FRoomPolygon*, int32> activeConnections;

	bool canRefine = true;
	SubRoomType type = SubRoomType::empty;

	void updatePassiveConnections(SplitStruct &p, bool first, FRoomPolygon* newP) {
		TArray<FRoomPolygon*> toRemove;

		TSet<FRoomPolygon*> childPassive;
		for (FRoomPolygon* p1 : passiveConnections[first ? p.min : p.max]) {
			int loc = p1->activeConnections[this];
			// this is the entrance, we're not allowed to place a wall that collides with this, try to fit newP on the other side of the entrance if possible, making it a single-entry room
			FVector point = middle(p1->points[loc], p1->points[loc - 1]);
			float dist = FVector::Dist(point, first ? p.p1 : p.p2);
			if (dist < 100) {
				FVector tangent1 = points[p.min] - points[p.min - 1];
				tangent1.Normalize();
				FVector tangent2 = points[p.max] - points[p.max - 1];
				tangent2.Normalize();
				float toMove = FVector::Dist(first ? p.p1 : p.p2, point - (first ? tangent1 : tangent2)* 100);
				p.p1 -= toMove * tangent1;
				p.p2 -= toMove * tangent2;
				// our child is now free from our master, while we're still slaves
			}
			else {
				// new cut doesn't interfere overlap with the entrance, now to see who gets it
				if (first && isOnLine(point, points[p.min - 1], p.p1) || !first && !isOnLine(point, points[p.max-1], p.p2)) {
					// i got it, no change
				}
				else {
					// my child got it
					childPassive.Add(p1);
					p1->activeConnections.Remove(this);
					p1->activeConnections.Add(newP, loc);
					toRemove.Add(p1);
				}
			}
		}
		if (childPassive.Num() > 0)
			newP->passiveConnections.Add(newP->points.Num(), childPassive);

		for (FRoomPolygon* t : toRemove) {
			passiveConnections[first ? p.min : p.max].Remove(t);
		}
	}


	FRoomPolygon* splitAlongMax(float approxRatio, bool entranceBetween) {
		SplitStruct p = getSplitProposal(false, approxRatio);
		if (p.p1.X == 0.0f) {
			return NULL;
		}
		FRoomPolygon* newP = new FRoomPolygon();
		newP->points.Add(p.p1);


		if (entrances.Contains(p.min) && !nonDuplicatingEntrances.Contains(p.min)) {
			// potentially add responsibility of child 
			//newP.entrances.Add(newP.points.Num());
		}
		if (windows.Contains(p.min)) {
			newP->windows.Add(newP->points.Num());
		}
		if (toIgnore.Contains(p.min)) {
			if (passiveConnections.Contains(p.min)) {
				updatePassiveConnections(p, true, newP);
			}
			newP->toIgnore.Add(newP->points.Num());
		}
		newP->points.Add(points[p.min]);

		for (int i = p.min + 1; i < p.max; i++) {
			if (entrances.Contains(i)) {
				entrances.Remove(i);
				if (nonDuplicatingEntrances.Contains(i)) {
					newP->nonDuplicatingEntrances.Add(newP->points.Num());
				}
				newP->entrances.Add(newP->points.Num());

				auto res = activeConnections.FindKey(i);
				if (res != nullptr) {
					FRoomPolygon *p1 = *res;
					for (int j = 1; j < p1->points.Num(); j++) {
						if (p1->passiveConnections.Contains(j) && p1->passiveConnections[j].Contains(this)) {
							p1->passiveConnections[j].Remove(this);
							p1->passiveConnections[j].Add(newP);
							newP->activeConnections.Add(newP, j);
							break;
						}
					}
				}

			}
			if (windows.Contains(i)) {
				windows.Remove(i);
				newP->windows.Add(newP->points.Num());
			}
			if (toIgnore.Contains(i)) {
				toIgnore.Remove(i);
				newP->toIgnore.Add(newP->points.Num());
				if (passiveConnections.Contains(i)) {
					TSet<FRoomPolygon*> pols = passiveConnections[i];
					newP->passiveConnections.Add(newP->points.Num(), pols);
					for (FRoomPolygon *p1 : pols) {
						p1->activeConnections.Remove(this);
						p1->activeConnections.Add(newP, newP->points.Num());
					}
					passiveConnections.Remove(i);
				}
			}
			newP->points.Add(points[i]);
		}

		//TMap<int32, TArray<FRoomPolygon*>> passiveConnections;
		//TMap<FRoomPolygon*, int32> activeConnections;

		if (entrances.Contains(p.max) && !nonDuplicatingEntrances.Contains(p.max)) {
			//newP.entrances.Add(newP.points.Num());
		}
		if (windows.Contains(p.max)) {
			newP->windows.Add(newP->points.Num());
		}
		if (toIgnore.Contains(p.max)) {
			newP->toIgnore.Add(newP->points.Num());
			if (passiveConnections.Contains(p.max)) {
				updatePassiveConnections(p, false, newP);
			}
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


		//toRemove.clear();
		//TSet<int32> newList;
		//for (int32 i : toIgnore) {
		//	if (i >= p.max)
		//		newList.Add(i - (p.max - p.min) + 2);
		//	else
		//		newList.Add(i);
		//}
		//toIgnore = newList;
		for (int32 i : toIgnore) {
			if (i >= p.max)
				toRemove.push_back(i);
		}
		for (int32 i : toRemove) {
			toIgnore.Remove(i);
			toIgnore.Add(i - (p.max - p.min) + 2);
		}

		TMap<FRoomPolygon*, int32> newActive;
		for (auto &pair : activeConnections) {
			if (pair.Value > p.min) {
				newActive.Add(pair.Key, pair.Value - (p.max - p.min) + 2);
			}
			else {
				newActive.Add(pair.Key, pair.Value);
			}
		}
		activeConnections = newActive;
			//for (int i = 0; i < pair.Key->points.Num(); i++) {

				//TSet<FRoomPolygon*> pols = pair.Key->passiveConnections[i];
				//if (pair.Key->passiveConnections.Contains(i) && pair.Key->passiveConnections[i].Contains(this)) {
				//	pair.Key->passiveConnections[i].Remove(this);
				//}
			//}	

		newP->points.Add(p.p2);

		// dont place the wall twice
		toIgnore.Add(p.min+1);
		//	// entrance to next room
		if (entranceBetween && ((!entrances.Contains(p.min) && !entrances.Contains(p.min+2) || nonDuplicatingEntrances.Contains(p.min) || nonDuplicatingEntrances.Contains(p.min+2)))) {
			TSet<FRoomPolygon*> passive;
			passive.Add(newP);
			passiveConnections.Add(p.min + 1, passive);
			newP->entrances.Add(newP->points.Num());
			newP->activeConnections.Add(this, newP->points.Num());
		}
		//}

		newP->points.Add(p.p1);


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




	TArray<FRoomPolygon*> fitSpecificationOnRooms(TArray<RoomSpecification> specs, TArray<FRoomPolygon*> &remaining, bool repeating) {
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
				for (int i = 0; i < remaining.Num(); i++) {
					FRoomPolygon *p = remaining[i];
					area = p->getArea();
					smaller = smaller || (area > r.maxArea);
					if (area <= r.maxArea && area >= r.minArea && p->type != SubRoomType::hallway) {
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
					//remaining.RemoveAt(targetNum);
					int count = 0;
					while (scale < minPctSplit && count++ < 5) {
						FRoomPolygon* newP = target->splitAlongMax(0.6, true);
						if (target->nonDuplicatingEntrances.Num() > 0) {
							FRoomPolygon *temp = target;
							target = newP;
							newP = target;
						}
						if (target->nonDuplicatingEntrances.Num() > 0) {
							target->type = SubRoomType::hallway;
						}
						if (newP->nonDuplicatingEntrances.Num() > 0) {
							newP->type = SubRoomType::hallway;
						}
						remaining.EmplaceAt(0, newP);
						scale = r.minArea / target->getArea();

					}
					if (target->getArea() <= r.maxArea && target->getArea() >= r.minArea && target->type != SubRoomType::hallway) {
						target->type = r.type;
						remaining.RemoveAt(targetNum);
						toReturn.Add(target);
					}
					else {
						float ideal = (r.maxArea - r.minArea) / 2 + r.minArea;
						FRoomPolygon* newP = target->splitAlongMax(r.minArea / target->getArea(), true);
						if (newP->nonDuplicatingEntrances.Num() > 0) {
							newP->type = SubRoomType::hallway;
							FRoomPolygon *temp = target;
							target = newP;
							newP = target;

						}
						newP->type = r.type;
						toReturn.Add(newP);
						//remaining.EmplaceAt(0, target);

					}
					couldPlace = true;
						
				}
			}
		} while (repeating && couldPlace && c1++ < 5);

		return toReturn;


	}

	// post placement part of algorithm
	void postFit(TArray<FRoomPolygon> &rooms) {
		for (FRoomPolygon &p : rooms) {
			if (!splitableType(p.type) && p.entrances.Num() + p.toIgnore.Num() > 1) {
				p.type = SubRoomType::corridor;
			}
		}
	}

	TArray<FRoomPolygon> getRooms(RoomBlueprint blueprint) {
		TArray<FRoomPolygon*> rooms;
		TArray<FRoomPolygon*> remaining;
		FRoomPolygon* thisP = new FRoomPolygon();
		*thisP = *this;
		thisP->type = SubRoomType::hallway;
		remaining.Add(thisP);
		//removeAllButOne(remaining[0].entrances);

		rooms.Append(fitSpecificationOnRooms(blueprint.needed, remaining, false));
		rooms.Append(fitSpecificationOnRooms(blueprint.optional, remaining, true));

		//postFit(rooms);
		for (FRoomPolygon *p : remaining) {
			//for (int i = 0; i < p.points.Num(); i++) {
			//	p.toIgnore.Add(i);
			//}
			rooms.Add(p);
		}
		TArray<FRoomPolygon> toReturn;
		for (FRoomPolygon *p : rooms) {
			toReturn.Add(*p);
		}
		for (FRoomPolygon *p : rooms) {
			//	delete p;
		}


		return toReturn;
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