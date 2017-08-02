// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "polypartition.h"
//#include "gpc.h"
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
TArray<FMaterialPolygon> ARoomBuilder::getSideWithHoles(FPolygon outer, TArray<FPolygon> holes, PolygonType type) {

	TArray<FMaterialPolygon> polygons;
	//if (outer.points.Num() < 3)
	//	return polygons;
	FVector e1 = outer.points[1] - outer.points[0];
	e1.Normalize();
	FVector n = /*outer.normal.Size() < 1.0f*/ true ? FVector::CrossProduct(e1, outer.points[outer.points.Num()-1] - outer.points[0]) : outer.normal;
	n.Normalize();
	FVector e2 = FVector::CrossProduct(e1, n);
	e2.Normalize();

	FVector origin = outer.points[0];
	TArray<FVector> allPoints;
	int current = 0;
	std::list<TPPLPoly> inPolys;
	std::list<TPPLPoly> outPolys;
	TPPLPoly outerP;
	outerP.SetHole(false);
	outerP.Init(outer.points.Num());

	for (int i = 0; i < outer.points.Num(); i++) {
		FVector point = outer.points[i];
		float y = FVector::DotProduct(e1, point - origin);
		float x = FVector::DotProduct(e2, point - origin);
		outerP[i] = TPPLPoint{ x, y, current++ };
		allPoints.Add(point);
	}
	outerP.SetOrientation(TPPL_CCW);
	inPolys.push_back(outerP);
	for (FPolygon p : holes) {
		TPPLPoly holeP;
		holeP.Init(p.points.Num());
		holeP.SetHole(true);
		for (int i = 0; i < p.points.Num(); i++) {
			FVector point = p.points[i];
			float y = FVector::DotProduct(e1, point - origin);
			float x = FVector::DotProduct(e2, point - origin);
			holeP[p.points.Num() - 1 - i] = TPPLPoint{ x, y, current++ };
			allPoints.Add(point);
		}
		holeP.SetOrientation(TPPL_CW);
		inPolys.push_back(holeP);
	}

	TPPLPartition part;
	part.RemoveHoles(&inPolys, &outPolys);
	for (TPPLPoly t : outPolys) {
		FMaterialPolygon newP;
		for (int i = 0; i < t.GetNumPoints(); i++) {
			newP.points.Add(allPoints[t[i].id]);
		}
		newP.type = type;
		polygons.Add(newP);
		//newP.
	}
	return polygons;
}


TArray<FPolygon> getBlockingVolumes(FRoomPolygon *r2, float entranceWidth, float blockingLength) {
	TArray<FPolygon> blocking = getBlockingEntrances(r2->points, r2->entrances, r2->specificEntrances, entranceWidth, blockingLength);
	//for (int i : r2->entrances) {
	//	//
	//	FPolygon entranceBlock;

	//	FVector inMiddle = r2->specificEntrances.Contains(i) ? r2->specificEntrances[i] : middle(r2->points[i%r2->points.Num()], r2->points[i - 1]);
	//	FVector tangent = r2->points[i%r2->points.Num()] - r2->points[i - 1];
	//	tangent.Normalize();
	//	FVector altTangent = FRotator(0, 90, 0).RotateVector(tangent);
	//	entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 + altTangent*blockingLength);
	//	entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 - altTangent*blockingLength);
	//	entranceBlock.points.Add(inMiddle + tangent * entranceWidth*0.5 - altTangent*blockingLength);
	//	entranceBlock.points.Add(inMiddle + tangent * entranceWidth*0.5 + altTangent*blockingLength);
	//	//entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 + altTangent*blockingLength);
	//	blocking.Add(entranceBlock);

	//}

	for (auto &pair : r2->passiveConnections) {
		for (FRoomPolygon *p : pair.Value) {
			FPolygon entranceBlock;
			int32 num = p->activeConnections[r2];
			FVector inMiddle = p->specificEntrances.Contains(num) ? p->specificEntrances[num] : middle(p->points[num%p->points.Num()], p->points[num - 1]);
			FVector tangent = p->points[num%p->points.Num()] - p->points[num - 1];
			tangent.Normalize();
			FVector altTangent = FRotator(0, 90, 0).RotateVector(tangent);
			entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 + altTangent*blockingLength);
			entranceBlock.points.Add(inMiddle - tangent * entranceWidth*0.5 - altTangent*blockingLength);
			entranceBlock.points.Add(inMiddle + tangent * entranceWidth*0.5 - altTangent*blockingLength);
			entranceBlock.points.Add(inMiddle + tangent * entranceWidth*0.5 + altTangent*blockingLength);
			blocking.Add(entranceBlock);
		}

	}



	return blocking;
}


