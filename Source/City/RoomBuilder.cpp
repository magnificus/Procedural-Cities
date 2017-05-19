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


TArray<FMaterialPolygon> getEntranceSide(FVector p1, FVector p2, float floorHeight, float doorHeight, float doorWidth, FVector doorPos) {
	TArray<FMaterialPolygon> polygons;


	FVector side = p2 - p1;
	float sideLen = side.Size();
	side.Normalize();
	float distToDoor = FVector::Dist(doorPos, p1) - doorWidth/2;
	TArray<FPolygon> holes;
	FMaterialPolygon doorPolygon;
	doorPolygon.points.Add(p1 + side*distToDoor + FVector(0, 0, doorHeight));
	doorPolygon.points.Add(p1 + side*distToDoor);// + FVector(0, 0, 100));
	doorPolygon.points.Add(p1 + side*distToDoor + side*doorWidth);// + FVector(0, 0, 100));
	doorPolygon.points.Add(p1 + side*distToDoor + side*doorWidth + FVector(0, 0, doorHeight));
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
				toReturn.Append(getEntranceSide(rp.points[i - 1] , rp.points[i], floorHeight, 300, 180, rp.specificEntrances.Contains(i) ? rp.specificEntrances[i] : middle(rp.points[i-1], rp.points[i])));
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


static TArray<FMeshInfo> getMeetingRoom(FRoomPolygon &r2) {
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

static TArray<FMeshInfo> getWorkingRoom(FRoomPolygon r2) {
	TArray<FMeshInfo> meshes;

	// build cubicles
	return meshes;
}

RoomBlueprint getOfficeBlueprint(float areaScale) {
	// one meeting room, rest working rooms
	TArray<RoomSpecification> needed;
	RoomSpecification meetingRoom;
	meetingRoom.maxArea = 200 * areaScale;
	meetingRoom.minArea = 100 * areaScale;
	meetingRoom.type = SubRoomType::meeting;
	needed.Add(meetingRoom);

	RoomSpecification workRoom{ 100 * areaScale, 200 * areaScale, SubRoomType::work };
	TArray<RoomSpecification> optional;
	optional.Add(workRoom);

	return RoomBlueprint{ needed, optional };

}

RoomBlueprint getApartmentBlueprint(float areaScale) {
	TArray<RoomSpecification> needed;
	RoomSpecification kitchen{40*areaScale, 90*areaScale, SubRoomType::kitchen};
	RoomSpecification bathroom{30*areaScale, 60*areaScale, SubRoomType::bath };
	RoomSpecification bedroom{50*areaScale, 90*areaScale, SubRoomType::bed };
	RoomSpecification living{100 * areaScale, 150 * areaScale, SubRoomType::living };
	RoomSpecification closet{ 10 * areaScale, 40 * areaScale, SubRoomType::closet };

	needed.Add(bedroom);
	needed.Add(living);
	needed.Add(kitchen);
	needed.Add(bathroom);

	TArray<RoomSpecification> optional;
	optional.Add(bedroom);
	optional.Add(closet);
	optional.Add(bedroom);
	optional.Add(bathroom);
	optional.Add(living);

	return RoomBlueprint{ needed, optional };


}

FRoomInfo ARoomBuilder::buildOffice(FRoomPolygon &f, int floor, float height, float density, float windowHeight, float windowWidth) {

	FRoomInfo r;
	TArray<FRoomPolygon> roomPols = f.getRooms(getOfficeBlueprint(1.0f));
	for (FRoomPolygon &r2 : roomPols) {
		r.meshes.Add(FMeshInfo{ "office_lamp", FTransform(r2.getCenter() + FVector(0, 0, height - 45))});

		switch (r2.type) {
		case SubRoomType::meeting: r.meshes.Append(getMeetingRoom(r2));
			break;
		case SubRoomType::work: r.meshes.Append(getWorkingRoom(r2));
			break;
		}

	}
	r.pols.Append(interiorPlanToPolygons(roomPols, height, density, windowHeight, windowWidth));

	return r;
}


TArray<FPolygon> getBlockingVolumes(FRoomPolygon &r2, float entranceWidth, float blockingLength) {
	TArray<FPolygon> blocking;
	blocking.Add(r2);
	for (int i : r2.entrances) {
		//
		FPolygon entranceBlock;
		FVector inMiddle = middle(r2.points[i], r2.points[i - 1]);
		FVector tangent = r2.points[i] - r2.points[i - 1];
		tangent.Normalize();
		FVector altTangent = FRotator(0, 90, 0).RotateVector(tangent);
		entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 + altTangent*blockingLength);
		entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 - altTangent*blockingLength);
		entranceBlock.points.Add(inMiddle + tangent * entranceWidth*0.5 - altTangent*blockingLength);
		entranceBlock.points.Add(inMiddle + tangent * entranceWidth*0.5 + altTangent*blockingLength);
		entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 + altTangent*blockingLength);
		blocking.Add(entranceBlock);

	}

	for (int i : r2.toIgnore) {
		FPolygon entranceBlock;
		FVector tangent = r2.points[i] - r2.points[i - 1];
		tangent.Normalize();
		FVector altTangent = FRotator(0, 90, 0).RotateVector(tangent);
		entranceBlock.points.Add(r2.points[i-1] + altTangent*blockingLength);
		entranceBlock.points.Add(r2.points[i - 1] - altTangent*blockingLength);
		entranceBlock.points.Add(r2.points[i] - altTangent*blockingLength);
		entranceBlock.points.Add(r2.points[i] + altTangent*blockingLength);
		entranceBlock.points.Add(r2.points[i - 1] + altTangent*blockingLength);
		blocking.Add(entranceBlock);

	}

	return blocking;
}


