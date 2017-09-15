// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "polypartition.h"
#include "RoomBuilder.h"


// Sets default values
ARoomBuilder::ARoomBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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

TArray<FMaterialPolygon> getRectangularWindow(FPolygon p, FVector center, float frameWidth, float frameLength, float frameDepth) {
	TArray<FMaterialPolygon> toReturn;
	for (int j = 1; j < p.points.Num()+1; j++) {
		FMaterialPolygon frame;
		frame.type = PolygonType::windowFrame;
		frame.width = frameDepth;
		FVector tangent1 = center - p.points[j - 1];
		FVector tangent2 = center - p.points[j%p.points.Num()];
		tangent1.Normalize();
		tangent2.Normalize();
		FVector windowNormal = getNormal(p.points[j%p.points.Num()], p.points[j - 1], true);
		windowNormal.Normalize();
		frame.points.Add(p.points[j - 1]);
		frame.points.Add(p.points[j%p.points.Num()]);
		frame.points.Add(p.points[j%p.points.Num()] + tangent2 * frameWidth);
		frame.points.Add(p.points[j - 1] + tangent1 * frameWidth);
		FVector frameDir = frame.getDirection();
		frame.offset(frameDir*(frameDepth / 2 - 20));
		toReturn.Add(frame);
	}
	return toReturn;
}


static TArray<FMaterialPolygon> getSign(FVector startP, FVector endP, FRandomStream stream, bool sideways) {
	TArray<FMaterialPolygon> signPols;

	float minHeight = 100;
	float maxHeight = 100;
	float minLen = 100;
	float maxLen = 500;
	FVector tangent = endP - startP;
	float len = tangent.Size();
	tangent.Normalize();
	float actualLen = stream.FRandRange(minLen, maxLen);
	float actualHeight = stream.FRandRange(minHeight, maxHeight);
	if (sideways) {

		maxLen = 300;
		actualLen = stream.FRandRange(minLen, maxLen);
		float first = stream.FRandRange(0, len);
		float outLen = 50;
		float thickness = 20;
		len = actualLen;
		startP += first * tangent;
		FVector tangentOld = tangent;
		tangent = FRotator(0, 90, 0).RotateVector(tangent);
		FMaterialPolygon lineHorizontal;
		lineHorizontal.type = PolygonType::roadMiddle;
		FVector startH = startP + -tangentOld * thickness / 3;
		lineHorizontal += startH + FVector(0, 0, actualHeight + 20);
		lineHorizontal += startH + FVector(0, 0, actualHeight + 30);
		lineHorizontal += startH + tangent*(actualLen + outLen*2) + FVector(0, 0, actualHeight + 30);
		lineHorizontal += startH + tangent*(actualLen + outLen*2) + FVector(0, 0, actualHeight + 20);
		lineHorizontal.width = thickness/3;
		signPols.Add(lineHorizontal);

		startP += tangent * outLen;
		// add two lines down, evenly spread
		FVector lineStartP = startP + tangent * actualLen / 3 + FVector(0, 0, actualHeight + 20) + -tangentOld * thickness / 3;
		FMaterialPolygon lineVertical;
		lineVertical.type = PolygonType::roadMiddle;
		lineVertical += lineStartP;
		lineVertical += lineStartP + tangent * 10;
		lineVertical += lineStartP + tangent * 10 + FVector(0, 0, -20);
		lineVertical += lineStartP + FVector(0, 0, -20);
		lineVertical.width = thickness / 3;

		FMaterialPolygon lineVertical2 = lineVertical;
		lineVertical2.offset(tangent * actualLen / 3);

		signPols.Add(lineVertical);
		signPols.Add(lineVertical2);


		//startP +=
	}


	float first = stream.FRandRange(0, len - actualLen);
	FVector startLoc = startP + tangent * first;
	FVector endLoc = startP + tangent * (first + actualLen);
	FMaterialPolygon outline;
	outline.type = PolygonType::roadMiddle;
	outline += startLoc;
	outline += startLoc + FVector(0, 0, actualHeight);
	outline += endLoc + FVector(0, 0, actualHeight);
	outline += endLoc;
	FVector norm = outline.getDirection();
	float outlineWidth = 15;
	signPols.Add(outline);
	return signPols;
}

