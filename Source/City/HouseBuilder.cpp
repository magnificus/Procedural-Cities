// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
//#include "simplexnoise.h"
//#include "polypartition.h"
#include "HouseBuilder.h"
//#include "ThreadedWorker.h"
struct FPolygon;


// Sets default values
AHouseBuilder::AHouseBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//mapStatic = map;
}

struct twoInt {
	int32 a;
	int32 b;
};

// very crudely checks which direction an array is going, needs to be checked in this way because it can wrap around 0 unfortunately
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

TArray<FMaterialPolygon> getEntrancePolygons(FVector begin, FVector end, float height, float thickness) {
	FMaterialPolygon roof;

	float len = 200;
	FVector dir = getNormal(begin, end, false);
	dir.Normalize();
	roof.type = PolygonType::exteriorSnd;
	roof.width = thickness;
	FVector mainDir = end - begin;
	mainDir.Normalize();
	roof += begin + FVector(0, 0, height) - mainDir * 50;
	roof += end + FVector(0, 0, height) + mainDir * 50;
	roof += end + FVector(0, 0, height) + dir * len + mainDir * 50;
	roof += begin + FVector(0, 0, height) + dir * len - mainDir * 50;


	TArray<FMaterialPolygon> pols;
	pols.Add(roof);
	//fillOutPolygons(pols);
	return pols;
}