static TArray<FMeshInfo> potentiallyGetTableAndChairs(FRoomPolygon &r2, TArray<FPolygon> &placed) {
	TArray<FMeshInfo> meshes;


	FVector center = r2.getCenter();
	FVector rot = r2.getRoomDirection();
	rot.Normalize();
	FVector tan = FRotator(0, 90, 0).RotateVector(rot);
	tan.Normalize();
	float extraChairHeight = 30;
	FMeshInfo table{ "large_table", FTransform(rot.Rotation() , center, FVector(1.0f, 1.0f, 1.0f)) };
	FVector c1 = center - rot * 70 + tan * 150;
	FVector c2 = center + rot * 70 + tan * 150;
	FVector c3 = center - rot * 70 - tan * 150;
	FVector c4 = center + rot * 70 - tan * 150;
	FMeshInfo chair1{ "chair", FTransform(rot.Rotation(), c1 + FVector(0, 0, extraChairHeight),  FVector(1.0f, 1.0f, 1.0f)) };
	FMeshInfo chair2{ "chair", FTransform(rot.Rotation(), c2 + FVector(0, 0, extraChairHeight),  FVector(1.0f, 1.0f, 1.0f)) };
	FMeshInfo chair3{ "chair", FTransform(FRotator(0, 180, 0) + rot.Rotation(), c3 + FVector(0, 0, extraChairHeight),  FVector(1.0f, 1.0f, 1.0f)) };
	FMeshInfo chair4{ "chair", FTransform(FRotator(0, 180, 0) + rot.Rotation(), c4 + FVector(0, 0, extraChairHeight),  FVector(1.0f, 1.0f, 1.0f)) };

	meshes.Add(table);
	meshes.Add(chair1);
	meshes.Add(chair2);
	meshes.Add(chair3);
	meshes.Add(chair4);

	return meshes;

}

static TArray<FMeshInfo> getLivingRoom(FRoomPolygon &r2) {
	TArray<FMeshInfo> meshes;
	
	TArray<FPolygon> placed;
	placed.Add(r2);
	meshes.Append(potentiallyGetTableAndChairs(r2, placed));
	// add maybe sofa and stuff? lamps?
	return meshes;
}

static TArray<FMeshInfo> getBathRoom(FRoomPolygon &r2) {
	TArray<FMeshInfo> meshes;
	
	r2.windows.Empty();
	//r2.entrances.Empty();
	//r2.toIgnore.Empty();
	TArray<FPolygon> placed;
	placed.Append(getBlockingVolumes(r2, 200, 100));
	for (int i = 1; i < r2.points.Num(); i++) {
		if (r2.entrances.Contains(i) || r2.toIgnore.Contains(i)) {
			continue;
		}
		int place = i;
		FVector dir = getNormal(r2.points[place], r2.points[place - 1], true);
		FVector tangent = r2.points[place] - r2.points[place - 1];
		tangent.Normalize();
		dir.Normalize();
		FVector origin = middle(r2.points[place - 1], r2.points[place]);
		FVector pos = origin + dir * 90;
		FRotator rot = dir.Rotation();
		FMeshInfo toilet{ "toilet", FTransform(rot, pos + FVector(0, 10, 0), FVector(1.0f, 1.0f, 1.0f)) };
		meshes.Add(toilet);
		break;
	}
	return meshes;
}