static TArray<FMaterialPolygon> placeSigns(FRoomPolygon *r2, FRandomStream stream) {
	// one sign per room max
	TArray<FMaterialPolygon> pols;
	for (int i : r2->windows) {
		pols = getSign((*r2)[i - 1] + FVector(0,0,300), (*r2)[i%r2->points.Num()] + FVector(0, 0, 300), stream, stream.FRand() < 0.2);
		return pols;
	}
	return pols;
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
	} break;
	case WindowType::rectangularHorizontalBigger: {
		toReturn = getRectangularWindow(p, center, frameWidth, frameLength, frameDepth);

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
			}


			
			newP.points.Add(p1 + FVector(0, 0, floorHeight) + extraFront);
			newP.points.Add(p1 + FVector(0, 0, extraBottom) + extraFront);
			newP.points.Add(p2 + FVector(0, 0, extraBottom) + extraBack);
			newP.points.Add(p2 + FVector(0, 0, floorHeight) + extraBack);

			TArray<FPolygon> holes;
			FVector entrancePos = FVector(-10000, -10000, -10000);
			float doorStart = 100000;
			float doorEnd = -1000000;
			if (rp->entrances.Contains(i) && FVector::Dist(rp->points[i-1], rp->points[i%rp->points.Num()]) > 100) {
				entrancePos = rp->specificEntrances.Contains(i) ?  rp->specificEntrances[i] : middle(rp->points[i - 1], rp->points[i%rp->points.Num()]);
				doorStart = FVector::Dist(rp->points[i - 1], entrancePos) - 137/2;
				doorEnd = doorStart + 137;
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
						else
							toReturn.Append(fillOutPolygon(p));

				}
				holes.Append(windows);

			}
			TArray<FMaterialPolygon> pols = getSideWithHoles(newP, holes, rp->exteriorWalls.Contains(i) ? PolygonType::exterior : PolygonType::interior);
			toReturn.Append(pols);


		}

	}
	return toReturn;

}

static void attemptPlaceAroundPolygon(FPolygon center, FString itemName, TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes, FRotator offsetRot, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, float density, FPolygon &surrounding) {
	for (int i = 1; i < center.points.Num() + 1; i++) {
		FVector tan = center[i%center.points.Num()] - center[i - 1];
		FVector dir = getNormal(center[i%center.points.Num()], center[i - 1], false);
		float len = tan.Size();
		tan.Normalize();
		float numSteps = std::floor(len * density) + 1;
		float stepLen = len / numSteps;
		for (int j = 1; j < numSteps; j++) {
			FVector pos = center[i - 1] + tan * j * stepLen;
			FPolygon res = getPolygon(dir.Rotation() + offsetRot, pos, itemName, map);
			FVector toMove = fitPolygonNextToPolygon(center, res, i, FRotator(0, 0, 0));
			pos += toMove;
			res.offset(toMove);
			//if (!testCollision(res, placed, 0, surrounding)){
			meshes.Add({ itemName, FTransform(dir.Rotation() + offsetRot, pos) });
			placed.Add(res);
			//}
		}
	}
}


bool attemptPlaceOnTop(FMeshInfo toUse, TArray<FMeshInfo> &meshes, FString name, float minDist, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map) {
	//getPolygon(toUse.transform.Rotator(), toUse.transform.GetLocation(), , map);
	FVector min;
	FVector max;
	if (map.Contains(toUse.description))
		map[toUse.description]->GetLocalBounds(min, max);
	else
		return false;
	FPolygon pol;
	pol.points.Add(FVector(min.X, min.Y, 0.0f) + toUse.transform.GetLocation());
	pol.points.Add(FVector(max.X, min.Y, 0.0f) + toUse.transform.GetLocation());
	pol.points.Add(FVector(max.X, max.Y, 0.0f) + toUse.transform.GetLocation());
	pol.points.Add(FVector(min.X, max.Y, 0.0f) + toUse.transform.GetLocation());
	FVector res = pol.getRandomPoint(true, minDist);
	if (res.X != 0.0f) {
		meshes.Add(FMeshInfo{ name, FTransform{FRotator(0,0,0), res + FVector(0,0,max.Z)}});
		return true;
	}
	return false;
}


