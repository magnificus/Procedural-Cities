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


TArray<FMaterialPolygon> ARoomBuilder::getSideWithHoles(FMaterialPolygon outer, TArray<FPolygon> holes, PolygonType type) {

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
			p1.type = type;

			p1.points.Add((p.points[3] - outer.points[0]).ProjectOnTo(tangentUp) + outer.points[0]);

			FMaterialPolygon p2 = FMaterialPolygon();
			p2.type = type;
			p2.points.Add(attach1);
			p2.points.Add(attach2);
			p2.points.Add(p.points[3]);

			FMaterialPolygon p3 = FMaterialPolygon();
			p3.type = type;
			p3.points.Add(attach2);
			p3.points.Add(p.points[1]);
			p3.points.Add(p.points[0]);

			FMaterialPolygon p4 = FMaterialPolygon();
			p4.type = type;
			p4.points.Add(attach2);
			p4.points.Add(attach3);
			p4.points.Add(p.points[1]);

			FMaterialPolygon p5 = FMaterialPolygon();
			p5.type = type;
			p5.points.Add(attach3);
			p5.points.Add((p.points[2] - outer.points[1]).ProjectOnTo(tangentDown) + outer.points[1]);
			p5.points.Add(p.points[2]);

			FMaterialPolygon p6 = FMaterialPolygon();
			p6.type = type;
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
		p7.type = type;
		p7.points.Add(attach1);
		p7.points.Add(attach4);
		p7.points.Add(outer.points[2]);

		FMaterialPolygon p8;
		p8.type = type;
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

TArray<FPolygon> getBlockingVolumes(FRoomPolygon *r2, float entranceWidth, float blockingLength) {
	TArray<FPolygon> blocking;
	for (int i : r2->entrances) {
		//
		FPolygon entranceBlock;

		FVector inMiddle = r2->specificEntrances.Contains(i) ? r2->specificEntrances[i] : middle(r2->points[i], r2->points[i - 1]);
		FVector tangent = r2->points[i] - r2->points[i - 1];
		tangent.Normalize();
		FVector altTangent = FRotator(0, 90, 0).RotateVector(tangent);
		entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 + altTangent*blockingLength);
		entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 - altTangent*blockingLength);
		entranceBlock.points.Add(inMiddle + tangent * entranceWidth*0.5 - altTangent*blockingLength);
		entranceBlock.points.Add(inMiddle + tangent * entranceWidth*0.5 + altTangent*blockingLength);
		entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 + altTangent*blockingLength);
		blocking.Add(entranceBlock);

	}

	for (auto &pair : r2->passiveConnections) {
		for (FRoomPolygon *p : pair.Value) {
			FPolygon entranceBlock;
			int32 num = p->activeConnections[r2];
			FVector inMiddle = p->specificEntrances.Contains(num) ? p->specificEntrances[num] : middle(p->points[num], p->points[num - 1]);
			FVector tangent = p->points[num] - p->points[num - 1];
			tangent.Normalize();
			FVector altTangent = FRotator(0, 90, 0).RotateVector(tangent);
			entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 + altTangent*blockingLength);
			entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 - altTangent*blockingLength);
			entranceBlock.points.Add(inMiddle + tangent * entranceWidth*0.5 - altTangent*blockingLength);
			entranceBlock.points.Add(inMiddle + tangent * entranceWidth*0.5 + altTangent*blockingLength);
			entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 + altTangent*blockingLength);
			blocking.Add(entranceBlock);
		}

	}



	return blocking;
}


FPolygon getPolygon(FRotator rot, FVector pos, FString name, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map) {
	FVector min;
	FVector max;
	FPolygon pol;

	if (map.Contains(name))
		map[name]->GetLocalBounds(min, max);
	else
		return pol;
	//UE_LOG(LogTemp, Warning, TEXT("box: %s, %s"), *min.ToString(), *max.ToString());

	pol.points.Add(rot.RotateVector(FVector(min.X, min.Y, 0.0f)) + pos);
	pol.points.Add(rot.RotateVector(FVector(max.X, min.Y, 0.0f)) + pos);
	pol.points.Add(rot.RotateVector(FVector(max.X, max.Y, 0.0f)) + pos);
	pol.points.Add(rot.RotateVector(FVector(min.X, max.Y, 0.0f)) + pos);
	pol.points.Add(rot.RotateVector(FVector(min.X, min.Y, 0.0f)) + pos);
	return pol;
}

