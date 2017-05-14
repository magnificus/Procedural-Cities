// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "HouseBuilder.h"

struct FPolygon;


// Sets default values
AHouseBuilder::AHouseBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}





struct twoInt {
	int32 a;
	int32 b;
};

// dont even ask
bool increasing(TArray<twoInt> ints) {
	for (twoInt t : ints) {
		if (t.b == t.a + 1) {
			return true;
		}
	}
	return false;
}

// this function returns all of the first grade Room Polygons, these can later be expanded upon when finalizing the room in RoomBuilder
TArray<FRoomPolygon> getInteriorPlan(FHousePolygon &f, FPolygon hole, bool ground, float corrWidth){
	TArray<FLine> lines;
	
	TArray<FRoomPolygon> roomPols;

	FRoomPolygon corners;
	corners.canRefine = false;

	// add corridors from the hole
	TArray<twoInt> connections;
	for (int i = 0; i < hole.points.Num() - 1; i++) {
		roomPols.Add(FRoomPolygon{});
		connections.Add(twoInt{});
	}
	for (int i = 1; i < hole.points.Num(); i++) {
		FVector tangent = hole.points[i] - hole.points[i - 1];
		float midPos = tangent.Size() / 2;
		tangent.Normalize();
		FVector altTangent = FRotator(0, 270, 0).RotateVector(tangent);
		FVector firstAttach = hole.points[i- 1] + (midPos - corrWidth * 0.5) * tangent;
		FVector sndAttach = FVector(0.0f, 0.0f, 0.0f);
		int conn = 0;
		for (int j = 1; j < f.points.Num(); j++) {
			FVector res = intersection(firstAttach, firstAttach + altTangent * 100000, f.points[j - 1], f.points[j]);
			if (res.X != 0.0f) {
				sndAttach = res;
				conn = j;
				break;
			}

		}
		if (sndAttach.X == 0.0f)
			return TArray<FRoomPolygon>();
		if (!ground || !f.entrances.Contains(conn))
			corners.points.Add(sndAttach);

		connections[i - 1].b = conn;
		roomPols[i - 1].points.Add(hole.points[i-1]);
		roomPols[i - 1].points.Add(firstAttach);
		roomPols[i - 1].entrances.Add(roomPols[i - 1].points.Num());
		roomPols[i - 1].points.Add(sndAttach);



		firstAttach = hole.points[i-1] + (midPos + corrWidth * 0.5) * tangent;
		sndAttach = FVector(0.0f, 0.0f, 0.0f);
		for (int j = 1; j < f.points.Num(); j++) {
			FVector res = intersection(firstAttach, firstAttach + altTangent * 100000, f.points[j - 1], f.points[j]);
			if (res.X != 0.0f) {
				sndAttach = res;
				conn = j;
				break;
			}
		}
		if (!ground || !f.entrances.Contains(conn))
			corners.points.Add(sndAttach);

		if (i == (hole.points.Num() - 1)) {

			TArray<int32> toRemove;
			for (int32 j : roomPols[0].entrances) {
				toRemove.Add(j);
			}
			for (int32 j : toRemove) {
				roomPols[0].entrances.Remove(j);
				roomPols[0].entrances.Add(j + 3);
			}
			connections[0].a = conn;
			roomPols[0].points.EmplaceAt(0, sndAttach);
			//roomPols[0].entrances.Add(1);
			roomPols[0].points.EmplaceAt(1, firstAttach);
			roomPols[0].points.EmplaceAt(2, hole.points[i]);
		}
		else {
			connections[i].a = conn;
			roomPols[i].points.Add(sndAttach);
			//roomPols[i].entrances.Add(roomPols[i].points.Num());
			roomPols[i].points.Add(firstAttach);
			roomPols[i].points.Add(hole.points[i]);
		}


	}

	// sew the roomPolygons together by adding walls

	bool isIncreasing = increasing(connections);
	for (int i = 0; i < roomPols.Num(); i++) {
		FRoomPolygon &fp = roomPols[i];
		//UE_LOG(LogTemp, Warning, TEXT("a %i,b %i"), connections[i].a, connections[i].b);

		if (isIncreasing) {
			for (int j = connections[i].b - 1 == -1 ? f.points.Num() - 1 : connections[i].b - 1; j != std::abs((connections[i].a - 1) % f.points.Num()); j = j == 0 ? f.points.Num() - 1 : j - 1) {
				if (f.windows.Contains(j+1)) {
					fp.windows.Add(fp.points.Num());
				}
				fp.points.Add(f.points[j]);


			}
		}
		else {
			for (int j = (connections[i].b) % f.points.Num(); j != (connections[i].a) % f.points.Num(); j++, j %= f.points.Num()) {
				if (f.windows.Contains(j)) {
					fp.windows.Add(fp.points.Num());
				}
				fp.points.Add(f.points[j]);

			}
		}
		if (f.windows.Contains(connections[i].a))
			roomPols[i].windows.Add(roomPols[i].points.Num());

		FVector first = roomPols[i].points[0];
		roomPols[i].points.Add(first);
	}
	for (FRoomPolygon p : roomPols) {
		if (FVector::Dist(p.points[0], p.points[p.points.Num() - 1]) > 0.1f) {
			UE_LOG(LogTemp, Warning, TEXT("SPOOK ALERT STOP THE PRESSES"));
		}
	}

	for (int i = 2; i < corners.points.Num(); i+=2) {
		corners.toIgnore.Add(i);
	}
	corners.reverse();
	roomPols.Add(corners);

	return roomPols;
}