static FRoomInfo getMeetingRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;
	TArray<FPolygon> placed = getBlockingVolumes(r2, 200, 200);
	FVector dir = r2->getRoomDirection();
	FVector center = r2->getCenter();


	attemptPlaceCenter(*r2, placed, r.meshes, "office_meeting_table", FRotator(0, 0, 0), FVector(0, 0, 10), map);
	float offsetLen = 100;

	if (r.meshes.Num() > 0)
		attemptPlaceAroundPolygon(placed[placed.Num() - 1], "office_chair", placed, r.meshes, FRotator(0, 180, 0), map, FMath::FRandRange(0.005, 0.01), *r2);

	if (FMath::FRand() < 0.5) {
		r2->attemptPlace(placed, r.meshes, false, 1, "shelf", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	}

	if (FMath::FRandRange(0,0.9999) < 0.5) {
		r2->attemptPlace(placed, r.meshes, false, 1, "office_whiteboard", FRotator(0, 180, 0), FVector(0, 0, 180), map, true);
	}

	r2->attemptPlace(placed, r.meshes, true, 1, "dispenser", FRotator(0, 0, 0), FVector(0, 0, 0), map, false);

	return r;
}



static FRoomInfo getWorkingRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	FRoomInfo r;
	TArray<FPolygon> placed;
	placed = getBlockingVolumes(r2, 200, 200);
	// first is height, second is width
	placeRows(r2, placed, meshes, FRotator(0, 180, 0), "office_cubicle", 0.0016, 0.002, map);
	for (FMeshInfo mesh : meshes) {
		mesh.transform.SetLocation(mesh.transform.GetLocation() + FVector(0, 0, 15));
		r.meshes.Add(mesh);
		FVector compUserOffset = FVector(90, 0, 115);
		FRotator compUserRot = FRotator(0, -90, 0);
		FVector compBoxOffset = FVector(90, 100, -45);
		FVector chairOffset = FVector(230, 0, 0);
		r.meshes.Add(FMeshInfo{ "comp_user", FTransform{mesh.transform.Rotator() + compUserRot, mesh.transform.GetLocation() + mesh.transform.Rotator().RotateVector(compUserOffset) } });
		r.meshes.Add(FMeshInfo{ "comp_box", FTransform{ mesh.transform.Rotator(), mesh.transform.GetLocation() + mesh.transform.Rotator().RotateVector(compBoxOffset) } });
		r.meshes.Add(FMeshInfo{ "office_chair", FTransform{mesh.transform.Rotator(), mesh.transform.GetLocation() + mesh.transform.Rotator().RotateVector(chairOffset) } });
	}
	r2->attemptPlace(placed, r.meshes, true, 1, "trash_can", FRotator(0, 0, 0), FVector(0, 0, 7), map, false);
	return r;
}

static TArray<FMeshInfo> placeAwnings(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	for (int i : r2->windows) {
		FVector tan = (*r2)[i%r2->points.Num()] - (*r2)[i - 1];
		float len = tan.Size();
		tan.Normalize();

		FVector min;
		FVector max;
		if (map.Contains("awning"))
			map["awning"]->GetLocalBounds(min, max);
		else
			return TArray<FMeshInfo>();
		float width = max.Y - min.Y + 5;
		for (int j = width / 2; j < len - width / 2; j += width) {
			meshes.Add(FMeshInfo{ "awning", FTransform{ getNormal((*r2)[i%r2->points.Num()], (*r2)[i - 1], true).Rotation(), (*r2)[i - 1] + tan * j + FVector(0,0,380)}});

		}
	}
	return meshes;
}





static TArray<FMeshInfo> potentiallyGetTableAndChairs(FRoomPolygon *r2, TArray<FPolygon> &placed, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	//FRoomInfo r;

	FVector center = r2->getCenter();
	FVector rot = r2->getRoomDirection();
	rot.Normalize();
	FVector tan = FRotator(0, 90, 0).RotateVector(rot);
	tan.Normalize();


	FPolygon tableP = getPolygon(rot.Rotation(), center, "large_table", map);
	
	if (!testCollision(tableP, placed, 0, *r2)) {
		FMeshInfo table{ "large_table", FTransform(rot.Rotation() , center, FVector(1.0f, 1.0f, 1.0f)) };
		meshes.Add(table);
		placed.Add(tableP);
		attemptPlaceAroundPolygon(tableP, "chair", placed, meshes, FRotator(0, 0, 0), map, 0.007, *r2);

		if (FMath::FRand() < 0.35)
			attemptPlaceOnTop(table, meshes, "kettle", 50, map);
	}

	return meshes;

}

