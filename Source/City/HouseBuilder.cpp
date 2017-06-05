// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "simplexnoise.h"
#include "polypartition.h"
#include "HouseBuilder.h"

struct FPolygon;


// Sets default values
AHouseBuilder::AHouseBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//mapStatic = map;
}

struct twoInt {
	int32 a;
	int32 b;
};




// VERY crudely checks which direction an array is going, it can wrap around 0 unfortunately
bool increasing(std::vector<int> nbrs) {
	for (int i = 1; i < nbrs.size(); i++) {
		if (nbrs[i] == nbrs[i - 1] + 1) {
			return true;
		}
	}
	return false;
}

// similar to above but for a different structure
bool increasing(TArray<twoInt> ints) {
	for (twoInt t : ints) {
		if (t.b == t.a + 1) {
			return true;
		}
	}
	return false;
}

// this function returns all of the first grade Room Polygons, these can later be expanded upon when finalizing the room in RoomBuilder
TArray<FRoomPolygon> getInteriorPlan(FHousePolygon &f, FPolygon hole, bool ground, float corrWidth, float maxRoomArea){
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

	bool isIncreasing = true; //increasing(connections);
	for (int i = 0; i < roomPols.Num(); i++) {
		FRoomPolygon &fp = roomPols[i];

		//if (isIncreasing) {
		for (int j = connections[i].b - 1 == -1 ? f.points.Num() - 1 : connections[i].b - 1; j != (connections[i].a - 1 == -1 ? f.points.Num() - 1 : connections[i].a - 1); j = j == 0 ? f.points.Num() - 1 : j - 1) {
			if (f.windows.Contains(j+1)) {
				fp.windows.Add(fp.points.Num());
			}
			fp.exteriorWalls.Add(fp.points.Num());
			fp.points.Add(f.points[j]);


		}
		//}
		//else {
		//	for (int j = (connections[i].b) % f.points.Num(); j != (connections[i].a) % f.points.Num(); j++, j %= f.points.Num()) {
		//		if (f.windows.Contains(j)) {
		//			fp.windows.Add(fp.points.Num());
		//		}
		//		fp.exteriorWalls.Add(fp.points.Num());
		//		fp.points.Add(f.points[j]);

		//	}
		//}
		if (f.windows.Contains(connections[i].a)) {
			roomPols[i].windows.Add(roomPols[i].points.Num());
		}
		fp.exteriorWalls.Add(fp.points.Num());

		FVector first = roomPols[i].points[0];
		roomPols[i].points.Add(first);
	}
	for (FRoomPolygon p : roomPols) {
		if (FVector::Dist(p.points[0], p.points[p.points.Num() - 1]) > 0.1f) {
			UE_LOG(LogTemp, Warning, TEXT("SPOOK ALERT STOP THE PRESSES"));
		}
	}



	TArray<FRoomPolygon> extra;
	for (FRoomPolygon &p : roomPols) {
		if (p.getArea() > maxRoomArea) {
			FRoomPolygon* newP = p.splitAlongMax(0.5, false);
			if (newP) {
				extra.Add(*newP);
				delete(newP);
			}
		}
	}
	roomPols.Append(extra);
	
	if (corners.points.Num() > 0) {
		corners.points.Add(FVector(corners.points[0]));
	}

	corners.reverse();
	for (int i = 1; i < corners.points.Num(); i += 2) {
		corners.toIgnore.Add(i);
	}
	for (int i = 0; i < corners.points.Num(); i+=2) {
		corners.exteriorWalls.Add(i);
		corners.windows.Add(i);
	}
	roomPols.Add(corners);

	return roomPols;
}





