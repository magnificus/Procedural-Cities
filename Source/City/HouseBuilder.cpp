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


TArray<FLine> getInteriorPlanGround(FHousePolygon f, FPolygon hole){
	TArray<FLine> lines;

	TArray<FLine> inputLines;

	// add the walls
	for (int i = 1; i < f.points.Num(); i++) {
		inputLines.Add(FLine{ f.points[i - 1], f.points[i], 10 });
	}

	// add corridors to shaft
	for (int i : f.entrances) {
		FVector middle = (f.points[i] - f.points[i - 1] ) / 2 + f.points[i - 1];
		inputLines.Add(FLine{ middle, hole.getCenter(), 200});
	}


	TArray<FMetaPolygon> pols = BaseLibrary::getSurroundingPolygons(inputLines);

	for (FMetaPolygon p : pols) {
		for (int i = 1; i < p.points.Num(); i++) {
			lines.Add(FLine{ p.points[i - 1], p.points[i], 10 });
		}
	}
	return lines;
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

TArray<FPolygon> getFloorPolygons(FHousePolygon &f, float floorBegin, float floorHeight, float windowDensity, float windowBegin, float windowWidth, float windowHeight, FPolygon hole) {
	TArray<FPolygon> polygons;

	//FPolygon floor;

	//for (FVector f2 : f.points) {
	//	floor.points.Add(f2 + FVector(0,0, floorBegin));
	//}

	polygons.Append(getFloorPolygonsWithHole(f, floorBegin, hole));
	for (int i = 1; i < f.points.Num(); i++) {
		FPolygon outer;
		outer.points.Add(f.points[i - 1] + FVector(0, 0, floorHeight + floorBegin));
		outer.points.Add(f.points[i - 1] + FVector(0, 0, floorBegin));
		outer.points.Add(f.points[i] + FVector(0, 0, floorBegin));
		outer.points.Add(f.points[i] + FVector(0, 0, floorHeight + floorBegin));

		FVector tangent = f.points[i] - f.points[i - 1];
		float len = tangent.Size();
		tangent.Normalize();

		TArray<FPolygon> windows;
		int spaces = FMath::FloorToInt(windowDensity * len) + 1;
		float jumpLen = len / (float) spaces;

		if (f.windows.Contains(i)) {
			for (int j = 1; j < spaces; j++) {
				FPolygon currWindow;
				FVector p1 = f.points[i - 1] + tangent * j * jumpLen + FVector(0, 0, floorBegin + windowBegin + windowHeight) - (tangent * windowWidth / 2);
				FVector p2 = p1 - FVector(0, 0, windowHeight);
				FVector p3 = p2 + tangent * windowWidth;
				FVector p4 = p3 + FVector(0, 0, windowHeight);

				currWindow.points.Add(p1);
				currWindow.points.Add(p2);
				currWindow.points.Add(p3);
				currWindow.points.Add(p4);

				windows.Add(currWindow);

			}
		}


		TArray<FPolygon> pols = getSideWithHoles(outer, windows);

		polygons.Append(pols);

	}
	return polygons;
}

FPolygon getShaftHolePolygon(FHousePolygon f) {
	FPolygon hole;
	FVector center = f.getCenter();

	float holeSize = 1500;

	FVector tangent1 = f.points[1] - f.points[0];
	tangent1.Normalize();
	FVector tangent2 = FRotator(0, 90, 0).RotateVector(tangent1);


	hole.points.Add(center + 0.5 * tangent1 * holeSize + 0.5 * tangent2 * holeSize);
	hole.points.Add(center - 0.5 * tangent1 * holeSize + 0.5 * tangent2 * holeSize);
	hole.points.Add(center - 0.5 * tangent1 * holeSize - 0.5 * tangent2 * holeSize);
	hole.points.Add(center + 0.5 * tangent1 * holeSize - 0.5 * tangent2 * holeSize);
	hole.points.Add(center + 0.5 * tangent1 * holeSize + 0.5 * tangent2 * holeSize);

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

TArray<FPolygon> AHouseBuilder::getHousePolygons(FHousePolygon f, int floors, float floorHeight)
{
	if (f.points.Num() < 3) {
		return TArray < FPolygon>();
	}
	makeInteresting(f);
	TArray<FPolygon> toReturn = getGroundPolygons(f, floorHeight, 400, 200);

	float wDens = randFloat() * 0.0010 + 0.0005;

	float wWidth = randFloat() * 500 + 500;
	float wHeight = randFloat() * 400 + 200;

	FPolygon hole = getShaftHolePolygon(f);

	toReturn.Append(getShaftSides(hole, 1, floorHeight * floors));

	TArray<FLine> floorLines = getInteriorPlanGround(f, hole);
	for (FLine f : floorLines) {
		FPolygon newP;
		newP.points.Add(f.p1 + FVector(0, 0, floorHeight));
		newP.points.Add(f.p1);
		newP.points.Add(f.p2);
		newP.points.Add(f.p2 + FVector(0, 0, floorHeight));
	}


	float currHeight = floorHeight;
	for (int i = 1; i < 2; i++) {
		toReturn.Append(getFloorPolygons(f, currHeight, floorHeight, wDens, floorHeight / 3, wWidth, wHeight, hole));
		currHeight += floorHeight;
	}
	FPolygon roof;
	for (FVector f2 : f.points) {
		roof.points.Add(f2 + FVector(0, 0, currHeight));
	}
	//toReturn.Add(roof);
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