static FRoomInfo getLivingRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;
	TArray<FPolygon> placed;
	placed.Append(getBlockingVolumes(r2, 200, 200));
	FTransform res = r2->attemptGetPosition(placed, r.meshes, false, 3, "tv", FRotator(0, 0, 0), FVector(35, 0, 150), map, true);
	if (res.GetLocation().X != 0.0f) {
		r.meshes.Add({ "tv", res });
		placed.Add(getPolygon(res.Rotator(), res.GetLocation(), "tv", map));
		FTransform sofaTrans = FTransform{ res.GetRotation(), res.GetLocation() + res.GetRotation().RotateVector(FVector(270, 0, 0)), FVector(1.0f, 1.0f, 1.0f) };
		FPolygon pol = getPolygon(sofaTrans.Rotator() , sofaTrans.GetLocation(), "sofa", map);
		if (!testCollision(pol, placed, 0, *r2)) {
			placed.Add(pol);
			r.meshes.Add({ "sofa", FTransform{sofaTrans.Rotator(), sofaTrans.GetLocation() - FVector(0,0,140), FVector(1,1,1)} });
		}

	}
	r.meshes.Append(potentiallyGetTableAndChairs(r2, placed, map));

	return r;
}

static FRoomInfo getRestaurantRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;

	TArray<FPolygon> placed;
	placed.Append(getBlockingVolumes(r2, 200, 200));
	r2->attemptPlace(placed, r.meshes, false, 1, "restaurant_bar", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);
	TArray<FMeshInfo> tables;

	placeRows(r2, placed, tables, FRotator(0, 0, 0), "restaurant_table", FMath::FRandRange(0.0015, 0.003), FMath::FRandRange(0.0015, 0.003), map);
	//tables.RemoveAt(0, tables.Num() / 2);
	for (FMeshInfo table : tables) {
		FTransform trans = table.transform;
		if (FMath::RandBool())
			r.meshes.Add({ "restaurant_chair", trans });
		trans.SetRotation(FQuat(trans.Rotator() + FRotator(0, 120, 0)));
		if (FMath::RandBool())
			r.meshes.Add({ "restaurant_chair", trans });
		trans.SetRotation(FQuat(trans.Rotator() + FRotator(0, 120, 0)));
		if (FMath::RandBool())
			r.meshes.Add({ "restaurant_chair", trans });

	}
	if (FMath::FRand() < 0.3)
		r.meshes.Append(placeAwnings(r2, map));
	r.meshes.Append(tables);
	r.pols.Append(placeSigns(r2, FRandomStream(r2->points[0].X * 1000 + r2->points[0].Y * 100 + r2->points[0].Z)));
	return r;
}




static FRoomInfo getBathRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;
	TArray<FPolygon> placed;

	TArray<FPolygon> blocking = getBlockingVolumes(r2, 200, 100);
	placed.Append(blocking);
	r2->attemptPlace(placed, r.meshes, false, 2, "toilet" , FRotator(0, 270, 0), FVector(0, 0, 0), map, false);
	FTransform res = r2->attemptGetPosition(placed, r.meshes, false, 2, "sink", FRotator(0, 0, 0), FVector(0, 0, 0), map, false);
	if (res.GetLocation().X != 0.0f) {
		FPolygon pol = getPolygon(res.Rotator(), res.GetLocation(), "sink", map);
		placed.Add(pol);
		r.meshes.Add(FMeshInfo{ "sink", res});
		res.SetLocation(res.GetLocation() + FVector(0, 0, 150));
		//res.Ro
		r.meshes.Add(FMeshInfo{ "mirror", FTransform(res.Rotator() + FRotator(0, 270, 0), res.GetLocation() + FVector(0, 0, 55) - res.Rotator().Vector() * 40, FVector(1.0, 1.0, 1.0)) });
	}
	return r;
}


static FRoomInfo getBedRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;
	TArray<FPolygon> placed;
	//placed.Add(r2);
	placed.Append(getBlockingVolumes(r2, 200, 200));
	r2->attemptPlace(placed, r.meshes, true, 2, "bed", FRotator(0, 270, 0), FVector(0, 0, 60), map, false);
	r2->attemptPlace(placed, r.meshes, true, 1, "small_table", FRotator(0, 0, 0), FVector(0, 0, -50), map, false);
	r2->attemptPlace(placed, r.meshes, false, 1, "shelf", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	r2->attemptPlace(placed, r.meshes, false, 1, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);

	return r;
}

