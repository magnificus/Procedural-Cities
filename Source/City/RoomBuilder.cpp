// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "RoomBuilder.h"


// Sets default values
ARoomBuilder::ARoomBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARoomBuilder::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARoomBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/*
holes are assumed to be of a certain structure, just four points upper left -> lower left -> lower right -> upper right, no window is allowed to overlap another or WEIRD things happen
0---3
|	|
1---2
*/

TArray<FMaterialPolygon> ARoomBuilder::getSideWithHoles(FMaterialPolygon outer, TArray<FPolygon> holes) {

	TArray<FMaterialPolygon> polygons;
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
			FMaterialPolygon p1 = FMaterialPolygon();
			p1.points.Add(attach1);
			p1.points.Add(p.points[3]);

			p1.points.Add((p.points[3] - outer.points[0]).ProjectOnTo(tangentUp) + outer.points[0]);

			FMaterialPolygon p2 = FMaterialPolygon();
			p2.points.Add(attach1);
			p2.points.Add(attach2);
			p2.points.Add(p.points[3]);

			FMaterialPolygon p3 = FMaterialPolygon();
			p3.points.Add(attach2);
			p3.points.Add(p.points[1]);
			p3.points.Add(p.points[0]);

			FMaterialPolygon p4 = FMaterialPolygon();
			p4.points.Add(attach2);
			p4.points.Add(attach3);
			p4.points.Add(p.points[1]);

			FMaterialPolygon p5 = FMaterialPolygon();
			p5.points.Add(attach3);
			p5.points.Add((p.points[2] - outer.points[1]).ProjectOnTo(tangentDown) + outer.points[1]);
			p5.points.Add(p.points[2]);

			FMaterialPolygon p6 = FMaterialPolygon();
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

		FMaterialPolygon p7;
		p7.points.Add(attach1);
		p7.points.Add(attach4);
		p7.points.Add(outer.points[2]);

		FMaterialPolygon p8;
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


TArray<FMaterialPolygon> getEntranceSide(FVector p1, FVector p2, float floorHeight, float doorHeight, float doorWidth) {
	TArray<FMaterialPolygon> polygons;


	FVector side = p2 - p1;
	float sideLen = side.Size();
	side.Normalize();
	float distToDoor = (sideLen - doorWidth) / 2;
	TArray<FPolygon> holes;
	FMaterialPolygon doorPolygon;
	doorPolygon.points.Add(p1 + side*distToDoor + FVector(0, 0, doorHeight));
	doorPolygon.points.Add(p1 + side*distToDoor);// + FVector(0, 0, 100));
	doorPolygon.points.Add(p2 - side*distToDoor);// + FVector(0, 0, 100));
	doorPolygon.points.Add(p2 - side*distToDoor + FVector(0, 0, doorHeight));
	holes.Add(doorPolygon);

	FMaterialPolygon outer;
	outer.points.Add(p1 + FVector(0, 0, floorHeight));
	outer.points.Add(p1 + FVector(0, 0, 0));
	outer.points.Add(p2 + FVector(0, 0, 0));
	outer.points.Add(p2 + FVector(0, 0, floorHeight));
	polygons.Append(ARoomBuilder::getSideWithHoles(outer, holes));

	return polygons;
}


TArray<FMaterialPolygon> ARoomBuilder::interiorPlanToPolygons(TArray<FRoomPolygon> roomPols, float floorHeight, float windowDensity, float windowHeight, float windowWidth) {
	TArray<FMaterialPolygon> toReturn;

	for (FRoomPolygon rp : roomPols) {
		for (int i = 1; i < rp.points.Num(); i++) {
			if (rp.toIgnore.Contains(i)) {
				continue;
			}
			FMaterialPolygon newP;
			newP.type = PolygonType::exterior;
			FVector p1 = rp.points[i - 1];
			FVector p2 = rp.points[i];
			newP.points.Add(p1 + FVector(0, 0, floorHeight));
			newP.points.Add(p1 + FVector(0, 0, 0));
			newP.points.Add(p2 + FVector(0, 0, 0));
			newP.points.Add(p2 + FVector(0, 0, floorHeight));
			newP.points.Add(p1 + FVector(0, 0, floorHeight));

			if (rp.entrances.Contains(i)) {
				toReturn.Append(getEntranceSide(rp.points[i - 1] , rp.points[i], floorHeight, 300, 180));
			}
			else if (rp.windows.Contains(i)) {

				FVector tangent = rp.points[i] - rp.points[i - 1];
				float len = tangent.Size();
				tangent.Normalize();

				TArray<FPolygon> windows;
				int spaces = FMath::FloorToInt(windowDensity * len) + 1;
				float jumpLen = len / (float)spaces;

				for (int j = 1; j < spaces; j++) {
					FPolygon currWindow;
					FVector pw1 = rp.points[i - 1] + tangent * j * jumpLen + FVector(0, 0, 50 + windowHeight) - (tangent * windowWidth / 2);
					FVector pw2 = pw1 - FVector(0, 0, windowHeight);
					FVector pw3 = pw2 + tangent * windowWidth;
					FVector pw4 = pw3 + FVector(0, 0, windowHeight);

					currWindow.points.Add(pw1);
					currWindow.points.Add(pw2);
					currWindow.points.Add(pw3);
					currWindow.points.Add(pw4);
					//currWindow.type = PolygonType::window;
					windows.Add(currWindow);

				}
				TArray<FMaterialPolygon> pols = getSideWithHoles(newP, windows);
				for (FPolygon p : windows) {
					FMaterialPolygon win;
					win.points = p.points;
					win.type = PolygonType::window;
					toReturn.Add(win);
				}
				toReturn.Append(pols);

			}
			else {
				toReturn.Add(newP);
			}

		}

	}
	return toReturn;

}


static TArray<FMeshInfo> getMeetingRoom(FRoomPolygon &r2 , float height) {
	TArray<FMeshInfo> meshes;

	FVector dir = r2.getRoomDirection();
	FVector center = r2.getCenter();
	meshes.Add(FMeshInfo{"office_meeting_table", FTransform(dir.Rotation(), center + FVector(0, 0, 2), FVector(1.0, 1.0, 1.0))});
	float offsetLen = 100;
	for (int i = 1; i < 4; i+=2) {
		FRotator curr = FRotator(0, 90 * i, 0);
		FVector chairPos = curr.Vector() * offsetLen + center + FVector(0, 0, 2);
		meshes.Add(FMeshInfo{ "office_meeting_chair", FTransform(curr.GetInverse(), chairPos, FVector(1.0, 1.0, 1.0)) });
	}

	if (randFloat() < 0.5) {
		//FVector corner = FMath::FloorToInt(randFloat());
		// add book shelf in corner of room

		float bookShelfLen = 100;
		float bookShelfWidth = 30;
		TArray<int32> acceptablePlaces;

		for (int i = 1; i < r2.points.Num(); i++) {
			if (FVector::Dist(r2.points[i], r2.points[i-1]) > 1000)
				acceptablePlaces.Add(i);
		}
		for (int i : r2.windows) {
			acceptablePlaces.Remove(i);
		}
		for (int i : r2.toIgnore) {
			acceptablePlaces.Remove(i);
		}
		if (acceptablePlaces.Num() > 0) {
			int random = FMath::FloorToInt(randFloat() * acceptablePlaces.Num());
			if (random == acceptablePlaces.Num()) {
				random--;
			}
			int place = acceptablePlaces[random];
			FVector dir2 = (r2.points[place] - r2.points[place - 1]) / 2;
			dir2.Normalize();
			FVector dir3 = FRotator(0, 270, 0).RotateVector(dir2);
			dir3.Normalize();
			FRotator curr = dir3.Rotation();
			FVector pos = r2.points[place - 1] + dir2 * bookShelfLen * 2 + dir3 * bookShelfWidth * 2;
			meshes.Add(FMeshInfo{ "office_shelf", FTransform(curr, pos, FVector(1.0, 1.0, 1.0)) });

		}
	}

	if (randFloat() < 0.5) {
		// add whiteboard
		TArray<int32> acceptablePlaces;

		for (int i = 1; i < r2.points.Num(); i++) {
			if (FVector::Dist(r2.points[i], r2.points[i - 1]) > 1000)
				acceptablePlaces.Add(i);
		}
		for (int i : r2.entrances) {
			acceptablePlaces.Remove(i);
		}
		for (int i : r2.toIgnore) {
			acceptablePlaces.Remove(i);
		}
		if (acceptablePlaces.Num() > 0) {
			int random = FMath::FloorToInt(randFloat() * acceptablePlaces.Num());
			if (random == acceptablePlaces.Num()) {
				random--;
			}
			int place = acceptablePlaces[random];
			FVector middle = (r2.points[place] - r2.points[place - 1]) / 2 + r2.points[place - 1] + FVector(0, 0, 200);
			FVector dir2 = (r2.points[place] - r2.points[place - 1]);
			dir2.Normalize();
			FRotator curr = FRotator(0, 90, 0).RotateVector(dir2).Rotation();
			meshes.Add(FMeshInfo{ "office_whiteboard", FTransform(curr, middle + FRotator(0, 270, 0).RotateVector(dir2) * 35, FVector(1.0, 1.0, 1.0)) });

			r2.windows.Remove(place);
		}
	}


	return meshes;
}

static TArray<FMeshInfo> getWorkingRoom(FRoomPolygon r2, float height) {
	TArray<FMeshInfo> meshes;

	// build cubicles
	return meshes;
}

FRoomInfo ARoomBuilder::buildOffice(FRoomPolygon f, int floor, float height, float density, float windowHeight, float windowWidth) {

	float meetingRoomProb = 0.2;
	float workingRoomProb = 0.5;


	FRoomInfo r;
	TArray<FRoomPolygon> roomPols = f.refine(200, 0);
	for (FRoomPolygon &r2 : roomPols) {
		//r.meshes.Add(FMeshInfo{ "bazinga", r2.getCenter() + FVector(0, 0, beginning)});
		r.meshes.Add(FMeshInfo{ "office_lamp", FTransform(r2.getCenter() + FVector(0, 0, height - 45))});
		if (randFloat() < meetingRoomProb) {
			r.meshes.Append(getMeetingRoom(r2, height));
		}
		else if (randFloat() < workingRoomProb) {
			r.meshes.Append(getWorkingRoom(r2, height));
		}

	}
	r.pols.Append(interiorPlanToPolygons(roomPols, height, density, windowHeight, windowWidth));

	return r;
}


FRoomInfo ARoomBuilder::buildRoom(FRoomPolygon f, RoomType type, int floor, float height, float density, float windowHeight, float windowWidth) {
	if (!f.canRefine) {
		FRoomInfo r;
		//r.beginning = beginning;
		//r.height = height;
		TArray<FRoomPolygon> pols;
		pols.Add(f);
		r.pols = interiorPlanToPolygons(pols, height, 0, 0, 0);
		return r;
	}
	switch (type) {
	case RoomType::office: return buildOffice(f, floor, height, 0.005, 250, 150);
	}
	return FRoomInfo();
}