bool attemptPlaceShelf(FRoomPolygon &r2, TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes, float offset, bool shallContinue) {
	bool found = false;
	for (int i = 1; i < r2.points.Num(); i++) {
		if (r2.windows.Contains(i) || r2.entrances.Contains(i) || r2.toIgnore.Contains(i)) {
			continue;
		}
		int place = i;
		FVector dir = getNormal(r2.points[place], r2.points[place - 1], true);
		FVector tangent = r2.points[place] - r2.points[place - 1];
		tangent.Normalize();
		dir.Normalize();
		FVector origin = r2.points[place - 1] + tangent * offset;
		FVector pos = origin + dir * 10;
		FRotator rot = dir.Rotation();
		FPolygon shelfP = MeshPolygonReference::getShelfPolygon(pos, rot);
		
		if (!testCollision(shelfP, placed, 0)) {
			placed.Add(shelfP);
			FMeshInfo shelf{ "shelf", FTransform(rot , pos, FVector(1.0f, 1.0f, 1.0f))};
			meshes.Add(shelf);
			found = true;
			if (!shallContinue)
				return true;
		}

	}
	return found;
}

static TArray<FMeshInfo> getBedRoom(FRoomPolygon &r2) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;
	placed.Add(r2);
	placed.Append(getBlockingVolumes(r2, 200, 200));
	//for (FPolygon p : placed) {
	//	for (FVector f : p.points) {
	//		meshes.Add(FMeshInfo{ "visualizer", FTransform(f) });
	//	}
	//}

	for (int i = 1; i < r2.points.Num(); i++) {
		if (r2.entrances.Contains(i) || r2.toIgnore.Contains(i)) {
			continue;
		}
		int place = i;
		FVector dir = getNormal(r2.points[place], r2.points[place - 1], true);
		FVector tangent = r2.points[place] - r2.points[place - 1];
		tangent.Normalize();
		dir.Normalize();
		FVector origin = r2.points[place - 1] + tangent * 120;
		FVector pos = origin + dir * 180;
		FRotator rot = dir.Rotation();
		FPolygon bedP = MeshPolygonReference::getBedPolygon(pos, rot);
		placed.Add(bedP);
		FMeshInfo bed{ "bed", FTransform(rot + FRotator(0, 270, 0), pos + FVector(0, 0, 50), FVector(1.0f, 1.0f, 1.0f)) };
		meshes.Add(bed);
		pos += tangent * 150 - dir * 70;
		FPolygon smallTableP = MeshPolygonReference::getSmallTablePolygon(pos, rot);
		placed.Add(smallTableP);
		FMeshInfo table{ "small_table", FTransform(rot , pos - FVector(0, 0, 50), FVector(1.0f, 1.0f, 1.0f)) };
		meshes.Add(table);
		break;
	}

	for (int i = 10; i < 200; i += 100) {
		if (attemptPlaceShelf(r2, placed, meshes, i, false))
			break;
	}
	return meshes;
}

FRoomInfo ARoomBuilder::buildApartment(FRoomPolygon &f, int floor, float height, float density, float windowHeight, float windowWidth) {

	FRoomInfo r;
	TArray<FRoomPolygon> roomPols = f.getRooms(getApartmentBlueprint(1.0f));


	for (FRoomPolygon &r2 : roomPols) {
		for (int i : r2.entrances) {
			FVector doorPos = r2.specificEntrances.Contains(i) ? r2.specificEntrances[i] : middle(r2.points[i], r2.points[i - 1]);
			FVector dir1 = getNormal(r2.points[i], r2.points[i - 1], true);
			dir1.Normalize();
			FVector dir2 = r2.points[i] - r2.points[i - 1];
			dir2.Normalize();
			r.meshes.Add(FMeshInfo{ "door", FTransform(dir1.Rotation(), doorPos + FVector(0, 0, 0) - dir2 * 90 + dir1 * 75 , FVector(1.0f, 1.0f, 1.0f)) });
		}

		r.meshes.Add(FMeshInfo{ "apartment_lamp", FTransform(r2.getCenter() + FVector(0, 0, height - 45)) });
		switch (r2.type) {
		case SubRoomType::living: r.meshes.Append(getLivingRoom(r2));
			break;
		case SubRoomType::bed: r.meshes.Append(getBedRoom(r2));
			break;
		case SubRoomType::closet:
			r.meshes.Add(FMeshInfo{ "room_closet", FTransform(r2.getCenter()) });
			break;
		case SubRoomType::corridor:
			r.meshes.Add(FMeshInfo{ "room_corridor", FTransform(r2.getCenter()) });
			break;
		case SubRoomType::kitchen:
			r.meshes.Add(FMeshInfo{ "room_kitchen", FTransform(r2.getCenter()) });
			break;
		case SubRoomType::bath: r.meshes.Append(getBathRoom(r2));
			r.meshes.Add(FMeshInfo{ "room_bathroom", FTransform(r2.getCenter()) });
			break;
		case SubRoomType::hallway: 	r.meshes.Add(FMeshInfo{ "room_hallway", FTransform(r2.getCenter()) });
			break;

		}


	}
	r.pols = interiorPlanToPolygons(roomPols, height, density, windowHeight, windowWidth);

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
	case RoomType::apartment: return buildApartment(f, floor, height, 0.002, 200, 200);
	}
	return FRoomInfo();
}