static FRoomInfo getHallWay(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;
	TArray<FPolygon> placed;

	r2->attemptPlace(placed, r.meshes, true, 1, "hanger", FRotator(0, 90, 0), FVector(0, 0, 10), map, false);
	r2->attemptPlace(placed, r.meshes, false, 1, "mirror2", FRotator(0, 0, 0), FVector(0, 0, 100), map, true);
	return r;
}

static FRoomInfo getKitchen(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	r2->attemptPlace(placed, r.meshes, false, 2, "kitchen", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);
	r.meshes.Append(potentiallyGetTableAndChairs(r2, placed, map));
	r2->attemptPlace(placed, r.meshes, false, 1, "shelf_upper_large", FRotator(0, 270, 0), FVector(0, 0, 200), map, false);
	r2->attemptPlace(placed, r.meshes, false, 1, "fridge", FRotator(0, 90, 0), FVector(0, 0, 0), map, true);
	r2->attemptPlace(placed, r.meshes, false, 1, "oven", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	return r;
}

static FRoomInfo getCorridor(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;
	TArray<FPolygon> placed;
	placed.Append(getBlockingVolumes(r2, 200, 100));
	r2->attemptPlace(placed, r.meshes, false, 1, "locker", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);
	if (r.meshes.Num() == 1 && FMath::FRand() < 0.2) {
		attemptPlaceOnTop(r.meshes[0], r.meshes, "vase", 50, map);

	}
	r2->attemptPlace(placed, r.meshes, false, 1, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 10), map, true);

	if (FMath::FRand() < 0.15)
		r2->attemptPlace(placed, r.meshes, false, 1, "mirror2", FRotator(0, 0, 0), FVector(0, 0, 100), map, true);


	return r;
}


static FRoomInfo getCloset(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	r2->attemptPlace(placed, r.meshes, false, 1, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 10), map, true);
	r2->attemptPlace(placed, r.meshes, false, 1, "shelf_upper_large", FRotator(0, 270, 0), FVector(0, 0, 200), map, true);


	return r;
}

static FRoomInfo getStoreFront(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	for (int i = 0; i < 5; i++) {
		r2->attemptPlace(placed, r.meshes, false, 1, "counter", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	}
	if (FMath::FRand() < 0.3)
		r.meshes.Append(placeAwnings(r2, map));

	placeRows(r2, placed, r.meshes, FRotator(0, 0, 0), "store_shelf", 0.003, 0.002, map);

	r.pols.Append(placeSigns(r2, FRandomStream(r2->points[0].X * 1000 + r2->points[0].Y * 100 + r2->points[0].Z)));
	return r;
}

static FRoomInfo getStoreBack(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));


	return r;
}


FRoomInfo ARoomBuilder::placeBalcony(FRoomPolygon *p, int place, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
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
	//side1.overridePolygonSides = true;
	FMaterialPolygon side2;
	side2.type = PolygonType::exteriorSnd;
	//side2.overridePolygonSides = true;
	FMaterialPolygon side3;
	side3.type = PolygonType::exteriorSnd;
	//side3.overridePolygonSides = true;

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
void ARoomBuilder::buildSpecificRoom(FRoomInfo &r, FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	switch (r2->type) {
	case SubRoomType::living: r.append(getLivingRoom(r2, map));
		break;
	case SubRoomType::bed: r.append(getBedRoom(r2, map));
		break;
	case SubRoomType::closet: r.append(getCloset(r2, map));
		break;
	case SubRoomType::corridor:	r.append(getCorridor(r2, map));
		break;
	case SubRoomType::kitchen: r.append(getKitchen(r2, map));
		break;
	case SubRoomType::bath: r.append(getBathRoom(r2, map));
		break;
	case SubRoomType::hallway: 	r.append(getHallWay(r2, map));
		break;
	case SubRoomType::meeting: r.append(getMeetingRoom(r2, map));
		break;
	case SubRoomType::work: r.append(getWorkingRoom(r2, map));
		break;
	case SubRoomType::restaurant: r.append(getRestaurantRoom(r2, map));
		break;
	case SubRoomType::storeFront: r.append(getStoreFront(r2, map));
		break;

	}
}