// ugly method for finding direction of polygon
bool increasing(std::vector<int> nbrs) {
	for (int i = 1; i < nbrs.size(); i++) {
		if (nbrs[i] == nbrs[i - 1] + 1) {
			return true;
		}
	}
	return false;
}

// this returns polygons corresponding to a hole with the shape "hole" in the polygon f
TArray<FMaterialPolygon> getFloorPolygonsWithHole(FHousePolygon f, float floorBegin, FPolygon hole) {
	hole.offset(FVector(0, 0, floorBegin));
	f.offset(FVector(0, 0, floorBegin));

	TArray<FMaterialPolygon> polygons;
	std::vector<int> connections;
	for (int i = 1; i < f.points.Num(); i++) {
		FVector currMid = (f.points[i] - f.points[i - 1]) / 2 + f.points[i - 1];
		int closest = 0;
		float closestDist = 10000000;
		for (int j = 0; j < hole.points.Num(); j++) {
			FVector f2 = hole.points[j];
			float currDist = FVector::Dist(f2, currMid);
			if (currDist < closestDist) {
				closestDist = currDist;
				closest = j;
			}
		}
		connections.push_back(closest);
		FMaterialPolygon newP;
		newP.points.Add(f.points[i - 1]);
		newP.points.Add(hole.points[closest]);
		newP.points.Add(f.points[i]);
		newP.type = PolygonType::floor;
		polygons.Add(newP);
	}
	int change = increasing(connections) ? 1 : -1;
	int prev = connections[0];
	for (int i = 1; i < connections.size(); i++) {
		while (connections[i] != prev) {
			FMaterialPolygon newP;
			int next = prev + change;
			next %= hole.points.Num();
			if (next < 0) {
				next += hole.points.Num();
			}
			newP.points.Add(f.points[i]);
			newP.points.Add(hole.points[prev]);
			newP.points.Add(hole.points[next]);

			newP.type = PolygonType::floor;
			polygons.Add(newP);
			prev = next;
		}

	}
	prev = connections[connections.size() - 1];
	while (prev != connections[0]) {
		FMaterialPolygon newP;
		int next = prev + change;
		next %= hole.points.Num();
		if (next < 0) {
			next += hole.points.Num();
		}

		newP.points.Add(hole.points[next]);
		newP.points.Add(f.points[0]);
		newP.points.Add(hole.points[prev]);
		newP.type = PolygonType::floor;
		polygons.Add(newP);
		prev = next;
	}

	return polygons;


}


