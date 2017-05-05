// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "HouseBuilder.h"

struct FPolygon;


// Sets default values
AHouseBuilder::AHouseBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

/*
holes are assumed to be of a certain structure, just four points upper left -> lower left -> lower right -> upper right, no window is allowed to overlap another
0---3
|	|
1---2
*/
TArray<FPolygon> getSideWithHoles(FPolygon outer, TArray<FPolygon> holes) {

	TArray<FPolygon> polygons;
	FVector start = outer.points[1];
	FVector end = outer.points[2];

	// sort holes according to dist to starting point so that they can be applied in order
	holes.Sort([start](const FPolygon &p1, const FPolygon &p2) {
		return FVector::DistSquared(p1.points[1], start) < FVector::DistSquared(p2.points[1], start);
	});
	if (holes.Num() > 0) {

		FVector sideTangent = outer.points[1] - outer.points[0];

		FVector attach1 = outer.points[0];
		FVector attach2 = (holes[0].points[0] - outer.points[0]).ProjectOnTo(sideTangent) + outer.points[0];
		FVector attach3 = (holes[0].points[1] - outer.points[0]).ProjectOnTo(sideTangent) + outer.points[0];
		FVector attach4 = outer.points[1];

		FVector tangentUp = outer.points[3] - outer.points[0];
		FVector tangentDown = outer.points[2] - outer.points[1];
		//tangentUp.Normalize();
		//tangentDown.Normalize();

		int count = 0;
		for (FPolygon p : holes) {
			//UE_LOG(LogTemp, Warning, TEXT("attach1 %s, %i"), *attach1.ToString(), count++);
			FPolygon p1 = FPolygon();
			p1.points.Add(attach1);
			p1.points.Add(p.points[3]);

			p1.points.Add((p.points[3] - outer.points[0]).ProjectOnTo(tangentUp) + outer.points[0]);

			FPolygon p2 = FPolygon();
			p2.points.Add(attach1);
			p2.points.Add(attach2);
			p2.points.Add(p.points[3]);

			FPolygon p3 = FPolygon();
			p3.points.Add(attach2);
			p3.points.Add(p.points[1]);
			p3.points.Add(p.points[0]);

			FPolygon p4 = FPolygon();
			p4.points.Add(attach2);
			p4.points.Add(attach3);
			p4.points.Add(p.points[1]);

			FPolygon p5 = FPolygon();
			p5.points.Add(attach3);
			p5.points.Add((p.points[2] - outer.points[1]).ProjectOnTo(tangentDown) + outer.points[1]);
			p5.points.Add(p.points[2]);

			FPolygon p6 = FPolygon();
			p6.points.Add(attach3);
			p6.points.Add(attach4);
			p6.points.Add((p.points[2] - outer.points[1]).ProjectOnTo(tangentDown) + outer.points[1]);

			polygons.Add(p1);
			polygons.Add(p2);
			polygons.Add(p3);
			polygons.Add(p4);
			polygons.Add(p5);
			polygons.Add(p6);

			attach1 = p1.points[2];
			attach2 = p1.points[1];
			attach3 = p5.points[2];
			attach4 = p5.points[1];
		}
		// attach to end of outer
		//UE_LOG(LogTemp, Warning, TEXT("attach1 %s, %i"), *attach1.ToString(), count++);

		FPolygon p7;
		p7.points.Add(attach1);
		p7.points.Add(attach4);
		p7.points.Add(outer.points[2]);

		FPolygon p8;
		p8.points.Add(attach1);
		p8.points.Add(outer.points[2]);
		p8.points.Add(outer.points[3]);

		polygons.Add(p7);
		polygons.Add(p8);
	}
	else {
		polygons.Add(outer);
	}



	return polygons;
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
			roomPols[0].entrances.Add(1);
			roomPols[0].points.EmplaceAt(1, firstAttach);
			roomPols[0].points.EmplaceAt(2, hole.points[i]);
		}
		else {
			connections[i].a = conn;
			roomPols[i].points.Add(sndAttach);
			roomPols[i].entrances.Add(roomPols[i].points.Num());
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
	roomPols.Add(corners);

	return roomPols;
}