// this returns polygons corresponding to a hole with the shape "hole" in the polygon f, using the keyhole triangulation algorithm
TArray<FMaterialPolygon> getFloorPolygonsWithHole(FPolygon f, float floorBegin, FPolygon hole, bool toMiddle) {
	TArray<FMaterialPolygon> polygons;


	 //move a little to avoid z fighting
	if (toMiddle) {
		FVector middle = f.getCenter();
		for (int i = 0; i < f.points.Num(); i++) {
			FVector dir = f.getPointDirection(i, true, true);
			f.points[i] += dir * 2;
		}
		//for (FVector &p : f.points) {
		//	f.getPointDirection();
		//	FVector tan = middle - p;
		//	tan.Normalize();
		//	p += tan * 2;
		//}
	}


	FMaterialPolygon f2;
	f2.type = PolygonType::floor;
	f2.points = f.points;
	f2.points.RemoveAt(f2.points.Num() - 1);
	hole.points.RemoveAt(hole.points.Num() - 1);
	//hole.reverse();
	f2.offset(FVector(0, 0, floorBegin));
	hole.offset(FVector(0, 0, floorBegin));
	TArray<FPolygon> holes;
	holes.Add(hole);
	return ARoomBuilder::getSideWithHoles(f2, holes, PolygonType::floor);

	hole.points.RemoveAt(hole.points.Num() - 1);

	int closestNum = -1;
	float closestDist = 10000000.0f;
	for (int i = 0; i < hole.points.Num(); i++) {
		FVector f2 = hole.points[i];
		if (FVector::Dist(f2, f.points[0]) < closestDist) {
			closestDist = FVector::Dist(f2, f.points[0]);
			closestNum = i;
		}
	}


	FMaterialPolygon pol;
	pol.type = PolygonType::floor;
	pol.width = 20;
	for (int i = 0; i < f.points.Num(); i++) {
		pol.points.Add(f.points[i] + FVector(0, 0, floorBegin));
	}
	//pol.points.Add(f.points[0] + FVector(0, 0, floorBegin));

	// decide direction
	int change = 1;
	int neg = (closestNum - 1);
	if (neg < 0) {
		neg += hole.points.Num();
	}
	//if (FVector::Dist(hole.points[(closestNum + 1) % hole.points.Num()], f.points[f.points.Num() - 2]) > FVector::Dist(hole.points[neg], f.points[f.points.Num() - 2])) {
	//	change = -1;
	//}
	int count = 0;
	for (int i = closestNum; count < hole.points.Num(); i+=change, i %= hole.points.Num(), count++) {
		if (i < 0)
			i += hole.points.Num();
		pol.points.Add(hole.points[i] + FVector(0, 0, floorBegin));
	}
	pol.points.Add(hole.points[closestNum] + FVector(0, 0, floorBegin));


	pol.points.Add(f.points[0] + FVector(0, 0, floorBegin));
	
	polygons.Add(pol);
	return polygons;


}