FPolygon getShaftHolePolygon(FHousePolygon f) {
	FPolygon hole;
	FVector center = f.getCenter();

	float holeSizeX = 1000;
	float holeSizeY = 2000;

	FVector tangent1 = f.points[1] - f.points[0];
	tangent1.Normalize();
	FVector tangent2 = FRotator(0, 90, 0).RotateVector(tangent1);


	hole.points.Add(center + 0.5 * tangent1 * holeSizeX + 0.5 * tangent2 * holeSizeY);
	hole.points.Add(center - 0.5 * tangent1 * holeSizeX + 0.5 * tangent2 * holeSizeY);
	hole.points.Add(center - 0.5 * tangent1 * holeSizeX - 0.5 * tangent2 * holeSizeY);
	hole.points.Add(center + 0.5 * tangent1 * holeSizeX - 0.5 * tangent2 * holeSizeY);
	hole.points.Add(center + 0.5 * tangent1 * holeSizeX + 0.5 * tangent2 * holeSizeY);

	return hole;
}

// changes the shape of the house
void makeInteresting(FHousePolygon &f) {
	if (randFloat() < 0.1f) {
		f.removePoint((randFloat() * f.points.Num() - 1) + 1);
		//f.points.RemoveAt((randFloat() * f.points.Num() - 1) + 1);
	}
	if (randFloat() < 0.1f) {
		float len = 5000;
		// turn a side inwards into a U
		int place = (randFloat() * (f.points.Num() - 2)) + 2;
		FVector tangent = f.points[place] - f.points[place - 1];
		float lenSide = tangent.Size();
		tangent.Normalize();
		FVector dir = FRotator(0, f.buildLeft ? 90 : 270, 0).RotateVector(tangent);
		FVector first = f.points[place - 1] + tangent * lenSide / 3;
		FVector first2 = first + dir * len;
		FVector snd = f.points[place - 1] + tangent * lenSide / (3/2) ;
		FVector snd2 = snd + dir * len;
		f.addPoint(place, first);
		f.addPoint(place + 1, first2);
		f.addPoint(place + 2, snd2);
		f.addPoint(place + 3, snd);

	}
	if (randFloat() < 0.1f) {
		float len = 500;
		// make sides more irregular by moving parts of it inwards (two points)
		int place = (randFloat() * (f.points.Num() - 2)) + 2;
		FVector tangent = f.points[place] - f.points[place - 1];
		tangent.Normalize();
		FVector dir = FRotator(0, f.buildLeft ? 90 : 270, 0).RotateVector(tangent);
		FVector first = f.points[place - 1] + dir * len;
		FVector snd = f.points[place] + dir * len;
		f.removePoint(place - 1);
		f.removePoint(place - 1);
		f.addPoint(place - 1, first);
		f.addPoint(place, snd);

	}
}


TArray<FPolygon> getShaftSides(FPolygon hole, int openSide, float height) {
	TArray<FPolygon> sides;
	for (int i = 1; i < hole.points.Num(); i++) {
		if (i == openSide)
			continue;
		FPolygon side;
		side.points.Add(hole.points[i - 1]);
		side.points.Add(hole.points[i - 1] + FVector(0, 0, height));
		side.points.Add(hole.points[i] + FVector(0, 0, height));
		side.points.Add(hole.points[i]);
		side.points.Add(hole.points[i - 1]);
		sides.Add(side);

	}
	return sides;
}


void addStairInfo(FRoomInfo &info, float height, FPolygon &hole) {
	float distFromCorners = 300;
	TArray<FMaterialPolygon> sides;
	for (int i = 1; i < hole.points.Num(); i++) {
		FMaterialPolygon side;
		side.points.Add(hole.points[i - 1]);
		side.points.Add(hole.points[i - 1] + FVector(0, 0, height));
		side.points.Add(hole.points[i] + FVector(0, 0, height));
		side.points.Add(hole.points[i]);
		side.points.Add(hole.points[i - 1]);
		sides.Add(side);
	}
	info.pols.Append(sides);

}