TArray<FPolygon> getEntranceSide(FVector p1, FVector p2, float floorHeight, float doorHeight, float doorWidth) {
	TArray<FPolygon> polygons;


	FVector side = p2 - p1;
	float sideLen = side.Size();
	side.Normalize();
	float distToDoor = (sideLen - doorWidth) / 2;
	TArray<FPolygon> holes;
	FPolygon doorPolygon;
	doorPolygon.points.Add(p1 + side*distToDoor + FVector(0, 0, doorHeight));
	doorPolygon.points.Add(p1 + side*distToDoor);// + FVector(0, 0, 100));
	doorPolygon.points.Add(p2 - side*distToDoor);// + FVector(0, 0, 100));
	doorPolygon.points.Add(p2 - side*distToDoor + FVector(0, 0, doorHeight));
	holes.Add(doorPolygon);

	FPolygon outer;
	outer.points.Add(p1 + FVector(0, 0, floorHeight));
	outer.points.Add(p1 + FVector(0, 0, 0));
	outer.points.Add(p2 + FVector(0, 0, 0));
	outer.points.Add(p2 + FVector(0, 0, floorHeight));
	polygons.Append(getSideWithHoles(outer, holes));

	return polygons;
}

// just the sides of the house with a hole for a single door
TArray<FPolygon> getGroundPolygons(FHousePolygon f, float floorHeight, float doorHeight, float doorWidth) {

	TArray<FPolygon> polygons;

	for (int i = 1; i < f.points.Num(); i++) {
		if (f.entrances.Contains(i)) {
			FVector side = f.points[i] - f.points[i - 1];
			float sideLen = side.Size();
			side.Normalize();
			float distToDoor = (sideLen - doorWidth) / 2;
			TArray<FPolygon> holes;
			FPolygon doorPolygon;
			doorPolygon.points.Add(f.points[i - 1] + side*distToDoor + FVector(0, 0, doorHeight));
			doorPolygon.points.Add(f.points[i - 1] + side*distToDoor);// + FVector(0, 0, 100));
			doorPolygon.points.Add(f.points[i] - side*distToDoor);// + FVector(0, 0, 100));
			doorPolygon.points.Add(f.points[i] - side*distToDoor + FVector(0, 0, doorHeight));
			holes.Add(doorPolygon);

			FPolygon outer;
			outer.points.Add(f.points[i - 1] + FVector(0, 0, floorHeight));
			outer.points.Add(f.points[i - 1] + FVector(0, 0, 0));
			outer.points.Add(f.points[i] + FVector(0, 0, 0));
			outer.points.Add(f.points[i] + FVector(0, 0, floorHeight));
			polygons.Append(getSideWithHoles(outer, holes));
		}
		else {
			FPolygon newP;
			newP.points.Add(f.points[i - 1] + FVector(0, 0, floorHeight));
			newP.points.Add(f.points[i - 1]);
			newP.points.Add(f.points[i]);
			newP.points.Add(f.points[i] + FVector(0, 0, floorHeight));
			polygons.Add(newP);
		}


	}

	return polygons;
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
TArray<FPolygon> getFloorPolygonsWithHole(FHousePolygon f, float floorBegin, FPolygon hole) {
	hole.offset(FVector(0, 0, floorBegin));
	f.offset(FVector(0, 0, floorBegin));

	TArray<FPolygon> polygons;
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
		FPolygon newP;
		newP.points.Add(f.points[i - 1]);
		newP.points.Add(hole.points[closest]);
		newP.points.Add(f.points[i]);
		polygons.Add(newP);
	}
	int change = increasing(connections) ? 1 : -1;
	int prev = connections[0];
	for (int i = 1; i < connections.size(); i++) {
		while (connections[i] != prev) {
			FPolygon newP;
			int next = prev + change;
			next %= hole.points.Num();
			if (next < 0) {
				next += hole.points.Num();
			}
			newP.points.Add(hole.points[prev]);
			newP.points.Add(f.points[i]);
			newP.points.Add(hole.points[next]);
			polygons.Add(newP);
			prev = next;
		}

	}
	prev = connections[connections.size() - 1];
	while (prev != connections[0]) {
		FPolygon newP;
		int next = prev + change;
		next %= hole.points.Num();
		if (next < 0) {
			next += hole.points.Num();
		}

		newP.points.Add(hole.points[prev]);
		newP.points.Add(f.points[0]);
		newP.points.Add(hole.points[next]);
		polygons.Add(newP);
		prev = next;
	}

	return polygons;


}