TArray<FMaterialPolygon> getEntranceSide(FVector p1, FVector p2, float floorHeight, float doorHeight, float doorWidth, FVector doorPos, PolygonType type) {
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
	polygons.Append(ARoomBuilder::getSideWithHoles(outer, holes, type));

	return polygons;
}


TArray<FMaterialPolygon> ARoomBuilder::interiorPlanToPolygons(TArray<FRoomPolygon*> roomPols, float floorHeight, float windowDensity, float windowHeight, float windowWidth) {
	TArray<FMaterialPolygon> toReturn;

	for (FRoomPolygon *rp : roomPols) {
		for (int i = 1; i < rp->points.Num(); i++) {
			if (rp->toIgnore.Contains(i)) {
				continue;
			}
			FMaterialPolygon newP;
			newP.type = rp->exteriorWalls.Contains(i) ? PolygonType::exterior : PolygonType::interior;
			FVector p1 = rp->points[i - 1];
			FVector p2 = rp->points[i];
			newP.points.Add(p1 + FVector(0, 0, floorHeight));
			newP.points.Add(p1 + FVector(0, 0, 0));
			newP.points.Add(p2 + FVector(0, 0, 0));
			newP.points.Add(p2 + FVector(0, 0, floorHeight));
			newP.points.Add(p1 + FVector(0, 0, floorHeight));

			if (rp->entrances.Contains(i)) {
				toReturn.Append(getEntranceSide(rp->points[i - 1] , rp->points[i], floorHeight, 297, 137, rp->specificEntrances.Contains(i) ? rp->specificEntrances[i] : middle(rp->points[i-1], rp->points[i]), rp->exteriorWalls.Contains(i) ? PolygonType::exterior : PolygonType::interior));
			}
			else if (rp->windows.Contains(i)) {

				FVector tangent = rp->points[i] - rp->points[i - 1];
				float len = tangent.Size();
				tangent.Normalize();

				TArray<FPolygon> windows;
				int spaces = FMath::FloorToInt(windowDensity * len) + 1;
				float jumpLen = len / (float)spaces;

				for (int j = 1; j < spaces; j++) {
					FPolygon currWindow;
					FVector pw1 = rp->points[i - 1] + tangent * j * jumpLen + FVector(0, 0, 50 + windowHeight) - (tangent * windowWidth / 2);
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
				TArray<FMaterialPolygon> pols = getSideWithHoles(newP, windows, rp->exteriorWalls.Contains(i) ? PolygonType::exterior : PolygonType::interior);
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

bool attemptPlaceOnTop(FPolygon &p, TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes) {
	return false;
}


FTransform attemptGetPosition(FRoomPolygon *r2, TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes, float verticalOffset, bool windowAllowed, int testsPerSide, FString string, FRotator offsetRot, FVector offsetPos, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map, bool onWall) {
	for (int i = 1; i < r2->points.Num(); i++) {
		if (r2->windows.Contains(i) && !windowAllowed) {
			continue;
		}
		int place = i;
		for (int j = 0; j < testsPerSide; j++) {
			FVector dir = getNormal(r2->points[place], r2->points[place - 1], true);
			FVector tangent = r2->points[place] - r2->points[place - 1];
			float sideLen = tangent.Size();
			tangent.Normalize();
			dir.Normalize();
			FVector origin = r2->points[place - 1] + tangent * (FMath::FRand() * (sideLen - 100.0f) + 100.0f);
			FVector pos = origin + dir * verticalOffset + offsetPos;
			FRotator rot = dir.Rotation() + offsetRot;

			FPolygon pol = getPolygon(rot, pos, string, map);
			if (onWall) {
				// at least two points has to be next to the wall
				int count = 0;
				for (int k = 1; k < pol.points.Num(); k++) {
					FVector curr = NearestPointOnLine(r2->points[i - 1], r2->points[i] - r2->points[i - 1], pol.points[k]);
					if (FVector::DistSquared(curr, pol.points[k]) < 2500) { // pick a number that seems reasonable
						count++;
					}
				}
				if (count < 2) {
					continue;
				}
			}

			if (!testCollision(pol, placed, 0, *r2)) {
				return FTransform(rot, pos, FVector(1.0f, 1.0f, 1.0f));
			}
		}


	}
	return FTransform(FRotator(0,0,0), FVector(0,0,0), FVector(0, 0, 0));
}

bool attemptPlace(FRoomPolygon *r2, TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes, float verticalOffset, bool windowAllowed, int testsPerSide, FString string, FRotator offsetRot,FVector offsetPos, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map, bool onWall) {
	FTransform pos = attemptGetPosition(r2, placed, meshes, verticalOffset, windowAllowed, testsPerSide, string, offsetRot, offsetPos, map, onWall);
	if (pos.GetLocation().X != 0.0f) {
		FPolygon pol = getPolygon(pos.Rotator(), pos.GetLocation(), string, map);
		placed.Add(pol);
		meshes.Add(FMeshInfo{string, pos});
		return true;
	}
	return false;
}


void placeRows(FRoomPolygon *r2, TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes, FRotator offsetRot, FString name, float vertDens, float horDens, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map) {
	int place = 1;
	// get a line through the room
	SplitStruct res = r2->getSplitProposal(false, 0.5);
	FVector origin = r2->points[res.min - 1];
	FVector normal = res.p2 - res.p1;
	normal.Normalize();
	FVector tangent = r2->points[res.min] - r2->points[res.min - 1];
	tangent.Normalize(); 
	float width = FVector::Dist(r2->points[res.min], r2->points[res.min - 1]);
	float height = FVector::Dist(res.p1, res.p2);

	int numWidth = FMath::FloorToInt(width * horDens) + 1;
	int numHeight = FMath::FloorToInt(height * vertDens) + 1;

	float intervalWidth = width / numWidth;
	float intervalHeight = height / numHeight;
	for (int i = 1; i < numWidth; i++) {
		for (int j = 1; j < numHeight; j++) {
			FPolygon pol = getPolygon(normal.Rotation(), origin + i*intervalWidth*tangent + j*intervalHeight*normal, name, map);
			//if (!testCollision(pol, placed, 0, *r2)) {
			placed.Add(pol);
			meshes.Add(FMeshInfo{ name, FTransform(normal.Rotation(),origin + i*intervalWidth*tangent + j*intervalHeight*normal, FVector(1.0f, 1.0f, 1.0f)) });
			//s}
		}
	}
	//FVector start =
}



static TArray<FMeshInfo> getMeetingRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed = getBlockingVolumes(r2, 200, 200);
	FVector dir = r2->getRoomDirection();
	FVector center = r2->getCenter();
	meshes.Add(FMeshInfo{"office_meeting_table", FTransform(dir.Rotation(), center + FVector(0, 0, 2), FVector(1.0, 1.0, 1.0))});
	float offsetLen = 100;
	for (int i = 1; i < 4; i+=2) {
		FRotator curr = FRotator(0, 90 * i, 0);
		FVector chairPos = curr.Vector() * offsetLen + center + FVector(0, 0, 2);
		meshes.Add(FMeshInfo{ "office_meeting_chair", FTransform(curr.GetInverse(), chairPos, FVector(1.0, 1.0, 1.0)) });
	}

	if (randFloat() < 0.5) {
		attemptPlace(r2, placed, meshes, 50.0f, false, 2, "shelf", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	}

	if (randFloat() < 0.5) {
		// add whiteboard
		attemptPlace(r2, placed, meshes, 35.0f, false, 1, "office_whiteboard", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	}

	attemptPlace(r2, placed, meshes, 50.0f, true, 2, "dispenser", FRotator(0, 0, 0), FVector(0, 0, 0), map, false);

	return meshes;
}

static TArray<FMeshInfo> getWorkingRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;
	placed = getBlockingVolumes(r2, 200, 200);
	placeRows(r2, placed, meshes, FRotator(0, 180, 0), "office_table_position", 0.004, 0.004, map);
	//int target = 1;
	//FVector start = middle(r2->points[target], r2->points[target - 1]);
	//FVector end = 
	//FVector end = ;
	//for (int i = 0; i < )
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
	RoomSpecification bathroom{ 30 * areaScale, 60 * areaScale, SubRoomType::bath };
	needed.Add(meetingRoom);
	//needed.Add(bathroom);

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





static TArray<FMeshInfo> potentiallyGetTableAndChairs(FRoomPolygon *r2, TArray<FPolygon> &placed, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;


	FVector center = r2->getCenter();
	FVector rot = r2->getRoomDirection();
	rot.Normalize();
	FVector tan = FRotator(0, 90, 0).RotateVector(rot);
	tan.Normalize();
	float extraChairHeight = 30;
	FMeshInfo table{ "large_table", FTransform(rot.Rotation() , center, FVector(1.0f, 1.0f, 1.0f)) };
	FVector c1 = center - rot * 70 + tan * 150;
	FVector c2 = center + rot * 70 + tan * 150;
	FVector c3 = center - rot * 70 - tan * 150;
	FVector c4 = center + rot * 70 - tan * 150;
	FPolygon c1P = getPolygon(rot.Rotation(), c1 + FVector(0, 0, extraChairHeight), "chair", map);
	FPolygon c2P = getPolygon(rot.Rotation(), c2 + FVector(0, 0, extraChairHeight), "chair", map);
	FPolygon c3P = getPolygon(FRotator(0, 180, 0) + rot.Rotation(), c3 + FVector(0, 0, extraChairHeight), "chair", map);
	FPolygon c4P = getPolygon(FRotator(0, 180, 0) + rot.Rotation(), c4 + FVector(0, 0, extraChairHeight), "chair", map);
	FPolygon tableP = getPolygon(rot.Rotation(), center, "large_table", map);
	
	if (!testCollision(tableP, placed, 0, *r2)) {
		meshes.Add(table);
	}
	else {
		return meshes;
	}
	if (!testCollision(c1P, placed, 0, *r2)) {
		meshes.Add({"chair", FTransform(rot.Rotation(), c1 + FVector(0, 0, extraChairHeight),  FVector(1.0f, 1.0f, 1.0f)) });
	}
	if (!testCollision(c2P, placed, 0, *r2)) {
		meshes.Add({ "chair", FTransform(rot.Rotation(), c2 + FVector(0, 0, extraChairHeight),  FVector(1.0f, 1.0f, 1.0f)) });
	}
	if (!testCollision(c3P, placed, 0, *r2)) {
		meshes.Add({ "chair", FTransform(rot.Rotation() + FRotator(0, 180, 0), c3 + FVector(0, 0, extraChairHeight),  FVector(1.0f, 1.0f, 1.0f)) });
	}
	if (!testCollision(c4P, placed, 0, *r2)) {
		meshes.Add({ "chair", FTransform(rot.Rotation() + FRotator(0, 180, 0), c4 + FVector(0, 0, extraChairHeight),  FVector(1.0f, 1.0f, 1.0f)) });
	}

	return meshes;

}

static TArray<FMeshInfo> getLivingRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	
	TArray<FPolygon> placed;
	placed.Append(getBlockingVolumes(r2, 200, 200));

	// add maybe sofa and stuff? lamps?
	return meshes;
}

static TArray<FMeshInfo> getBathRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	
	r2->windows.Empty();
	TArray<FPolygon> placed;

	TArray<FPolygon> blocking = getBlockingVolumes(r2, 200, 100);
	//for (FPolygon p : blocking) {
	//	for (FVector f : p.points) {
	//		meshes.Add(FMeshInfo{ "visualizer", FTransform(f) });
	//	}
	//}
	placed.Append(blocking);
	attemptPlace(r2, placed, meshes, 80, false, 5, "toilet" , FRotator(0, 270, 0), FVector(0, 0, 0), map, false);
	FTransform res = attemptGetPosition(r2, placed, meshes, 80, false, 5, "sink", FRotator(0, 180, 0), FVector(0, 0, 0), map, false);
	if (res.GetLocation().X != 0.0f) {
		FPolygon pol = getPolygon(res.Rotator(), res.GetLocation(), "sink", map);
		placed.Add(pol);
		meshes.Add(FMeshInfo{ "sink", res});
		res.SetLocation(res.GetLocation() + FVector(0, 0, 150));
		//res.Ro
		meshes.Add(FMeshInfo{ "mirror", FTransform(res.Rotator() + FRotator(0, 270, 0), res.GetLocation() + FVector(0, 0, 55) + res.Rotator().Vector() * 50, FVector(1.0, 1.0, 1.0)) });
	}
	//attemptPlace(r2, placed, meshes, 80, false, 5, "sink", FRotator(0, 180, 0), FVector(0, 0, 0), map, false);
	return meshes;
}


static TArray<FMeshInfo> getBedRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;
	//placed.Add(r2);
	placed.Append(getBlockingVolumes(r2, 200, 200));

	for (int i = 1; i < r2->points.Num(); i++) {
		if (r2->entrances.Contains(i) || r2->toIgnore.Contains(i)) {
			continue;
		}
		int place = i;
		FVector dir = getNormal(r2->points[place], r2->points[place - 1], true);
		FVector tangent = r2->points[place] - r2->points[place - 1];
		tangent.Normalize();
		dir.Normalize();
		FVector origin = r2->points[place - 1] + tangent * 120;
		FVector pos = origin + dir * 180;
		FRotator rot = dir.Rotation();
		FPolygon bedP = getPolygon(rot, pos, "bed", map);
		//if (testCollision(bedP, placed, 0)) {
		//	continue;
		//}
		placed.Add(bedP);
		FMeshInfo bed{ "bed", FTransform(rot + FRotator(0, 270, 0), pos + FVector(0, 0, 50), FVector(1.0f, 1.0f, 1.0f)) };
		meshes.Add(bed);
		pos += tangent * 150 - dir * 70;
		//FPolygon smallTableP = MeshPolygonReference::getSmallTablePolygon(pos, rot);
		//if (!testCollision(smallTableP, placed, 0, *r2)) {
		//	placed.Add(smallTableP);
		//	FMeshInfo table{ "small_table", FTransform(rot , pos - FVector(0, 0, 50), FVector(1.0f, 1.0f, 1.0f)) };
		//	meshes.Add(table);
		//}
		break;
	}
	attemptPlace(r2, placed, meshes, 100, true, 2, "small_table", FRotator(0, 0, 0), FVector(0, 0, -50), map, false);
	attemptPlace(r2, placed, meshes, 50.0f, false, 2, "shelf", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	attemptPlace(r2, placed, meshes, 50, false, 2, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);

	return meshes;
}

static TArray<FMeshInfo> getHallWay(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	attemptPlace(r2, placed, meshes, 70, true, 1, "hanger", FRotator(0, 0, 0), FVector(0, 0, 0), map, false);
	return meshes;
}

static TArray<FMeshInfo> getKitchen(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	attemptPlace(r2, placed, meshes, 50, false, 3, "kitchen", FRotator(0, 90, 0), FVector(0, 0, 0), map, true);
	meshes.Append(potentiallyGetTableAndChairs(r2, placed, map));
	attemptPlace(r2, placed, meshes, 45.0f, false, 1, "shelf_upper_large", FRotator(0, 270, 0), FVector(0, 0, 200), map, true);
	attemptPlace(r2, placed, meshes, 80, false, 3, "fridge", FRotator(0, 90, 0), FVector(0, 0, 0), map, true);
	attemptPlace(r2, placed, meshes, 85, false, 3, "oven", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	return meshes;
}

static TArray<FMeshInfo> getCorridor(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	attemptPlace(r2, placed, meshes, 50, false, 5, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);

	return meshes;
}


static TArray<FMeshInfo> getCloset(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	attemptPlace(r2, placed, meshes, 50, false, 5, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);
	attemptPlace(r2, placed, meshes, 45.0f, false, 1, "shelf_upper_large", FRotator(0, 270, 0), FVector(0, 0, 200), map, true);


	return meshes;
}

FRoomInfo placeBalcony(FRoomPolygon *p, int place, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;

	float width = 400;
	float length = 200;
	float height = 150;

	FVector tangent = p->points[place] - p->points[place - 1];
	FVector normal = getNormal(p->points[place], p->points[place - 1], false);
	normal.Normalize();
	float len = tangent.Size();
	tangent.Normalize();
	FVector start = p->points[place - 1] + tangent*(len - width) * 0.5;
	FVector end = p->points[place - 1] + tangent*(len + width) * 0.5;
	FVector endOut = end + normal*length;
	FVector startOut = start + normal*length;

	FMaterialPolygon floor;
	floor.type = PolygonType::exterior;
	floor.points.Add(start);
	floor.points.Add(end);
	floor.points.Add(endOut);
	floor.points.Add(startOut);
	floor.points.Add(start);

	r.pols.Add(floor);


	FMaterialPolygon side1;
	side1.type = PolygonType::exterior;
	FMaterialPolygon side2;
	side2.type = PolygonType::exterior;
	FMaterialPolygon side3;
	side3.type = PolygonType::exterior;

	side1.points.Add(start);
	side1.points.Add(startOut);
	side1.points.Add(startOut + FVector(0, 0, height));
	side1.points.Add(start + FVector(0, 0, height));

	side2.points.Add(startOut);
	side2.points.Add(endOut);
	side2.points.Add(endOut + FVector(0, 0, height));
	side2.points.Add(startOut + FVector(0, 0, height));

	side3.points.Add(endOut);
	side3.points.Add(end);
	side3.points.Add(end + FVector(0, 0, height));
	side3.points.Add(endOut + FVector(0, 0, height));

	r.pols.Add(side1);
	r.pols.Add(side2);
	r.pols.Add(side3);
	

	return r;
}

FRoomInfo ARoomBuilder::buildOffice(FRoomPolygon *f, int floor, float height, float density, float windowHeight, float windowWidth, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {

	FRoomInfo r;
	TArray<FRoomPolygon*> roomPols = f->getRooms(getOfficeBlueprint(1.0f));
	for (FRoomPolygon *r2 : roomPols) {
		r.meshes.Add(FMeshInfo{ "office_lamp", FTransform(r2->getCenter() + FVector(0, 0, height - 45)) });
		for (int i : r2->entrances) {
			FVector doorPos = r2->specificEntrances.Contains(i) ? r2->specificEntrances[i] : middle(r2->points[i], r2->points[i - 1]);
			FVector dir1 = getNormal(r2->points[i], r2->points[i - 1], true);
			dir1.Normalize();
			FVector dir2 = r2->points[i] - r2->points[i - 1];
			dir2.Normalize();
			r.meshes.Add(FMeshInfo{ "door_frame", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });
			r.meshes.Add(FMeshInfo{ "door", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });
		}
		switch (r2->type) {
		case SubRoomType::meeting: r.meshes.Append(getMeetingRoom(r2, map));
			break;
		case SubRoomType::work: r.meshes.Append(getWorkingRoom(r2, map));
			break;
		case SubRoomType::bath: r.meshes.Append(getBathRoom(r2, map));
			break;
		}

	}
	r.pols.Append(interiorPlanToPolygons(roomPols, height, density, windowHeight, windowWidth));

	return r;
}

FRoomInfo ARoomBuilder::buildApartment(FRoomPolygon *f, int floor, float height, float density, float windowHeight, float windowWidth, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool balcony) {

	FRoomInfo r;
	TArray<FRoomPolygon*> roomPols = f->getRooms(getApartmentBlueprint(1.0f));
	
	if (balcony) {
		for (FRoomPolygon *p : roomPols) {
			if (splitableType(p->type)) {
				// these are the balcony candidates
				if (p->windows.Num() > 0) {
					// this is the place
					int place = *p->windows.CreateIterator();
					p->entrances.Add(place);
					FVector mid = middle(p->points[place], p->points[place - 1]);
					p->specificEntrances.Add(place, mid);


					r = placeBalcony(p, place, map);
					break;
				}
			}
		}
	}


	for (FRoomPolygon* r2 : roomPols) {
		for (int i : r2->entrances) {
			FVector doorPos = r2->specificEntrances.Contains(i) ? r2->specificEntrances[i] : middle(r2->points[i], r2->points[i - 1]);
			FVector dir1 = getNormal(r2->points[i], r2->points[i - 1], true);
			dir1.Normalize();
			FVector dir2 = r2->points[i] - r2->points[i - 1];
			dir2.Normalize();
			r.meshes.Add(FMeshInfo{ "door_frame", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });
			r.meshes.Add(FMeshInfo{ "door", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });
		}

		r.meshes.Add(FMeshInfo{ "apartment_lamp", FTransform(r2->getCenter() + FVector(0, 0, height - 45)) });
		switch (r2->type) {
		case SubRoomType::living: r.meshes.Append(getLivingRoom(r2, map));
			r.meshes.Add(FMeshInfo{ "room_living", FTransform(r2->getCenter()) });
			break;
		case SubRoomType::bed: r.meshes.Append(getBedRoom(r2, map));
			r.meshes.Add(FMeshInfo{ "room_bed", FTransform(r2->getCenter()) });
			break;
		case SubRoomType::closet: r.meshes.Append(getCloset(r2, map));
			r.meshes.Add(FMeshInfo{ "room_closet", FTransform(r2->getCenter()) });
			break;
		case SubRoomType::corridor:
			r.meshes.Append(getCorridor(r2, map));
			r.meshes.Add(FMeshInfo{ "room_corridor", FTransform(r2->getCenter()) });
			break;
		case SubRoomType::kitchen:
			r.meshes.Append(getKitchen(r2, map));
			r.meshes.Add(FMeshInfo{ "room_kitchen", FTransform(r2->getCenter()) });
			break;
		case SubRoomType::bath: r.meshes.Append(getBathRoom(r2, map));
			r.meshes.Add(FMeshInfo{ "room_bathroom", FTransform(r2->getCenter()) });
			break;
		case SubRoomType::hallway: 	r.meshes.Append(getHallWay(r2, map));
			r.meshes.Add(FMeshInfo{ "room_hallway", FTransform(r2->getCenter()) });
			break;

		}


	}

	//TArray<FRoomPolygon> toReturn;

	//for (FRoomPolygon *p : roomPols) {
	//	toReturn.Add(*p);
	//}


	r.pols.Append(interiorPlanToPolygons(roomPols, height, density, windowHeight, windowWidth));

	for (FRoomPolygon *p : roomPols) {
		delete p;
	}

	return r;
}


FRoomInfo ARoomBuilder::buildRoom(FRoomPolygon *f, RoomType type, int floor, float height, float density, float windowHeight, float windowWidth, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	if (!f->canRefine) {
		FRoomInfo r;
		//r.beginning = beginning;
		//r.height = height;
		TArray<FRoomPolygon*> pols;
		pols.Add(f);
		r.pols = interiorPlanToPolygons(pols, height, 0, 0, 0);
		return r;
	}
	switch (type) {
	case RoomType::office: return buildOffice(f, floor, height, 0.005, 270, 170, map);
	case RoomType::apartment: return buildApartment(f, floor, height, 0.002, 200, 200, map, floor != 0);
	}
	return FRoomInfo();
}