FPolygon getEntranceHole(FVector p1, FVector p2, float floorHeight, float doorHeight, float doorWidth, FVector doorPos) {
	FVector side = p2 - p1;
	float sideLen = side.Size();
	side.Normalize();
	float distToDoor = FVector::Dist(doorPos, p1) - doorWidth / 2;
	FMaterialPolygon doorPolygon;
	doorPolygon.points.Add(p1 + side*distToDoor + FVector(0, 0, doorHeight));
	doorPolygon.points.Add(p1 + side*distToDoor + FVector(0, 0, 2));// + FVector(0, 0, 100));
	doorPolygon.points.Add(p1 + side*distToDoor + side*doorWidth + FVector(0, 0, 2));// + FVector(0, 0, 100));
	doorPolygon.points.Add(p1 + side*distToDoor + side*doorWidth + FVector(0, 0, doorHeight));
	return doorPolygon;
}


TArray<FMaterialPolygon> getCrossWindow(FPolygon p, FVector center, float frameWidth, float frameLength, float frameDepth) {
	TArray<FMaterialPolygon> toReturn;
	for (int j = 1; j < 3; j++) {
		FMaterialPolygon frame;
		frame.type = PolygonType::windowFrame;
		frame.width = frameDepth;
		FVector tangent1 = center - p.points[j - 1];
		FVector tangent2 = p.points[j+1] - p.points[j];
		float tan2Len = tangent2.Size();
		tangent1.Normalize();
		tangent2.Normalize();
		FVector windowNormal = getNormal(p.points[j], p.points[j - 1], true);
		windowNormal.Normalize();
		frame.points.Add(middle(p.points[j - 1], p.points[j]) - tangent1 * frameWidth/2);
		frame.points.Add(middle(p.points[j - 1], p.points[j]) + tangent1 * frameWidth/2);
		frame.points.Add(middle(p.points[j - 1], p.points[j]) + tangent1 * frameWidth/2 + tangent2*tan2Len);
		frame.points.Add(middle(p.points[j - 1], p.points[j]) - tangent1 * frameWidth / 2 + tangent2*tan2Len);
 		FVector frameDir = frame.getDirection();
		frame.offset(frameDir*(frameDepth / 2 - 20));
		toReturn.Add(frame);
	}
	return toReturn;
}


TArray<FMaterialPolygon> getRectangularWindow(FPolygon p, FVector center, float frameWidth, float frameLength, float frameDepth) {
	TArray<FMaterialPolygon> toReturn;
	for (int j = 1; j < p.points.Num(); j++) {
		FMaterialPolygon frame;
		frame.type = PolygonType::windowFrame;
		frame.width = frameDepth;
		FVector tangent1 = center - p.points[j - 1];
		FVector tangent2 = center - p.points[j];
		tangent1.Normalize();
		tangent2.Normalize();
		FVector windowNormal = getNormal(p.points[j], p.points[j - 1], true);
		windowNormal.Normalize();
		frame.points.Add(p.points[j - 1]);
		frame.points.Add(p.points[j]);
		frame.points.Add(p.points[j] + tangent2 * frameWidth);
		frame.points.Add(p.points[j - 1] + tangent1 * frameWidth);
		FVector frameDir = frame.getDirection();
		frame.offset(frameDir*(frameDepth / 2 - 20));
		toReturn.Add(frame);
	}
	return toReturn;
}

TArray<FMaterialPolygon> getAppropriateWindowFrame(FPolygon p, FVector center, WindowType type) {
	float frameWidth = 15;
	float frameLength = 20;
	float frameDepth = 30;
	TArray<FMaterialPolygon> toReturn;
	p.points.Add(FVector(p.points[0]));

	switch (type) {
	case WindowType::rectangular: {
		toReturn = getRectangularWindow(p, center, frameWidth, frameLength, frameDepth);
	} break;
	case WindowType::cross: {
		toReturn = getRectangularWindow(p, center, frameWidth, frameLength, frameDepth);
		toReturn.Append(getCrossWindow(p, center, frameWidth, frameLength, frameDepth));
	} break;
	case WindowType::verticalLines: {
		toReturn = getRectangularWindow(p, center, frameWidth, frameLength, frameDepth);
		for (int i = 0; i < toReturn.Num(); i++) {
			toReturn.RemoveAt(i);
		}
	} break;
	case WindowType::rectangularHorizontalBigger: {
		toReturn = getRectangularWindow(p, center, frameWidth, frameLength, frameDepth);
		for (int i = 1; i < toReturn.Num(); i++) {
			toReturn.RemoveAt(i);
		}
	} break;
	}

	return toReturn;
}

bool fitWindowAround(float &wStart, float &wEnd, float otherStart, float otherEnd) {
	//return true;
	if (wStart < otherEnd)
		wEnd = std::min(wEnd, otherStart - 30);
	if (wEnd > otherStart)
		wStart = std::max(wStart, otherEnd + 30);
	return wStart < wEnd;
}