FPolygon getShaftHolePolygon(FHousePolygon f) {
	FPolygon hole;
	FVector center = f.getCenter();

	float holeSizeX = 900;
	float holeSizeY = 1200;

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

// changes the shape of the house to make it less cube-like
void makeInteresting(FHousePolygon &f, FHouseInfo &toReturn, FPolygon &centerHole) {
	//return;
	if (randFloat() < 0.1f && f.points.Num() > 3) {
		// move side inwards
		float len = FMath::FRandRange(400, 1000);
		int place = FMath::FRand() * (f.points.Num() - 3) + 2;
		FVector dir1 = f.points[place - 2] - f.points[place - 1];
		dir1.Normalize();
		FVector dir2 = f.points[place + 1] - f.points[place];
		dir2.Normalize();

		FPolygon line;
		//intersection()
		FSimplePlot simplePlot;
		simplePlot.pol.points.Add(f.points[place]);
		simplePlot.pol.points.Add(f.points[place - 1]);
		FVector toChange1To = f.points[place - 1] + dir1 * len;
		FVector toChange2To = f.points[place] + dir2 * len;
		line.points.Add(toChange1To);
		line.points.Add(toChange2To);
		line.points.Add(toChange1To);
		if (intersection(line, centerHole).X == 0.0f) {
			f.points[place - 1] = toChange1To;
			f.points[place] = toChange2To;
			simplePlot.pol.points.Add(f.points[place - 1]);
			simplePlot.pol.points.Add(f.points[place]);
			simplePlot.pol.offset(FVector(0, 0, 30));
			simplePlot.type = FMath::RandBool() ? SimplePlotType::green : SimplePlotType::asphalt;
			simplePlot.decorate();
			toReturn.remainingPlots.Add(simplePlot);

			f.windows.Add(place);
		}



	}

	else if (randFloat() < 0.1f) {
		// remove corner
		int place = FMath::FRand() * (f.points.Num() - 3) + 1;
		FVector p1 = middle(f.points[place - 1], f.points[place]);
		FVector p2 = middle(f.points[place + 1], f.points[place]);

		FSimplePlot simplePlot;
		simplePlot.pol.points.Add(p1);
		simplePlot.pol.points.Add(p2);
		simplePlot.pol.points.Add(f.points[place]);
		simplePlot.pol.points.Add(p1);
		simplePlot.pol.offset(FVector(0, 0, 30));
		if (intersection(simplePlot.pol, centerHole).X == 0.0f) {
			bool hadW = f.entrances.Contains(place);
			bool hadE = f.entrances.Contains(place);

			simplePlot.type = FMath::RandBool() ? SimplePlotType::green : SimplePlotType::asphalt;

			simplePlot.decorate();
			toReturn.remainingPlots.Add(simplePlot);

			f.addPoint(place, p1);
			f.addPoint(place + 1, p2);
			f.removePoint(place + 2);
			f.windows.Add(place + 1);
			f.entrances.Add(place + 1);
			if (hadW)
				f.windows.Add(place);
			if (hadE)
				f.entrances.Add(place);
		}



	}
	else if (randFloat() < 0.1f) {
		float depth = FMath::FRandRange(500, 1000);
		// turn a side inwards into a U
		int place = 2;
		place = FMath::FRand() * (f.points.Num() - 3) + 1;

		FVector tangent = f.points[place] - f.points[place - 1];
		float lenSide = tangent.Size();
		tangent.Normalize();
		FVector dir = FRotator(0, f.buildLeft ? 90 : 270, 0).RotateVector(tangent);
		FVector first = f.points[place - 1] + (tangent * (lenSide / 3));
		FVector first2 = first + dir * depth;
		FVector snd = f.points[place] - (tangent * (lenSide / 3)) ;
		FVector snd2 = snd + dir * depth;

		FSimplePlot simplePlot;
		simplePlot.pol.points.Add(first);
		simplePlot.pol.points.Add(first2);
		simplePlot.pol.points.Add(snd2);
		simplePlot.pol.points.Add(snd);
		simplePlot.pol.points.Add(first);
		if (intersection(simplePlot.pol, centerHole).X == 0.0f) {
			simplePlot.pol.offset(FVector(0, 0, 30));
			simplePlot.type = FMath::RandBool() ? SimplePlotType::green : SimplePlotType::asphalt;

			simplePlot.decorate();
			toReturn.remainingPlots.Add(simplePlot);

			f.addPoint(place, first);
			f.windows.Add(place);
			f.addPoint(place + 1, first2);
			f.windows.Add(place + 1);

			f.addPoint(place + 2, snd2);
			f.windows.Add(place + 2);
			f.entrances.Add(place + 2);

			f.addPoint(place + 3, snd);
			f.windows.Add(place + 3);
			f.windows.Add(place + 4);
		}

	}
}


TArray<FMaterialPolygon> getShaftSides(FPolygon hole, int openSide, float height) {
	TArray<FMaterialPolygon> sides;
	FVector center = hole.getCenter();
	for (int i = 1; i < hole.points.Num(); i++) {
		if (i == openSide)
			continue;
		FMaterialPolygon side;
		side.type = PolygonType::interior;
		side.points.Add(hole.points[i - 1]);
		side.points.Add(hole.points[i - 1] + FVector(0, 0, height));
		side.points.Add(hole.points[i] + FVector(0, 0, height));
		side.points.Add(hole.points[i]);
		side.points.Add(hole.points[i - 1]);
		sides.Add(side);
	}

	// move a little to avoid z fighting
	for (FPolygon &p : sides) {
		for (FVector &f : p.points) {
			FVector tan = center - f;
			tan.Normalize();
			f += tan * 2;
		}
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

void buildRoof(FRoomInfo &info, FHousePolygon pol) {
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


void addFacade(FHousePolygon &f, FRoomInfo &toReturn, float beginHeight, float facadeHeight, float width) {
	for (int i = 1; i < f.points.Num(); i++) {
		FMaterialPolygon fac;
		fac.type = PolygonType::exteriorSnd;
		fac.width = width;

		FVector tangent1 = f.points[i - 1] - f.housePosition;
		FVector tangent2 = f.points[i] - f.housePosition;
		tangent1.Normalize();
		tangent2.Normalize();

		fac.points.Add(f.points[i - 1] + tangent1 * width + FVector(0, 0, beginHeight));
		fac.points.Add(fac.points[0] + FVector(0, 0, facadeHeight));
		fac.points.Add(f.points[i] + tangent2 * width + FVector(0, 0, beginHeight + facadeHeight));
		fac.points.Add(fac.points[2] - FVector(0, 0, facadeHeight));
		fac.points.Add(f.points[i - 1] + tangent1 * width + FVector(0, 0, beginHeight));

		toReturn.pols.Add(fac);

	}
}

void addRoofDetail(FMaterialPolygon &roof, FRoomInfo &toReturn) {
	if (FMath::FRand() < 0.5) {
		// edge detail
		//for (int i = 1; i < roof.points.Num()-1; i++) {
		//	FVector cornerNormal = getNormal(roof.points[i - 1], roof.points[i], true) + getNormal(roof.points[i], roof.points[i + 1], true);
		//	cornerNormal.Normalize();

		//}
		FPolygon shape = roof;
		for (int i = 1; i < roof.points.Num(); i++) {
			FPolygon curr;


		}
	}
	float minHeight = 200;
	float maxHeight = 600;
	float len = 500;
	float offset = FMath::FRandRange(minHeight, maxHeight);
	if (FMath::FRand() < 0.6) {
		// add box shape on top of room
		FMaterialPolygon box;
		bool found = true;
		int count = 0;
		do {
			if (count > 4) {
				found = false;
				break;
			}
			box = FMaterialPolygon();
			box.type = PolygonType::roof;
			FVector center = roof.getCenter();
			FVector p1 = center + FVector(FMath::FRandRange(900, 2500) * FMath::RandBool() ? 1 : -1, FMath::FRandRange(900, 2500) * FMath::RandBool() ? 1 : -1, offset);
			FVector tangent = roof.points[1] - roof.points[0];
			float firstLen = FMath::FRandRange(900, 2500);
			float sndLen = FMath::FRandRange(900, 2500);
			tangent.Normalize();
			FVector p2 = p1 + tangent * firstLen;
			tangent = FRotator(0, 90, 0).RotateVector(tangent);
			FVector p3 = p2 + tangent * sndLen;
			tangent = FRotator(0, 90, 0).RotateVector(tangent);
			FVector p4 = p3 + tangent * firstLen;
			box.points.Add(p1);
			box.points.Add(p4);
			box.points.Add(p3);
			box.points.Add(p2);
			box.points.Add(p1);
			count++;

		} while (intersection(box, roof).X != 0.0f);

		if (found) {
			for (int i = 1; i < box.points.Num(); i++) {
				FMaterialPolygon side;
				side.type = PolygonType::roof;
				side.points.Add(box.points[i - 1]);
				side.points.Add(box.points[i - 1] - FVector(0, 0, offset));
				side.points.Add(box.points[i] - FVector(0, 0, offset));
				side.points.Add(box.points[i]);
				side.points.Add(box.points[i - 1]);
				toReturn.pols.Add(side);
			}
			toReturn.pols.Add(box);
		}
	}
	else if (FMath::FRand() < 0.3){

		// same shape as roof, but smaller
		FMaterialPolygon shape = roof;
		shape.type = PolygonType::roof;
		shape.offset(FVector(0, 0, offset));

		FVector center = shape.getCenter();
		for (FVector &point : shape.points) {
			FVector tangent = center - point;
			//tangent.Normalize();
			point += tangent / 3;
		}
		for (int i = 1; i < shape.points.Num(); i++) {
			FMaterialPolygon side;
			side.type = PolygonType::roof;
			side.points.Add(shape.points[i - 1]);
			side.points.Add(shape.points[i - 1] - FVector(0, 0, offset));
			side.points.Add(shape.points[i] - FVector(0, 0, offset));
			side.points.Add(shape.points[i]);
			side.points.Add(shape.points[i - 1]);
			toReturn.pols.Add(side);
		}
		toReturn.pols.Add(shape);
	}
}

FHouseInfo AHouseBuilder::getHouseInfo(FHousePolygon f, float noiseMultiplier, float floorHeight, float maxRoomArea, bool shellOnly, int minFloors, int maxFloors)
{
	if (f.points.Num() < 3) {
		return FHouseInfo();
	}
	FPolygon hole = getShaftHolePolygon(f);
	FHouseInfo toReturn;
	for (int i = 0; i < makeInterestingAttempts; i++)
		makeInteresting(f, toReturn, hole);
	//noise
	int floors = minFloors + (maxFloors - minFloors) * (FMath::FRand());
	floors *= FMath::FRand() > 0.1 ? 1 : 2;
	//if (scaled_raw_noise_2d(minFloors, maxFloors, f.housePosition.X*noiseMultiplier, f.housePosition.Y*noiseMultiplier) > 0.7f) {
	//	floors *= 2;
	//}
	//int floors = scaled_raw_noise_2d(minFloors, maxFloors, f.housePosition.X*noiseMultiplier, f.housePosition.Y*noiseMultiplier);


	float wDens = randFloat() * 0.0010 + 0.0005;

	float wWidth = randFloat() * 500 + 500;
	float wHeight = randFloat() * 400 + 200;

	TArray<FRoomPolygon> roomPols = getInteriorPlan(f, hole, true, 300, maxRoomArea);

	bool potentialBalcony = f.type == RoomType::apartment && floors < 10;
	for (FRoomPolygon p : roomPols) {
		FRoomInfo newR = ARoomBuilder::buildRoom(&p, f.type, 0, floorHeight, map, false, shellOnly);
		newR.offset(FVector(0, 0, 30));
		toReturn.roomInfo.pols.Append(newR.pols);
		toReturn.roomInfo.meshes.Append(newR.meshes);
	}
	FVector rot = getNormal(hole.points[1], hole.points[0], true);
	rot.Normalize();
	FPolygon stairPol = MeshPolygonReference::getStairPolygon(hole.getCenter() + rot*200, rot.Rotation());
	FPolygon elevatorPol = MeshPolygonReference::getStairPolygon(hole.getCenter() - rot * 200, rot.Rotation());
	FVector stairPos = stairPol.getCenter();
	FVector elevatorPos = elevatorPol.getCenter();

	FPolygon hole2;
	hole2.points = hole.points;
	FVector extra = hole.points[hole.points.Num() - 1];

	hole2.points.Add(extra);

	if (!shellOnly) {
		toReturn.roomInfo.pols.Append(getShaftSides(elevatorPol, 3, floorHeight * floors));
		toReturn.roomInfo.pols.Append(getShaftSides(stairPol, 3, floorHeight * floors));
	}


	bool facade = FMath::FRand() < 0.2;
		for (int i = 1; i < floors; i++) {
			if (!shellOnly) {
				toReturn.roomInfo.pols.Append(getFloorPolygonsWithHole(f, floorHeight*i + 1, stairPol, true));
				toReturn.roomInfo.meshes.Add(FMeshInfo{ "stair", FTransform(rot.Rotation(), stairPos + FVector(0, 0, floorHeight * (i - 1)), FVector(1.0f, 1.0f, 1.0f)) });
			}
			if (facade)
				addFacade(f, toReturn.roomInfo, floorHeight*i + 1, 70, 20);

			roomPols = getInteriorPlan(f, hole, false, 300, 500);
			for (FRoomPolygon &p : roomPols) {
				FRoomInfo newR = ARoomBuilder::buildRoom(&p, f.type, 1, floorHeight, map, potentialBalcony, shellOnly);
				newR.offset(FVector(0, 0, floorHeight*i));
				toReturn.roomInfo.pols.Append(newR.pols);
				toReturn.roomInfo.meshes.Append(newR.meshes);
			}
		}
		if (!shellOnly) {
			for (int i = 1; i <= floors; i++) {
				FVector elDir = elevatorPol.points[2] - elevatorPol.points[1];
				elDir.Normalize();
				toReturn.roomInfo.meshes.Add(FMeshInfo{ "elevator", FTransform(rot.Rotation() + FRotator(0, 180, 0), elevatorPos + FVector(0, 0, floorHeight * (i - 1)) + elDir * 180, FVector(1.0f, 1.0f, 1.0f)) }); // elevator doors
				FMaterialPolygon above; // space above elevator
				above.type = PolygonType::interior;
				above.points.Add(elevatorPol.points[2] + FVector(0, 0, floorHeight * (i - 1) + 290));
				above.points.Add(elevatorPol.points[3] + FVector(0, 0, floorHeight * (i - 1) + 290));
				above.points.Add(elevatorPol.points[3] + FVector(0, 0, floorHeight * (i - 1) + 400));
				above.points.Add(elevatorPol.points[2] + FVector(0, 0, floorHeight * (i - 1) + 400));

				toReturn.roomInfo.pols.Add(above);
			}
		}




	FMaterialPolygon roof;
	roof.points = f.points;
	roof.offset(FVector(0, 0, floorHeight*floors));
	roof.type = PolygonType::roof;
	roof.reverse();

	if (generateRoofs) {
		toReturn.roomInfo.pols.Add(roof);
		addRoofDetail(roof, toReturn.roomInfo);
	}


	FMaterialPolygon floor;
	floor.points = f.points;
	floor.offset(FVector(0, 0, 20));
	floor.type = PolygonType::floor;
	toReturn.roomInfo.pols.Add(floor);

	if (!shellOnly) {
		TArray<FMaterialPolygon> otherSides;
		for (FMaterialPolygon &p : toReturn.roomInfo.pols) {
			otherSides.Add(p);
			FMaterialPolygon other = p;
			// exterior walls are interiors on the inside
			if (p.type == PolygonType::exterior || p.type == PolygonType::exteriorSnd) {
				other.type = PolygonType::interior;
			}

			other.offset(p.getDirection() * p.width);
			for (int i = 1; i < p.points.Num(); i++) {
				FMaterialPolygon newP1;
				newP1.type = p.type;
				newP1.points.Add(other.points[i - 1]);
				newP1.points.Add(p.points[i]);
				newP1.points.Add(p.points[i - 1]);
				otherSides.Add(newP1);

				FMaterialPolygon newP2;
				newP2.type = p.type;
				newP2.points.Add(other.points[i - 1]);
				newP2.points.Add(other.points[i]);
				newP2.points.Add(p.points[i]);
				otherSides.Add(newP2);

			}

			FMaterialPolygon newP1;
			newP1.type = p.type;
			newP1.points.Add(other.points[p.points.Num() - 1]);
			newP1.points.Add(p.points[0]);
			newP1.points.Add(p.points[p.points.Num() - 1]);
			otherSides.Add(newP1);

			FMaterialPolygon newP2;
			newP2.type = p.type;
			newP2.points.Add(other.points[p.points.Num() - 1]);
			newP2.points.Add(other.points[0]);
			newP2.points.Add(p.points[0]);
			otherSides.Add(newP2);

			other.reverse();
			otherSides.Add(other);


		}
		toReturn.roomInfo.pols.Append(otherSides);
		//toReturn.roomInfo.pols = otherSides;

	}

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