TArray<FPolygon> getFloorPolygons(FHousePolygon &f, float floorBegin, float floorHeight, FPolygon hole) {
	TArray<FPolygon> polygons;


	polygons.Append(getFloorPolygonsWithHole(f, floorBegin, hole));
	for (int i = 1; i < f.points.Num(); i++) {
		FPolygon outer;
		outer.points.Add(f.points[i - 1] + FVector(0, 0, floorHeight + floorBegin));
		outer.points.Add(f.points[i - 1] + FVector(0, 0, floorBegin));
		outer.points.Add(f.points[i] + FVector(0, 0, floorBegin));
		outer.points.Add(f.points[i] + FVector(0, 0, floorHeight + floorBegin));


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



TArray<FPolygon> interiorPlanToPolygons(TArray<FRoomPolygon> roomPols, float currentHeight, float floorHeight, float windowDensity, float windowHeight, float windowWidth) {
	TArray<FPolygon> toReturn;

	for (FRoomPolygon rp : roomPols) {
		for (int i = 1; i < rp.points.Num(); i++) {
			if (rp.toIgnore.Contains(i)) {
				continue;
			}
			FPolygon newP;
			FVector p1 = rp.points[i - 1];
			FVector p2 = rp.points[i];
			newP.points.Add(p1 + FVector(0, 0, currentHeight + floorHeight));
			newP.points.Add(p1 + FVector(0, 0, currentHeight));
			newP.points.Add(p2 + FVector(0, 0, currentHeight));
			newP.points.Add(p2 + FVector(0, 0, currentHeight + floorHeight));
			newP.points.Add(p1 + FVector(0, 0, currentHeight + floorHeight));

			if (rp.entrances.Contains(i)) {
				toReturn.Append(getEntranceSide(rp.points[i - 1] + FVector(0, 0, currentHeight), rp.points[i] + FVector(0, 0, currentHeight), floorHeight , 300, 180));
			}
			else if (rp.windows.Contains(i)) {

				FVector tangent = rp.points[i] - rp.points[i - 1];
				float len = tangent.Size();
				tangent.Normalize();

				TArray<FPolygon> windows;
				int spaces = FMath::FloorToInt(windowDensity * len) + 1;
				float jumpLen = len / (float) spaces;

				for (int j = 1; j < spaces; j++) {
					FPolygon currWindow;
					FVector pw1 = rp.points[i - 1] + tangent * j * jumpLen + FVector(0, 0, currentHeight + 50 + windowHeight) - (tangent * windowWidth / 2);
					FVector pw2 = pw1 - FVector(0, 0, windowHeight);
					FVector pw3 = pw2 + tangent * windowWidth;
					FVector pw4 = pw3 + FVector(0, 0, windowHeight);

					currWindow.points.Add(pw1);
					currWindow.points.Add(pw2);
					currWindow.points.Add(pw3);
					currWindow.points.Add(pw4);

					windows.Add(currWindow);

				}
				TArray<FPolygon> pols = getSideWithHoles(newP, windows);
				toReturn.Append(pols);

			}
			else {
				toReturn.Add(newP);
			}

		}

	}
	return toReturn;

}

FHouseInfo AHouseBuilder::getHouseInfo(FHousePolygon f, int floors, float floorHeight)
{
	if (f.points.Num() < 3) {
		return FHouseInfo();
	}
	makeInteresting(f);
	FHouseInfo toReturn;




	float wDens = randFloat() * 0.0010 + 0.0005;

	float wWidth = randFloat() * 500 + 500;
	float wHeight = randFloat() * 400 + 200;


	FPolygon hole = getShaftHolePolygon(f);

	//toReturn.Append(getShaftSides(hole, 1, floorHeight * floors));
	//FPolygon lessHole = 

	TArray<FRoomPolygon> roomPols = getInteriorPlan(f, hole, true, 300);
	TArray<RoomInfo> roomInfos;

	
	for (FRoomPolygon p : roomPols) {
		roomInfos.Add(ARoomBuilder::buildRoom(p, RoomType::office, 0, 0, floorHeight));
	}
	for (int i = 1; i < floors; i++) {
		toReturn.polygons.Append(getFloorPolygonsWithHole(f, floorHeight*i, hole));

		//refinedRooms.Empty();
		roomPols = getInteriorPlan(f, hole, false, 300);
		for (FRoomPolygon p : roomPols) {
			roomInfos.Add(ARoomBuilder::buildRoom(p, RoomType::office, 1, floorHeight*i, floorHeight));
		}
	}
	for (RoomInfo r : roomInfos) {
		toReturn.polygons.Append(interiorPlanToPolygons(r.rooms, r.beginning, r.height, 0.004, 400, 200));
		toReturn.meshes.Append(r.meshes);
	}


	FPolygon roof = f;
	roof.offset(FVector(0, 0, floorHeight*floors));
	toReturn.polygons.Add(roof);
	FPolygon floor = f;
	floor.offset(FVector(0, 0, 10));
	toReturn.polygons.Add(floor);
	return toReturn;
	// we have the outline of the house, have to place levels, start with bottom & door



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