TArray<FMaterialPolygon> ARoomBuilder::interiorPlanToPolygons(TArray<FRoomPolygon*> roomPols, float floorHeight, float windowDensity, float windowHeight, float windowWidth, int floor, bool shellOnly, bool windowFrames) {
	TArray<FMaterialPolygon> toReturn;

	for (FRoomPolygon *rp : roomPols) {
		for (int i = 1; i < rp->points.Num() + 1; i++) {
			if (rp->toIgnore.Contains(i) || (shellOnly && !rp->exteriorWalls.Contains(i))) {
				continue;
			}
			FMaterialPolygon newP;
			newP.type = rp->exteriorWalls.Contains(i) ? PolygonType::exterior : PolygonType::interior;
			FVector p1 = rp->points[i - 1];
			FVector p2 = rp->points[i%rp->points.Num()];

			// this extra offset is to avoid z-fighting between walls
			FVector tan = p2 - p1;
			tan.Normalize();
			FVector extraBack = FVector(0, 0, 0);
			FVector extraFront = FVector(0, 0, 0);

			int prev = i > 1 ? i - 1 : rp->points.Num();
			int next = i < rp->points.Num()? i + 1 : 1;
			float extraBottom = 0;
			if (!rp->exteriorWalls.Contains(i)) {
				// this means that prev wall was exterior, move our back a little forward
				if (rp->exteriorWalls.Contains(prev))
					extraFront = tan * 20;

				// this means that next wall is exterior, move our front a little back
				if (rp->exteriorWalls.Contains(next))
					extraBack = -tan * 20;
				//extraBottom = 2;
			}


			
			newP.points.Add(p1 + FVector(0, 0, floorHeight) + extraFront);
			newP.points.Add(p1 + FVector(0, 0, extraBottom) + extraFront);
			newP.points.Add(p2 + FVector(0, 0, extraBottom) + extraBack);
			newP.points.Add(p2 + FVector(0, 0, floorHeight) + extraBack);
			//newP.points.Add(p1 + FVector(0, 0, floorHeight) + extraFront);

			TArray<FPolygon> holes;
			FVector entrancePos = FVector(-10000, -10000, -10000);
			float doorStart = 100000;
			float doorEnd = -1000000;
			if (rp->entrances.Contains(i) && FVector::Dist(rp->points[i-1], rp->points[i%rp->points.Num()]) > 200) {
				entrancePos = rp->specificEntrances.Contains(i) ?  rp->specificEntrances[i] : middle(rp->points[i - 1], rp->points[i%rp->points.Num()]);
				doorStart = FVector::Dist(rp->points[i - 1], entrancePos) - 137/2;
				doorEnd = doorStart + 137;
				//holes.Add(getEntranceHole(rp->points[i - 1], rp->points[i%rp->points.Num()], floorHeight, 297, 137, entrancePos));
				// 2 1 4 3
				auto entH = getEntranceHole(rp->points[i - 1], rp->points[i%rp->points.Num()], floorHeight, 297, 137, entrancePos);
				newP.points.EmplaceAt(2, entH[1]);
				newP.points.EmplaceAt(3, entH[0]);
				newP.points.EmplaceAt(4, entH[3]);
				newP.points.EmplaceAt(5, entH[2]);

			}
			if (rp->windows.Contains(i)) {
				FVector tangent = rp->points[i%rp->points.Num()] - rp->points[i - 1];
				float len = tangent.Size();
				tangent.Normalize();

				TArray<FPolygon> windows;
				int spaces = FMath::FloorToInt(std::min(windowDensity * len, len / (windowWidth + 20.0f)));
				float jumpLen = len / (float)spaces;

				for (int j = 1; j < spaces; j++) {
					FPolygon currWindow;
					float currStart = j * jumpLen - windowWidth/2;
					float currEnd = j * jumpLen + windowWidth/2;
					if (fitWindowAround(currStart, currEnd, doorStart, doorEnd) && currEnd - currStart > 100) {
						FVector pw1 = rp->points[i - 1] + tangent * currStart + FVector(0, 0, 50 + windowHeight);
						FVector pw2 = pw1 - FVector(0, 0, windowHeight);
						FVector pw3 = rp->points[i - 1] + tangent * currEnd + FVector(0, 0, 50);
						FVector pw4 = pw3 + FVector(0, 0, windowHeight);

						currWindow.points.Add(pw1);
						currWindow.points.Add(pw2);
						currWindow.points.Add(pw3);
						currWindow.points.Add(pw4);
						windows.Add(currWindow);
					}
					//}
				}
					for (FPolygon p : windows) {
						FMaterialPolygon win;
						win.points = p.points;
						win.width = 8;
						win.type = shellOnly ? PolygonType::occlusionWindow : PolygonType::window;
						toReturn.Add(win);
						FVector center = p.getCenter();
						// potentially add window frame as well

						if (windowFrames)
							toReturn.Append(getAppropriateWindowFrame(p, center, rp->windowType));

				}
				holes.Append(windows);

			}
			//holes.Empty();
			TArray<FMaterialPolygon> pols = getSideWithHoles(newP, holes, rp->exteriorWalls.Contains(i) ? PolygonType::exterior : PolygonType::interior);
			toReturn.Append(pols);

			//else {
			//	toReturn.Add(newP);
			//}

		}

	}
	return toReturn;

}