// this function returns all of the first grade Room Polygons, these can later be expanded upon when finalizing the room in RoomBuilder
TArray<FRoomPolygon> getInteriorPlanAndPlaceEntrancePolygons(FHousePolygon &f, FPolygon hole, bool ground, float corrWidth, float maxRoomArea, FRandomStream &stream, TArray<FMaterialPolygon> &pols){
	TArray<FLine> lines;
	
	TArray<FRoomPolygon> roomPols;

	FRoomPolygon corners;
	corners.canRefine = false;

	// add corridors from the hole
	TArray<twoInt> connections;
	for (int i = 0; i < hole.points.Num(); i++) {
		roomPols.Add(FRoomPolygon{});
		connections.Add(twoInt{});
	}
	for (int i = 1; i < hole.points.Num() + 1; i++) {
		FVector tangent = hole.points[i%hole.points.Num()] - hole.points[i - 1];
		float midPos = tangent.Size() / 2;
		tangent.Normalize();
		FVector altTangent = FRotator(0, 270, 0).RotateVector(tangent);
		FVector firstAttach = hole.points[i- 1] + (midPos - corrWidth * 0.5) * tangent;
		FVector sndAttach = FVector(0.0f, 0.0f, 0.0f);
		int conn = 0;
		for (int j = 1; j < f.points.Num()+1; j++) {
			FVector res = intersection(firstAttach, firstAttach + altTangent * 100000, f.points[j - 1], f.points[j%f.points.Num()]);
			if (res.X != 0.0f) {
				sndAttach = res;
				conn = j;
				break;
			}

		}
		FVector prevAttach;
		if (sndAttach.X == 0.0f)
			return TArray<FRoomPolygon>();
		if (!ground || !f.entrances.Contains(conn)) {
			corners.points.Add(sndAttach);
		}
		else {
			prevAttach = sndAttach;
		}
		connections[i - 1].b = conn;
		roomPols[i - 1].points.Add(hole.points[i-1]);
		roomPols[i - 1].points.Add(firstAttach);
		roomPols[i - 1].entrances.Add(roomPols[i - 1].points.Num());
		roomPols[i - 1].points.Add(sndAttach);


		firstAttach = hole.points[i-1] + (midPos + corrWidth * 0.5) * tangent;
		sndAttach = FVector(0.0f, 0.0f, 0.0f);
		for (int j = 1; j < f.points.Num()+1; j++) {
			FVector res = intersection(firstAttach, firstAttach + altTangent * 100000, f.points[j - 1], f.points[j%f.points.Num()]);
			if (res.X != 0.0f) {
				sndAttach = res;
				conn = j;
				break;
			}
		}

		if (!ground || !f.entrances.Contains(conn)) {
			if (f.windows.Contains(conn))
				corners.windows.Add(corners.points.Num());
			corners.points.Add(sndAttach);
		}
		else {
			if (stream.FRand() < 0.2 && sndAttach.X != 0.0f && FVector::Dist(prevAttach, sndAttach) < 20000.0f)
				pols.Append(getEntrancePolygons(prevAttach, sndAttach, 450, 50));
		}

		if (i == (hole.points.Num())) {
			TArray<int32> toRemove;
			for (int32 j : roomPols[0].entrances) {
				toRemove.Add(j);
			}
			for (int32 j : toRemove) {
				roomPols[0].entrances.Remove(j);
				// offset bc of the new points we're adding here
				roomPols[0].entrances.Add(j + 3);
			}
			connections[0].a = conn;
			roomPols[0].points.EmplaceAt(0, sndAttach);
			roomPols[0].entrances.Add(1);
			roomPols[0].points.EmplaceAt(1, firstAttach);
			roomPols[0].points.EmplaceAt(2, hole.points[i%hole.points.Num()]);
		}
		else {
			connections[i].a = conn;
			roomPols[i].points.Add(sndAttach);
			roomPols[i].entrances.Add(roomPols[i].points.Num());
			roomPols[i].points.Add(firstAttach);
			roomPols[i].points.Add(hole.points[i%hole.points.Num()]);
		}


	}

	// sew the roomPolygons together by adding walls

	for (int i = 0; i < roomPols.Num(); i++) {
		FRoomPolygon &fp = roomPols[i];

		//if (isIncreasing) {
		for (int j = connections[i].b - 1 == -1 ? f.points.Num(): connections[i].b - 1; j != (connections[i].a - 1 == -1 ? f.points.Num(): connections[i].a - 1); j = j == 0 ? f.points.Num(): j - 1) {
			if (f.windows.Contains(j+1)) {
				fp.windows.Add(fp.points.Num());
			}
			fp.exteriorWalls.Add(fp.points.Num());
			fp.points.Add(f.points[j%f.points.Num()]);


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

		//FVector first = roomPols[i].points[0];
		//roomPols[i].points.Add(first);
	}



	TArray<FRoomPolygon> extra;
	for (FRoomPolygon &p : roomPols) {
		if (p.getArea() > maxRoomArea) {
			FRoomPolygon* newP = p.splitAlongMax(0.5, false);
			if (newP) {
				newP->entrances.Add(1);
				p.entrances.Add(1);
				extra.Add(*newP);
				delete(newP);
			}
		}
	}
	roomPols.Append(extra);
	


	corners.reverse();
	if (corners.points.Num() > 0) {
		corners.points.Add(FVector(corners.points[0]));
	}
	for (int i = 0; i < corners.points.Num(); i += 2) {
		corners.toIgnore.Add(i);
	}
	for (int i = 1; i < corners.points.Num(); i+=2) {
		corners.exteriorWalls.Add(i);
		//corners.windows.Add(i);
	}
	roomPols.Add(corners);

	return roomPols;
}





// this returns polygons corresponding to a hole with the shape "hole" in the polygon f
TArray<FMaterialPolygon> getFloorPolygonsWithHole(FPolygon f, float floorBegin, FPolygon hole) {
	TArray<FMaterialPolygon> polygons;

	FMaterialPolygon f2;
	f2.type = PolygonType::floor;
	f2.points = f.points;

	//f2.points.RemoveAt(f2.points.Num() - 1);

	//hole.points.RemoveAt(hole.points.Num() - 1);
	f2.offset(FVector(0, 0, floorBegin));
	hole.offset(FVector(0, 0, floorBegin));

	f2.normal = FVector(0, 0, -1);
	polygons.Add(f2);


	TArray<FMaterialPolygon> pols;
	TArray<FPolygon> holes;
	//if (!f2.getIsClockwise())
	//	f2.reverse();

	if (hole.getIsClockwise())
		hole.reverse();
	holes.Add(hole);
	pols = ARoomBuilder::getSideWithHoles(f2, holes, PolygonType::floor);

	for (auto &pol : pols)
		pol.normal = FVector(0, 0, -1);

	return pols;
	FMaterialPolygon floorP;
	floorP.points = f.points;
	floorP.type = PolygonType::floor;
	floorP.offset(FVector(0, 0, floorBegin));
	//hole.offset(FVector(0, 0, floorBegin));
	//hole.reverse();
	int closestH = 0;
	float closestHDist = FVector::Dist(hole[0], floorP[0]);;
	for (int i = 1; i < hole.points.Num(); i++) {
		float currDist = FVector::Dist(hole[i], floorP[0]);
		if (currDist < closestHDist) {
			closestHDist = currDist;
			closestH = i;
		}
	}
	floorP.normal = FVector(0, 0, -1);
	floorP.points.Add(FVector(floorP[0]));
	floorP.points.Add(hole[closestH]);
	int start = closestH == hole.points.Num() - 1 ? 0 : closestH + 1;
	for (int i = start; i != closestH; i++, i %= hole.points.Num()/*i = i == 0 ? hole.points.Num()-1 : i-1*/) {
		floorP.points.Add(hole[i]);
	}
	//int start = closestH == 0 ? hole.points.Num()-1: closestH + 1;
	//for (int i = start; i != closestH; i--) {
	//	if (i < 0)
	//		i = hole.points.Num() - 1;
	//	floorP.points.Add(hole[i]);
	//}

	floorP.points.Add(hole[closestH]);

	pols.Add(floorP);
	 return pols;

}


FPolygon AHouseBuilder::getShaftHolePolygon(FHousePolygon f) {
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
	//hole.points.Add(center + 0.5 * tangent1 * holeSizeX + 0.5 * tangent2 * holeSizeY);



	return hole;
}



// return polygon covering the space that was removed from f
FPolygon attemptMoveSideInwards(FHousePolygon &f, int place, FPolygon &centerHole, float len, FVector offset) {
	int prev2 = place > 1 ? place - 2 : place - 3 + f.points.Num();
	FVector dir1 = f.points[prev2] - f.points[place - 1];
	dir1.Normalize();
	FVector dir2 = f.points[(place + 1)%f.points.Num()] - f.points[place%f.points.Num()];
	dir2.Normalize();

	FPolygon line;
	//intersection()
	//FSimplePlot simplePlot;
	FPolygon pol;
	pol.points.Add(f.points[place%f.points.Num()]);
	pol.points.Add(f.points[place - 1]);
	FVector toChange1To = f.points[place - 1] + dir1 * len;
	FVector toChange2To = f.points[place%f.points.Num()] + dir2 * len;
	line.points.Add(toChange1To);
	line.points.Add(toChange2To);
	line.points.Add(toChange1To);
	if (intersection(line, centerHole).X == 0.0f && intersection(line, f).X == 0.0f) {
		f.points[place - 1] = toChange1To;
		f.points[place%f.points.Num()] = toChange2To;
		pol.points.Add(toChange1To);
		pol.points.Add(toChange2To);
		pol.offset(offset);
		f.windows.Add(place%f.points.Num());
		return pol;
	}
	return FPolygon();
}

// this method changes the shape of the house to make it less cube-like, can be called several times for more "interesting" shapes 
void AHouseBuilder::makeInteresting(FHousePolygon &f, TArray<FSimplePlot> &toReturn, FPolygon &centerHole, FRandomStream stream) {
	//return;
	int place = stream.FRandRange(0, 0.99) * (f.points.Num() - 1) + 1; 
	if (stream.FRand() < 0.15f && f.points.Num() > 3) {
		// move side inwards
		float len = stream.FRandRange(400, 1500);
		FPolygon res = attemptMoveSideInwards(f, place, centerHole, len, FVector(0,0,30));
		if (res.points.Num() > 0) {
			FSimplePlot simplePlot;
			simplePlot.pol = res;
			//simplePlot.pol.points.Add(FVector(simplePlot.pol.points[0]));
			simplePlot.pol.reverse();
			simplePlot.type = f.simplePlotType;
			toReturn.Add(simplePlot);

		}


	}

	else if (stream.FRand() < 0.1f && f.points.Num() > 3 && FVector::Dist(f.points[place%f.points.Num()], f.points[place-1]) > 500) {
		// remove corner
		FVector p1 = middle(f.points[place - 1], f.points[place%f.points.Num()]);
		FVector p2 = middle(f.points[(place + 1) % f.points.Num()], f.points[place%f.points.Num()]);

		FSimplePlot simplePlot;
		simplePlot.pol.points.Add(p1);
		simplePlot.pol.points.Add(p2);
		simplePlot.pol.points.Add(f.points[place%f.points.Num()]);
		//simplePlot.pol.points.Add(p1);
		//simplePlot.pol.points.Add(p1);
		simplePlot.pol.offset(FVector(0, 0, 30));
		if (intersection(simplePlot.pol, centerHole).X == 0.0f) {
			bool hadW = f.windows.Contains(place);
			bool hadE = f.entrances.Contains(place);

			simplePlot.type = f.simplePlotType; // stream.RandBool() ? SimplePlotType::green : SimplePlotType::asphalt;

			simplePlot.pol.reverse();
			toReturn.Add(simplePlot);

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
	else if (stream.FRand() < 0.1f) {
		float depth = stream.FRandRange(500, 1500);
		// turn a side inwards into a U
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
		simplePlot.pol.points.Add(snd);
		simplePlot.pol.points.Add(snd2);
		simplePlot.pol.points.Add(first2);
		//simplePlot.pol.points.Add(first);

		FHousePolygon cp = f;
		if (intersection(simplePlot.pol, centerHole).X == 0.0f && !selfIntersection(simplePlot.pol)){
			simplePlot.pol.offset(FVector(0, 0, 30));
			simplePlot.type = f.simplePlotType;

			toReturn.Add(simplePlot);

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
		if (selfIntersection(f))
			f = cp;
	}
}


TArray<FMaterialPolygon> AHouseBuilder::getShaftSides(FPolygon hole, int openSide, float height) {
	TArray<FMaterialPolygon> sides;
	FVector center = hole.getCenter();
	for (int i = 1; i < hole.points.Num()+1; i++) {
		if (i == openSide)
			continue;
		FMaterialPolygon side;
		side.type = PolygonType::interior;
		side.points.Add(hole.points[i - 1]);
		side.points.Add(hole.points[i - 1] + FVector(0, 0, height));
		side.points.Add(hole.points[i%hole.points.Num()] + FVector(0, 0, height));
		side.points.Add(hole.points[i%hole.points.Num()]);
		//side.points.Add(hole.points[i - 1]);
		sides.Add(side);
	}

	return sides;
}


void addStairInfo(FRoomInfo &info, float height, FPolygon &hole) {
	float distFromCorners = 300;
	TArray<FMaterialPolygon> sides;
	for (int i = 1; i < hole.points.Num()+1; i++) {
		FMaterialPolygon side;
		side.points.Add(hole.points[i - 1]);
		side.points.Add(hole.points[i - 1] + FVector(0, 0, height));
		side.points.Add(hole.points[i%hole.points.Num()] + FVector(0, 0, height));
		side.points.Add(hole.points[i%hole.points.Num()]);
		//side.points.Add(hole.points[i - 1]);
		sides.Add(side);
	}
	info.pols.Append(sides);

}

void addFacade(FHousePolygon &f, FRoomInfo &toReturn, float beginHeight, float facadeHeight, float width) {
	for (int i = 1; i < f.points.Num()+1; i++) {
		FMaterialPolygon fac;
		fac.type = PolygonType::exteriorSnd;
		fac.width = width;

		FVector tangent1 = f.points[i - 1] - f.housePosition;
		FVector tangent2 = f.points[i%f.points.Num()] - f.housePosition;
		tangent1.Normalize();
		tangent2.Normalize();

		fac.points.Add(f.points[i - 1] + tangent1 * width + FVector(0, 0, beginHeight));
		fac.points.Add(fac.points[0] + FVector(0, 0, facadeHeight));
		fac.points.Add(f.points[i%f.points.Num()] + tangent2 * width + FVector(0, 0, beginHeight + facadeHeight));
		fac.points.Add(fac.points[2] - FVector(0, 0, facadeHeight));
		//fac.points.Add(f.points[i - 1] + tangent1 * width + FVector(0, 0, beginHeight));

		toReturn.pols.Add(fac);

	}
}


// recursive method for adding details to part of a roof
void addDetailOnPolygon(int depth, int maxDepth, int maxBoxes, FMaterialPolygon pol, FRoomInfo &toReturn, FRandomStream stream) {
	if (depth == maxDepth)
		return;
	TArray<FMaterialPolygon> nextShapes;
	if (stream.FRand() < 0.5) {
		// edge detail
		FMaterialPolygon shape = pol;
		float size = stream.FRandRange(100, 250);
		shape.offset(FVector(0, 0, size));
		TArray<FMaterialPolygon> sides = getSidesOfPolygon(shape, PolygonType::exteriorSnd, size);
		toReturn.pols.Append(fillOutPolygons(sides));
		toReturn.pols.Append(sides);

	}
	float minHeight = 100;
	float maxHeight = 1000;
	float len = 500;
	float offset = stream.FRandRange(minHeight, maxHeight);
	if (stream.FRand() < 0.5) {
		int numBoxes = stream.RandRange(0, maxBoxes);
		// add box shapes on top of pol
		for (int j = 0; j < numBoxes; j++) {
			offset = stream.FRandRange(minHeight, maxHeight);
			FMaterialPolygon box;
			bool found = true;
			int count = 0;
			do {
				if (count > 4) {
					found = false;
					break;
				}
				box = FMaterialPolygon();
				box.type = PolygonType::exteriorSnd;
				FVector center = pol.getCenter();
				FVector p1 = center + FVector(stream.FRandRange(0, 4000) * (stream.FRand() < 0.5 ? 1 : -1), stream.FRandRange(0, 4000) * (stream.FRand() < 0.5 ? 1 : -1), offset);
				FVector tangent = pol.points[1] - pol.points[0];
				float firstLen = stream.FRandRange(500, 4000);
				float sndLen = stream.FRandRange(500, 4000);
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
				//box.points.Add(p1);
				count++;

			} while (intersection(box, pol).X != 0.0f || !testCollision(box, pol, 0));

			if (found) {
				//TArray<FMaterialPolygon> boxA;
				//box.width = offset;
				//boxA.Add(box);
				//toReturn.pols.Append(fillOutPolygons(boxA));
				for (int i = 1; i < box.points.Num()+1; i++) {
					FMaterialPolygon side;
					side.type = PolygonType::exteriorSnd;
					side.points.Add(box.points[i%box.points.Num()]);
					side.points.Add(box.points[i%box.points.Num()] - FVector(0, 0, offset));
					side.points.Add(box.points[i - 1] - FVector(0, 0, offset));
					side.points.Add(box.points[i - 1]);

					toReturn.pols.Add(side);
				}
				toReturn.pols.Add(box);
				nextShapes.Add(box);
			}
		}
	}
	else if (stream.FRand() < 0.5) {

		// same shape as pol, but smaller 
		FMaterialPolygon shape = pol;
		shape.type = pol.type;
		shape.offset(FVector(0, 0, offset));

		FVector center = shape.getCenter();
		for (FVector &point : shape.points) {
			FVector tangent = center - point;
			//tangent.Normalize();
			point += tangent / 4;
		}
		//TArray<FMaterialPolygon> shapeA;
		//shape.width = offset;
		//shapeA.Add(shape);
		//toReturn.pols.Append(fillOutPolygons(shapeA));

		for (int i = 1; i < shape.points.Num() + 1; i++) {
			FMaterialPolygon side;
			side.type = PolygonType::roof;
			side.points.Add(shape.points[i - 1]);
			side.points.Add(shape.points[i - 1] - FVector(0, 0, offset));
			side.points.Add(shape.points[i%shape.points.Num()] - FVector(0, 0, offset));
			side.points.Add(shape.points[i%shape.points.Num()]);
			toReturn.pols.Add(side);
		}


		toReturn.pols.Add(shape);
		nextShapes.Add(shape);
	}

	for (FMaterialPolygon p : nextShapes)
		addDetailOnPolygon(depth++, maxDepth, 1, p, toReturn, stream);
}

void addRoofDetail(FMaterialPolygon &roof, FRoomInfo &toReturn, FRandomStream stream) {
	addDetailOnPolygon(0, 2, 2, roof, toReturn, stream);
}


TArray<FMaterialPolygon> potentiallyShrink(FHousePolygon &f, FPolygon &centerHole, FRandomStream stream) {
	TArray<FMaterialPolygon> toReturn;
	FHousePolygon newF = FHousePolygon(f);
	if (stream.FRand() < 0.3) {
		// only shrink one side
		float len = stream.FRandRange(400, 1500);
		int place = stream.FRandRange(0, 0.99) * (f.points.Num() - 3) + 2;
		FPolygon res = attemptMoveSideInwards(f, place, centerHole, len, FVector(0,0,0));
		if (res.points.Num() > 2) {
			FMaterialPolygon pol;
			pol.points = res.points;
			pol.offset(FVector(0, 0, 20));
			pol.type = PolygonType::roof;
			pol.normal = FVector(0, 0, 1);
			toReturn.Add(pol);
			return toReturn;
		}
	}
	// shrink whole symmetrically
	else if (stream.FRand() < 0.3) {
		float len = stream.FRandRange(400, 1200);
		FHousePolygon cp = FHousePolygon(f);
		cp.symmetricShrink(len, cp.buildLeft);
		if (intersection(cp, centerHole).X == 0.0f && !selfIntersection(cp)) {
			//f.points.RemoveAt(f.points.Num() - 1);
			f.reverse();
			TArray<FPolygon> holes;
			holes.Add(cp);
			toReturn = ARoomBuilder::getSideWithHoles(f, holes, PolygonType::roof);
			for (auto &a : toReturn) {
				a.offset(FVector(0, 0, 20));
				a.normal = FVector(0, 0, 1);
			}
			f = cp;
			for (int i = 1; i < f.points.Num(); i++) {
				f.windows.Add(i);
			}
		}
	}
	return toReturn;
}

void AHouseBuilder::buildHouse(FHousePolygon f, float floorHeight, float maxRoomArea, bool shellOnly, bool simple, bool fullReplacement) {
	housePol = f;

	worker = new ThreadedWorker(this, f, floorHeight, maxRoomArea, shellOnly, simple, fullReplacement);
	workerWorking = true;
}

void AHouseBuilder::buildHouseFromInfo(FHouseInfo res, bool fullReplacement) {
	if (procMeshActorClass) {
		if (procMeshActor) {
			procMeshActor->clearMeshes(fullReplacement);
			for (auto pair : map)
				pair.Value->ClearInstances();
		}
		else {
			procMeshActor = GetWorld()->SpawnActor<AProcMeshActor>(procMeshActorClass, FActorSpawnParameters());
		}
		//UE_LOG(LogTemp, Log, TEXT("procmeshactor assigned"));
	}
	if (!procMeshActor)
		return;
	TArray<FSimplePlot> otherSides;
	if (firstTime) {
		firstTime = false;
		for (FSimplePlot fs : res.remainingPlots) {
			fs.decorate();
			for (FMeshInfo mesh : fs.meshes) {
				if (mesh.instanced) {
					if (map.Find(mesh.description))
						map[mesh.description]->AddInstance(mesh.transform);
				}
				else {
					if (staticMap.Find(mesh.description)) {
						auto object = NewObject<UStaticMeshComponent>(staticMap[mesh.description]);
						//object->transform
						object->SetWorldTransform(mesh.transform);
					}

				}
			}
			//fs.pol.reverse();
			//otherSides.Add(fs);
		}
	}
	res.roomInfo.pols.Append(BaseLibrary::getSimplePlotPolygons(res.remainingPlots));

	procMeshActor->buildPolygons(res.roomInfo.pols, FVector(0, 0, 0));
	for (FMeshInfo mesh : res.roomInfo.meshes) {
		if (map.Find(mesh.description))
			map[mesh.description]->AddInstance(mesh.transform);
	}
}

FHouseInfo AHouseBuilder::getHouseInfoSimple(FHousePolygon f, float floorHeight, float maxRoomArea) {
	FHouseInfo toReturn;
	for (int i = 1; i < f.points.Num() + 1; i++) {
		FMaterialPolygon side;
		side.type = PolygonType::exterior;
		side.points.Add(f.points[i - 1] + FVector(0, 0, f.height*floorHeight));
		side.points.Add(f.points[i - 1]);
		side.points.Add(f.points[i% f.points.Num()]);
		side.points.Add(f.points[i% f.points.Num()] + FVector(0, 0, f.height*floorHeight));
		//side.points.Add(f.points[i - 1] + FVector(0, 0, f.height*floorHeight));
		side.reverse();
		toReturn.roomInfo.pols.Add(side);
	}
	FMaterialPolygon roof;
	roof.points = f.points;
	roof.type = PolygonType::roof;
	roof.offset(FVector(0, 0, f.height*floorHeight));
	roof.reverse();
	toReturn.roomInfo.pols.Add(roof);

	return toReturn;

}


FHouseInfo AHouseBuilder::getHouseInfo(FHousePolygon f, float floorHeight, float maxRoomArea, bool shellOnly)
{
	float dist = FVector::Dist(f[0], f[f.points.Num() - 1]);
	UE_LOG(LogTemp, Warning, TEXT("dist between start and end: %f"), dist);
	FRandomStream stream;
	float corrWidth = 300;
	stream.Initialize(f.housePosition.X * 1000 + f.housePosition.Y);
	//if (f.points.Num() < 3) {
	//	return FHouseInfo();
	//}
	f.checkOrientation();
	FPolygon hole = getShaftHolePolygon(f);
	FHouseInfo toReturn;
	if (f.canBeModified) {
		for (int i = 0; i < makeInterestingAttempts; i++) {
			makeInteresting(f, toReturn.remainingPlots, hole, stream);
		}
	}
	int floors = f.height;


	float wDens = stream.FRand() * 0.0010 + 0.0005;

	float wWidth = stream.FRand() * 500 + 500;
	float wHeight = stream.FRand() * 400 + 200;

	TArray<FRoomPolygon> roomPols = getInteriorPlanAndPlaceEntrancePolygons(f, hole, true, corrWidth, maxRoomArea, stream, toReturn.roomInfo.pols);

	bool potentialBalcony = f.type == RoomType::apartment && floors < 10 && stream.FRand() < 0.3;
	for (FRoomPolygon &p : roomPols) {
		RoomType toUse = f.type;
		p.windowType = WindowType::rectangular;
		if (p.windows.Num() > 0 && f.type == RoomType::apartment) {
			for (int i : p.windows) {
				p.entrances.Add(i);
			}
			toUse = stream.FRand() > 0.5 ? RoomType::store : RoomType::restaurant;
		}
		FRoomInfo newR = ARoomBuilder::buildRoom(&p, toUse, 0, floorHeight, map, false, shellOnly, stream);
		newR.offset(FVector(0, 0, 30));
		toReturn.roomInfo.pols.Append(newR.pols);
		toReturn.roomInfo.meshes.Append(newR.meshes);
	}
	FVector rot = getNormal(hole.points[1], hole.points[0], true);
	rot.Normalize();
	FPolygon stairPol = MeshPolygonReference::getStairPolygon(hole.getCenter() + rot*200, rot.Rotation());
	FPolygon elevatorPol = MeshPolygonReference::getStairPolygon(hole.getCenter() - rot * 200, rot.Rotation());
	FVector stairPos = stairPol.getCenter();
	//if (!stairPol.getIsClockwise())
	//	stairPol.reverse();
	FVector elevatorPos = elevatorPol.getCenter();

	if (!shellOnly) {
		toReturn.roomInfo.pols.Append(getShaftSides(elevatorPol, 3, floorHeight * floors));
		toReturn.roomInfo.pols.Append(getShaftSides(stairPol, 3, floorHeight * floors));

		FMaterialPolygon floor;
		floor.points = f.points;
		floor.offset(FVector(0, 0, 20));
		floor.type = PolygonType::floor;
		toReturn.roomInfo.pols.Add(floor);
	}

	int windowChangeCutoff = stream.RandRange(1, 20);
	WindowType currentWindowType = WindowType(stream.RandRange(0, 3));
	bool facade = stream.FRand() < 0.2;
		for (int i = 1; i < floors; i++) {
			if (i == windowChangeCutoff)
				currentWindowType = WindowType(stream.RandRange(0,3));

			if (stream.FRand() < 0.15 && f.canBeModified) {
				TArray<FMaterialPolygon> res = potentiallyShrink(f, hole,stream);
				for (FMaterialPolygon &fm : res) {
					fm.offset(FVector(0,0,floorHeight*i));
				}
				toReturn.roomInfo.pols.Append(res);
			}

			if (!shellOnly) {
				TArray<FMaterialPolygon> floor = getFloorPolygonsWithHole(f, floorHeight*i + 1, stairPol);
				//for (auto &pol : floor) {
				//	if(!pol.getIsClockwise())
				//		pol.reverse();
				//}
				toReturn.roomInfo.pols.Append(floor);
				toReturn.roomInfo.meshes.Add(FMeshInfo{ "stair", FTransform(rot.Rotation(), stairPos + FVector(0, 0, floorHeight * (i - 1)), FVector(1.0f, 1.0f, 1.0f)) });
			}
			if (facade)
				addFacade(f, toReturn.roomInfo, floorHeight*i + 1, 70, 20);

			roomPols = getInteriorPlanAndPlaceEntrancePolygons(f, hole, false, corrWidth, maxRoomArea, stream, toReturn.roomInfo.pols);
			for (FRoomPolygon &p : roomPols) {
				p.windowType = currentWindowType;
				FRoomInfo newR = ARoomBuilder::buildRoom(&p, f.type, 1, floorHeight, map, potentialBalcony, shellOnly, stream);
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
	//roof.points.RemoveAt(roof.points.Num() - 1);
	roof.offset(FVector(0, 0, floorHeight*floors));
	roof.type = PolygonType::roof;
	//roof.reverse();

	if (generateRoofs)
		toReturn.roomInfo.pols.Add(roof);

	if (!shellOnly) {
		TArray<FMaterialPolygon> otherSides = fillOutPolygons(toReturn.roomInfo.pols);
		toReturn.roomInfo.pols.Append(otherSides);
	}

	if (generateRoofs) {
		addRoofDetail(roof, toReturn.roomInfo, stream);
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
	if (workerWorking && worker->IsFinished()) {
		buildHouseFromInfo(worker->resultingInfo, worker->fullReplacement);
		workerWorking = false;
	}


}