void buildRoof(FRoomInfo &info, FPolygon pol) {
	if (randFloat() < 0.5f) {
		// add angled roof
		float height = 1000;
		FVector center = pol.getCenter();
		center += FVector(0, 0, height);
		for (int i = 1; i < pol.points.Num(); i++) {
			FMaterialPolygon newP;
			newP.points.Add(pol.points[i - 1]);
			newP.points.Add(pol.points[i]);
			newP.points.Add(center);
			newP.points.Add(pol.points[i - 1]);
			newP.type = PolygonType::roof;
			info.pols.Add(newP);
		}
	}
}

FRoomInfo AHouseBuilder::getHouseInfo(FHousePolygon f, int floors, float floorHeight)
{
	if (f.points.Num() < 3) {
		return FRoomInfo();
	}
	//makeInteresting(f);
	FRoomInfo toReturn;




	float wDens = randFloat() * 0.0010 + 0.0005;

	float wWidth = randFloat() * 500 + 500;
	float wHeight = randFloat() * 400 + 200;


	FPolygon hole = getShaftHolePolygon(f);
	TArray<FRoomPolygon> roomPols = getInteriorPlan(f, hole, true, 300);

	
	for (FRoomPolygon p : roomPols) {
		FRoomInfo newR = ARoomBuilder::buildRoom(p, f.type, 0, floorHeight, 0.005, 250, 150);
		newR.offset(FVector(0, 0, 30));
		toReturn.pols.Append(newR.pols);
		toReturn.meshes.Append(newR.meshes);
	}
	//for (int i = 1; i < floors; i++) {
	//	toReturn.pols.Append(getFloorPolygonsWithHole(f, floorHeight*i, hole));

	//	roomPols = getInteriorPlan(f, hole, false, 300);
	//	for (FRoomPolygon &p : roomPols) {
	//		//p.offset(FVector(0, 0, floorHeight*i));
	//		FRoomInfo newR = ARoomBuilder::buildRoom(p, f.type, 1, floorHeight, 0.005, 250, 150);
	//		newR.offset(FVector(0, 0, floorHeight*i));
	//		toReturn.pols.Append(newR.pols);
	//		toReturn.meshes.Append(newR.meshes);
	//	}
	//}


	FMaterialPolygon roof;
	roof.points = f.points;
	roof.offset(FVector(0, 0, floorHeight*floors));
	roof.type = PolygonType::roof;
	//toReturn.pols.Add(roof);

	//buildRoof(toReturn, roof);
	FMaterialPolygon floor;
	floor.points = f.points;
	floor.offset(FVector(0, 0, 30));
	floor.type = PolygonType::floor;
	toReturn.pols.Add(floor);

	TArray<FMaterialPolygon> otherSides;
	for (FMaterialPolygon p : toReturn.pols) {
		FMaterialPolygon other = p;
		//other.reverse();
		p.offset(p.getDirection() * 20);
		otherSides.Add(p);
		for (int i = 1; i < p.points.Num(); i++) {
			FMaterialPolygon newP1;
			newP1.type = p.type;
			newP1.points.Add(p.points[i - 1]);
			newP1.points.Add(p.points[i]);
			newP1.points.Add(other.points[i - 1]);
			otherSides.Add(newP1);

			FMaterialPolygon newP2;
			newP2.type = p.type;
			newP2.points.Add(other.points[i - 1]);
			newP2.points.Add(other.points[i]);
			newP2.points.Add(p.points[i]);
			otherSides.Add(newP2);

		}
	}

	toReturn.pols.Append(otherSides);
	return toReturn;



}

// Called when the game starts or when spawned
void AHouseBuilder::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHouseBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