bool attemptPlaceOnTop(FPolygon &p, TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes) {
	return false;
}

void placeRows(FRoomPolygon *r2, TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes, FRotator offsetRot, FString name, float vertDens, float horDens, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map) {
	// get a line through the room
	//SplitStruct res = r2->getSplitProposal(false, 0.5);
	//if (res.min == 0) {
	//	return;
	//}
	for (int k = 1; k < r2->points.Num(); k++) {
		FVector origin = middle(r2->points[k], r2->points[k - 1]);
		FVector tangent = r2->points[k] - r2->points[k - 1];
		tangent.Normalize();
		FVector normal = FRotator(0, 270, 0).RotateVector(tangent);
		int target = -1;
		FVector targetP;
		r2->getSplitCorrespondingPoint(k, origin, normal, target, targetP);
		if (target == -1) {
			UE_LOG(LogTemp, Warning, TEXT("Couldn't place row, no corresponding split point"));
			return;
		}
		float width = FVector::Dist(r2->points[k], r2->points[k - 1]);
		float height = FVector::Dist(origin, targetP);

		int numWidth = FMath::FloorToInt(width * horDens) + 1;
		int numHeight = FMath::FloorToInt(height * vertDens) + 1;
			
		float intervalWidth = width / numWidth;
		float intervalHeight = height / numHeight;
		TArray<FPolygon> toPlace;
		for (int i = 1; i < numWidth; i++) {
			for (int j = 1; j < numHeight; j++) {
				FPolygon pol = getPolygon(normal.Rotation(), r2->points[k - 1] + i*intervalWidth*tangent + j*intervalHeight*normal, name, map);
				// make sure it's fully inside the room
				if (!testCollision(pol, placed, 0, *r2)) {// && intersection(pol, *r2).X == 0.0f) {
					toPlace.Add(pol);
					meshes.Add(FMeshInfo{ name, FTransform(normal.Rotation(),r2->points[k - 1] + i*intervalWidth*tangent + j*intervalHeight*normal, FVector(1.0f, 1.0f, 1.0f)), true });
				}
			}
		}
		if (toPlace.Num() > 0) {
			placed.Append(toPlace);
			return;
		}
	}
	

	//FVector start =
}



static TArray<FMeshInfo> getMeetingRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed = getBlockingVolumes(r2, 200, 200);
	FVector dir = r2->getRoomDirection();
	FVector center = r2->getCenter();

	FPolygon pol = getPolygon(dir.Rotation(), center, "office_meeting_table", map);
	if (!testCollision(pol, placed, 0, *r2)) {
		meshes.Add(FMeshInfo{ "office_meeting_table", FTransform(dir.Rotation(), center + FVector(0,0,10), FVector(1.0, 1.0, 1.0)) });
	}
	else {
		pol = getPolygon(dir.Rotation() + FRotator(0, 90, 0), center, "office_meeting_table", map);
		if (!testCollision(pol, placed, 0, *r2)) {
			meshes.Add(FMeshInfo{ "office_meeting_table", FTransform(dir.Rotation() + FRotator(0, 90, 0), center + FVector(0,0,10), FVector(1.0, 1.0, 1.0)) });
		}
	}
	float offsetLen = 100;

	if (randFloat() < 0.5) {
		r2->attemptPlace(placed, meshes, false, 1, "shelf", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	}

	if (randFloat() < 0.5) {
		// add whiteboard
		r2->attemptPlace(placed, meshes, false, 1, "office_whiteboard", FRotator(0, 180, 0), FVector(0, 0, 180), map, true);
	}

	r2->attemptPlace(placed, meshes, true, 1, "dispenser", FRotator(0, 0, 0), FVector(0, 0, 0), map, false);

	return meshes;
}



static TArray<FMeshInfo> getWorkingRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;
	placed = getBlockingVolumes(r2, 200, 200);
	// first is height, second is width
	placeRows(r2, placed, meshes, FRotator(0, 180, 0), "office_cubicle", 0.0016, 0.016, map);
	TArray<FMeshInfo> finalMeshes;
	for (FMeshInfo mesh : meshes) {
		mesh.transform.SetLocation(mesh.transform.GetLocation() + FVector(0, 0, 15));
		finalMeshes.Add(mesh);
		FVector compUserOffset = FVector(90, 0, 115);
		FRotator compUserRot = FRotator(0, -90, 0);
		FVector compBoxOffset = FVector(90, 100, -45);
		finalMeshes.Add(FMeshInfo{ "comp_user", FTransform{mesh.transform.Rotator() + compUserRot, mesh.transform.GetLocation() + mesh.transform.Rotator().RotateVector(compUserOffset) } });
		finalMeshes.Add(FMeshInfo{ "comp_box", FTransform{ mesh.transform.Rotator(), mesh.transform.GetLocation() + mesh.transform.Rotator().RotateVector(compBoxOffset) } });
	}
	r2->attemptPlace(placed, finalMeshes, true, 1, "trash_can", FRotator(0, 0, 0), FVector(0, 0, 7), map, false);
	//finalMeshes.Add(FMeshInfo{ "trash_can", FTransform{FRotator(0,0,0), r2->getCenter() + FVector(0,0,20)} });
	return finalMeshes;
}

RoomBlueprint getOfficeBlueprint(float areaScale) {
	// one meeting room, rest working rooms
	TArray<RoomSpecification> needed;
	RoomSpecification meetingRoom{ 100 * areaScale, 200 * areaScale, SubRoomType::meeting };
	needed.Add(meetingRoom);
	RoomSpecification bathroom{ 30 * areaScale, 60 * areaScale, SubRoomType::bath };
	RoomSpecification workRoom{ 10 * areaScale, 200 * areaScale, SubRoomType::work };
	TArray<RoomSpecification> optional;
	optional.Add(workRoom);
	optional.Add(workRoom);
	optional.Add(workRoom);
	needed.Add(bathroom);
	optional.Add(workRoom);

	return RoomBlueprint{ needed, optional, false };
}

RoomBlueprint getRestaurantBlueprint(float areaScale) {
	TArray<RoomSpecification> needed;
	RoomSpecification restaurant{ 50 * areaScale, 1000 * areaScale, SubRoomType::restaurant };
	needed.Add(restaurant);

	TArray<RoomSpecification> optional;
	optional.Add(restaurant);
	return RoomBlueprint{ needed, optional };

}


RoomBlueprint getStoreBlueprint(float areaScale) {
	// one meeting room, rest working rooms
	TArray<RoomSpecification> needed;
	RoomSpecification storeBack{ 50 * areaScale, 200 * areaScale, SubRoomType::storeBack };
	RoomSpecification storeFront{ 200 * areaScale, 400 * areaScale, SubRoomType::storeFront };
	RoomSpecification bathroom{ 30 * areaScale, 60 * areaScale, SubRoomType::bath };
	needed.Add(storeFront);

	TArray<RoomSpecification> optional;
	optional.Add(bathroom);
	optional.Add(storeBack);

	return RoomBlueprint{ needed, optional };

}

RoomBlueprint getApartmentBlueprint(float areaScale) {
	TArray<RoomSpecification> needed;
	RoomSpecification kitchen{40*areaScale, 90*areaScale, SubRoomType::kitchen};
	RoomSpecification bathroom{30*areaScale, 60*areaScale, SubRoomType::bath };
	RoomSpecification bedroom{50*areaScale, 100*areaScale, SubRoomType::bed };
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

	return RoomBlueprint{ needed, optional, true };


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
	FTransform res = r2->attemptGetPosition(placed, meshes, false, 3, "tv", FRotator(0, 0, 0), FVector(35, 0, 150), map, true);
	if (res.GetLocation().X != 0.0f) {
		meshes.Add({ "tv", res });
		placed.Add(getPolygon(res.Rotator(), res.GetLocation(), "tv", map));
		FTransform sofaTrans = FTransform{ res.GetRotation(), res.GetLocation() + res.GetRotation().RotateVector(FVector(270, 0, 0)), FVector(1.0f, 1.0f, 1.0f) };
		FPolygon pol = getPolygon(sofaTrans.Rotator() , sofaTrans.GetLocation(), "sofa", map);
		if (!testCollision(pol, placed, 0, *r2)) {
			placed.Add(pol);
			meshes.Add({ "sofa", FTransform{sofaTrans.Rotator(), sofaTrans.GetLocation() - FVector(0,0,140), FVector(1,1,1)} });
		}

	}
	meshes.Append(potentiallyGetTableAndChairs(r2, placed, map));

	return meshes;
}

static TArray<FMeshInfo> getRestaurantRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;

	TArray<FPolygon> placed;
	placed.Append(getBlockingVolumes(r2, 200, 200));
	r2->attemptPlace(placed, meshes, false, 1, "restaurant_bar", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);
	TArray<FMeshInfo> tables;

	placeRows(r2, placed, tables, FRotator(0, 0, 0), "restaurant_table", FMath::FRandRange(0.0015, 0.003), FMath::FRandRange(0.0015, 0.003), map);
	tables.RemoveAt(0, tables.Num() / 2);
	for (FMeshInfo table : tables) {
		FTransform trans = table.transform;
		if (FMath::RandBool())
			meshes.Add({ "restaurant_chair", trans });
		trans.SetRotation(FQuat(trans.Rotator() + FRotator(0, 120, 0)));
		if (FMath::RandBool())
			meshes.Add({ "restaurant_chair", trans });
		trans.SetRotation(FQuat(trans.Rotator() + FRotator(0, 120, 0)));
		if (FMath::RandBool())
			meshes.Add({ "restaurant_chair", trans });

	}
	meshes.Append(tables);
	return meshes;
}

static TArray<FMeshInfo> getBathRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	TArray<FPolygon> blocking = getBlockingVolumes(r2, 200, 100);
	placed.Append(blocking);
	r2->attemptPlace(placed, meshes, false, 2, "toilet" , FRotator(0, 270, 0), FVector(0, 0, 0), map, false);
	FTransform res = r2->attemptGetPosition(placed, meshes, false, 2, "sink", FRotator(0, 0, 0), FVector(0, 0, 0), map, false);
	if (res.GetLocation().X != 0.0f) {
		FPolygon pol = getPolygon(res.Rotator(), res.GetLocation(), "sink", map);
		placed.Add(pol);
		meshes.Add(FMeshInfo{ "sink", res});
		res.SetLocation(res.GetLocation() + FVector(0, 0, 150));
		//res.Ro
		meshes.Add(FMeshInfo{ "mirror", FTransform(res.Rotator() + FRotator(0, 270, 0), res.GetLocation() + FVector(0, 0, 55) - res.Rotator().Vector() * 40, FVector(1.0, 1.0, 1.0)) });
	}
	return meshes;
}


static TArray<FMeshInfo> getBedRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;
	//placed.Add(r2);
	placed.Append(getBlockingVolumes(r2, 200, 200));
	r2->attemptPlace(placed, meshes, true, 2, "bed", FRotator(0, 270, 0), FVector(0, 0, 60), map, false);
	r2->attemptPlace(placed, meshes, true, 1, "small_table", FRotator(0, 0, 0), FVector(0, 0, -50), map, false);
	r2->attemptPlace(placed, meshes, false, 1, "shelf", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	r2->attemptPlace(placed, meshes, false, 1, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);

	return meshes;
}

static TArray<FMeshInfo> getHallWay(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	r2->attemptPlace(placed, meshes, true, 1, "hanger", FRotator(0, 90, 0), FVector(0, 0, 10), map, false);
	return meshes;
}

static TArray<FMeshInfo> getKitchen(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	r2->attemptPlace(placed, meshes, false, 2, "kitchen", FRotator(0, 90, 0), FVector(0, 0, 0), map, true);
	meshes.Append(potentiallyGetTableAndChairs(r2, placed, map));
	r2->attemptPlace(placed, meshes, false, 1, "shelf_upper_large", FRotator(0, 270, 0), FVector(0, 0, 200), map, false);
	r2->attemptPlace(placed, meshes, false, 1, "fridge", FRotator(0, 90, 0), FVector(0, 0, 0), map, true);
	r2->attemptPlace(placed, meshes, false, 1, "oven", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	return meshes;
}

static TArray<FMeshInfo> getCorridor(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;
	placed.Append(getBlockingVolumes(r2, 200, 100));
	r2->attemptPlace(placed, meshes, false, 1, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 10), map, true);

	return meshes;
}


static TArray<FMeshInfo> getCloset(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	r2->attemptPlace(placed, meshes, false, 1, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 10), map, true);
	r2->attemptPlace(placed, meshes, false, 1, "shelf_upper_large", FRotator(0, 270, 0), FVector(0, 0, 200), map, true);


	return meshes;
}

static TArray<FMeshInfo> getStoreFront(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	for (int i = 0; i < 5; i++) {
		r2->attemptPlace(placed, meshes, false, 1, "counter", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	}
	//r2->attemptPlace(placed, meshes, 50, false, 5, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);
	///r2->attemptPlace(placed, meshes, 45.0f, false, 1, "shelf_upper_large", FRotator(0, 270, 0), FVector(0, 0, 200), map, true);


	return meshes;
}

static TArray<FMeshInfo> getStoreBack(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	//r2->attemptPlace(placed, meshes, 50, false, 5, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);
	//r2->attemptPlace(placed, meshes, 45.0f, false, 1, "shelf_upper_large", FRotator(0, 270, 0), FVector(0, 0, 200), map, true);


	return meshes;
}


FRoomInfo placeBalcony(FRoomPolygon *p, int place, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;

	float width = 500;
	float length = 200;
	float height = 150;

	FVector tangent = p->points[place%p->points.Num()] - p->points[place - 1];
	FVector normal = getNormal(p->points[place%p->points.Num()], p->points[place - 1], false);
	normal.Normalize();
	float len = tangent.Size();
	width = std::min(width, len);
	tangent.Normalize();
	FVector start = p->points[place - 1] + tangent*(len - width) * 0.5;
	FVector end = p->points[place - 1] + tangent*(len + width) * 0.5;
	FVector endOut = end + normal*length;
	FVector startOut = start + normal*length;

	FMaterialPolygon floor;
	floor.type = PolygonType::exteriorSnd;
	floor.points.Add(start);
	floor.points.Add(end);
	floor.points.Add(endOut);
	floor.points.Add(startOut);

	r.pols.Add(floor);


	FMaterialPolygon side1;
	side1.type = PolygonType::exteriorSnd;
	FMaterialPolygon side2;
	side2.type = PolygonType::exteriorSnd;
	FMaterialPolygon side3;
	side3.type = PolygonType::exteriorSnd;

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
	p->windows.Remove(place);

	return r;
}

FRoomInfo ARoomBuilder::buildOffice(FRoomPolygon *f, int floor, float height, float density, float windowHeight, float windowWidth, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool shellOnly) {

	FRoomInfo r;
	TArray<FRoomPolygon*> roomPols = f->getRooms(getOfficeBlueprint(1.0f));
	for (FRoomPolygon *r2 : roomPols) {
		if (!shellOnly)
			r.meshes.Add(FMeshInfo{ "office_lamp", FTransform(r2->getCenter() + FVector(0, 0, height - 45)) });
		for (int i : r2->entrances) {
			if (shellOnly && !r2->exteriorWalls.Contains(i))
				continue;
			FVector doorPos = r2->specificEntrances.Contains(i) ? r2->specificEntrances[i] : middle(r2->points[i], r2->points[i - 1]);
			FVector dir1 = getNormal(r2->points[i%r2->points.Num()], r2->points[i - 1], true);
			dir1.Normalize();
			FVector dir2 = r2->points[i%r2->points.Num()] - r2->points[i - 1];
			dir2.Normalize();
			r.meshes.Add(FMeshInfo{ "door_frame", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });
			//r.meshes.Add(FMeshInfo{ "door", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });
		}
		if (!shellOnly) {
			switch (r2->type) {
			case SubRoomType::meeting: r.meshes.Append(getMeetingRoom(r2, map));
				break;
			case SubRoomType::work: r.meshes.Append(getWorkingRoom(r2, map));
				break;
			case SubRoomType::bath: r.meshes.Append(getBathRoom(r2, map));
				break;
			}
		}


	}
	r.pols.Append(interiorPlanToPolygons(roomPols, height, density, windowHeight, windowWidth, floor, shellOnly, false));
	for (FRoomPolygon* roomP : roomPols)
		delete(roomP);
	return r;
}

FRoomInfo ARoomBuilder::buildRestaurant(FRoomPolygon *f, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool shellOnly, FRandomStream stream) {
	FRoomInfo r;
	TArray<FRoomPolygon*> roomPols = f->getRooms(getRestaurantBlueprint(1.0f));
	for (FRoomPolygon* r2 : roomPols) {
		for (int i : r2->windows) {
			if (FVector::Dist(f->points[i%f->points.Num()], f->points[i-1]) > 150.0f)
			r2->entrances.Add(i);
		}
		for (int i : r2->entrances) {
			FVector doorPos = r2->specificEntrances.Contains(i) ? r2->specificEntrances[i] : middle(r2->points[i%r2->points.Num()], r2->points[i - 1]);
			FVector dir1 = getNormal(r2->points[i%r2->points.Num()], r2->points[i - 1], true);
			dir1.Normalize();
			FVector dir2 = r2->points[i%r2->points.Num()] - r2->points[i - 1];
			dir2.Normalize();
			r.meshes.Add(FMeshInfo{ "door_frame", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });
			//r.meshes.Add(FMeshInfo{ "door", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });
		}
		if (!shellOnly) {
			//r.meshes.Add(FMeshInfo{ "apartment_lamp", FTransform(r2->getCenter() + FVector(0, 0, height - 45)) });
			switch (r2->type) {
			case SubRoomType::restaurant: r.meshes.Append(getRestaurantRoom(r2, map));
				r.meshes.Add(FMeshInfo{ "restaurant_room", FTransform(r2->getCenter()) });
				break;
			case SubRoomType::bath: r.meshes.Append(getBathRoom(r2, map));
				r.meshes.Add(FMeshInfo{ "bath", FTransform(r2->getCenter()) });
				break;
			}
		}
	}
	r.pols.Append(interiorPlanToPolygons(roomPols, height, 1.0, stream.FRandRange(300,200), stream.FRandRange(300, 200), 0, shellOnly, true));
	for (FRoomPolygon* roomP : roomPols)
		delete(roomP);
	return r;
}


FRoomInfo ARoomBuilder::buildStore(FRoomPolygon *f, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool shellOnly) {
	FRoomInfo r;

	TArray<FRoomPolygon*> roomPols = f->getRooms(getStoreBlueprint(1.0f));

	for (FRoomPolygon* r2 : roomPols) {
		// stores always have entrances outwards
		for (int i : r2->windows) {
			if (f->points.Num() > i && FVector::Dist(f->points[i], f->points[i - 1]) > 150.0f)
				r2->entrances.Add(i);
		}
		for (int i : r2->entrances) {
			FVector doorPos = r2->specificEntrances.Contains(i) ? r2->specificEntrances[i] : middle(r2->points[i%r2->points.Num()], r2->points[i - 1]);
			FVector dir1 = getNormal(r2->points[i%r2->points.Num()], r2->points[i - 1], true);
			dir1.Normalize();
			FVector dir2 = r2->points[i%r2->points.Num()] - r2->points[i - 1];
			dir2.Normalize();
			r.meshes.Add(FMeshInfo{ "door_frame", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });
			//r.meshes.Add(FMeshInfo{ "door", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });
		}
		if (!shellOnly) {
			r.meshes.Add(FMeshInfo{ "apartment_lamp", FTransform(r2->getCenter() + FVector(0, 0, height - 45)) });
			switch (r2->type) {
			case SubRoomType::storeFront: r.meshes.Append(getStoreFront(r2, map));
				r.meshes.Add(FMeshInfo{ "store_front", FTransform(r2->getCenter()) });
				break;
			case SubRoomType::bath: r.meshes.Append(getBathRoom(r2, map));
				r.meshes.Add(FMeshInfo{ "bath", FTransform(r2->getCenter()) });
				break;
			}
		}

	}

	r.pols.Append(interiorPlanToPolygons(roomPols, height, 1.0, 300, 300, 0, shellOnly, true));
	for (FRoomPolygon* roomP : roomPols)
		delete(roomP);
	return r;
}


FRoomInfo ARoomBuilder::buildApartment(FRoomPolygon *f, int floor, float height, float density, float windowHeight, float windowWidth, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool balcony, bool shellOnly) {

	FRoomInfo r;
	TArray<FRoomPolygon*> roomPols = f->getRooms(getApartmentBlueprint(1.0f));
	
	if (balcony) {
		for (FRoomPolygon *p : roomPols) {
			if (splitableType(p->type)) {
				// these are the balcony candidate rooms
				if (p->windows.Num() > 0) {
					for (int place : p->windows) {
						if (FVector::DistSquared(p->points[place%p->points.Num()], p->points[place - 1]) > 10000) {
							p->entrances.Add(place);
							FVector mid = middle(p->points[place%p->points.Num()], p->points[place - 1]);
							p->specificEntrances.Add(place, mid);
							r = placeBalcony(p, place, map);
							goto balconyDone;
						}

					}

				}
			}
		}
	}
	balconyDone:

	if (!shellOnly) {
		for (FRoomPolygon* r2 : roomPols) {
			for (int i : r2->entrances) {
				FVector doorPos = r2->specificEntrances.Contains(i) ? r2->specificEntrances[i] : middle(r2->points[i], r2->points[i - 1]);
				FVector dir1 = getNormal(r2->points[i%r2->points.Num()], r2->points[i - 1], true);
				dir1.Normalize();
				FVector dir2 = r2->points[i%r2->points.Num()] - r2->points[i - 1];
				dir2.Normalize();
				r.meshes.Add(FMeshInfo{ "door_frame", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });

				//r.meshes.Add(FMeshInfo{ "door", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });
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
	}

	r.pols.Append(interiorPlanToPolygons(roomPols, height, density, windowHeight, windowWidth, floor, shellOnly, true));

	for (FRoomPolygon *p : roomPols) {
		delete p;
	}

	return r;
}


FRoomInfo ARoomBuilder::buildRoom(FRoomPolygon *f, RoomType type, int floor, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony, bool shellOnly, FRandomStream stream) {
	if (!f->canRefine) {
		FRoomInfo r;
		//r.beginning = beginning;
		//r.height = height;
		TArray<FRoomPolygon*> pols;
		pols.Add(f);
		switch (type) {
			case RoomType::office: r.pols = interiorPlanToPolygons(pols, height, 1, 340, 190, floor, shellOnly, false);
			break;
			case RoomType::apartment: r.pols = interiorPlanToPolygons(pols, height, 0.003, 200, 200, floor, shellOnly, true);
			break;
			default: r.pols = interiorPlanToPolygons(pols, height, 0.003, 200, 200, floor, shellOnly, true);
				break;

		}

		return r;
	}
	switch (type) {
	case RoomType::office: return buildOffice(f, floor, height, 1/* 0.0042*/, 320.0f, 190.0f, map, shellOnly);
	case RoomType::apartment: return buildApartment(f, floor, height, 0.003, 200.0f, 200.0f, map, potentialBalcony, shellOnly);
	case RoomType::store: return buildStore(f, height, map, shellOnly);
	case RoomType::restaurant: return buildRestaurant(f, height, map, shellOnly, stream);
	}
	return FRoomInfo();
}
