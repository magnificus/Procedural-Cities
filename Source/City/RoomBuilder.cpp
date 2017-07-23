// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "polypartition.h"
#include "gpc.h"
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

	FVector e1 = outer.points[1] - outer.points[0];
	e1.Normalize();
	FVector n = FVector::CrossProduct(e1, outer.points[2] - outer.points[0]);
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

//TArray<FMaterialPolygon> ARoomBuilder::getSideWithHoles(FPolygon outer, TArray<FPolygon> holes, PolygonType type) {
//
//	TArray<FMaterialPolygon> polygons;
//	FVector e1 = outer.points[1] - outer.points[0];
//	e1.Normalize();
//	FVector n = FVector::CrossProduct(e1, outer.points[2] - outer.points[0]);
//	n.Normalize();
//	FVector e2 = FVector::CrossProduct(e1, n);
//	e2.Normalize();
//
//	FVector origin = outer.points[0];
//	int current = 0;
//
//	gpc_polygon outerP;
//	gpc_vertex_list* outerPList = new gpc_vertex_list();
//	outerPList->num_vertices = outer.points.Num();
//	outerPList->vertex = new gpc_vertex[outer.points.Num()];
//	for (int i = 0; i < outer.points.Num(); i++) {
//		FVector point = outer.points[i];
//		float y = FVector::DotProduct(e1, point - origin);
//		float x = FVector::DotProduct(e2, point - origin);
//		outerPList->vertex[i] = gpc_vertex{ x,y };
//	}
//	outerP.num_contours = 1;
//	outerP.contour = outerPList;
//
//
//	gpc_polygon holesP;// = new gpc_polygon();
//
//	holesP.hole = new int[holes.Num()];
//	for (int i = 0; i < holes.Num(); i++)
//		holesP.hole[i] = 1;
//	holesP.num_contours = holes.Num();
//	holesP.contour = new gpc_vertex_list[holes.Num()];
//	for (int j = 0; j < holes.Num(); j++) {
//		FPolygon p = holes[j];
//		gpc_vertex_list currHoleList;
//		currHoleList.num_vertices = p.points.Num();
//		gpc_vertex* currVerts = new gpc_vertex[p.points.Num()];
//		for (int i = 0; i < p.points.Num(); i++) {
//			FVector point = p.points[i];
//			float x = FVector::DotProduct(e1, point - origin);
//			float y = FVector::DotProduct(e2, point - origin);
//			currVerts[i] = gpc_vertex{ x,y };
//			//holeP[p.points.Num() - 1 - i] = TPPLPoint{ x, y, current++ };
//			//allPoints.Add(point);
//		}
//		currHoleList.vertex = currVerts;
//		holesP.contour[j] = currHoleList;
//		//holeP.SetOrientation(TPPL_CW);
//		//inPolys.push_back(holeP);
//	}
//	gpc_polygon resultP;// = new gpc_polygon();
//	gpc_polygon_clip(GPC_DIFF, &outerP, &holesP, &resultP);
//
//
//	for (int i = 0; i < resultP.num_contours; i++) {
//			//if (resultP.hole[i] == 1)
//			//	continue;
//		FMaterialPolygon newPolygon;
//		newPolygon.type = type;
//		for (int j = 0; j < resultP.contour[i].num_vertices; j++) {
//			gpc_vertex currVertex = resultP.contour[i].vertex[j];
//			newPolygon.points.Add(origin + currVertex.x * e1 + currVertex.y * e2);
//		}
//		if (newPolygon.points.Num() > 2)
//			polygons.Add(newPolygon);
//	}
//
//	//TPPLPartition part;
//	//part.RemoveHoles(&inPolys, &outPolys);
//	//for (TPPLPoly t : outPolys) {
//	//	FMaterialPolygon newP;
//	//	for (int i = 0; i < t.GetNumPoints(); i++) {
//	//		newP.points.Add(allPoints[t[i].id]);
//	//	}
//	//	newP.type = type;
//	//	polygons.Add(newP);
//	//	//newP.
//	//}
//
//
//	//gpc_free_polygon(outerP);
//	//gpc_free_polygon(holesP);
//
//	return polygons;
//}

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


FPolygon getEntranceHole(FVector p1, FVector p2, float floorHeight, float doorHeight, float doorWidth, FVector doorPos) {
	FVector side = p2 - p1;
	float sideLen = side.Size();
	side.Normalize();
	float distToDoor = FVector::Dist(doorPos, p1) - doorWidth / 2;
	FMaterialPolygon doorPolygon;
	doorPolygon.points.Add(p1 + side*distToDoor + FVector(0, 0, doorHeight));
	doorPolygon.points.Add(p1 + side*distToDoor + FVector(0, 0, 4));// + FVector(0, 0, 100));
	doorPolygon.points.Add(p1 + side*distToDoor + side*doorWidth + FVector(0, 0, 4));// + FVector(0, 0, 100));
	doorPolygon.points.Add(p1 + side*distToDoor + side*doorWidth + FVector(0, 0, doorHeight));
	//doorPolygon.points.Add(p1 + side*distToDoor + FVector(0, 0, doorHeight));
	//doorPolygon.reverse();
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
		frame.points.Add(middle(p.points[j - 1], p.points[j]) - tangent1 * frameWidth / 2);
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
		frame.points.Add(p.points[j - 1]);
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
		for (int i = 1; i < rp->points.Num(); i++) {
			if (rp->toIgnore.Contains(i) || (shellOnly && !rp->exteriorWalls.Contains(i))) {
				continue;
			}
			FMaterialPolygon newP;
			newP.type = rp->exteriorWalls.Contains(i) ? PolygonType::exterior : PolygonType::interior;
			FVector p1 = rp->points[i - 1];
			FVector p2 = rp->points[i];

			// this extra offset is to avoid z-fighting between walls
			FVector tan = p2 - p1;
			tan.Normalize();
			FVector extraBack = FVector(0, 0, 0);
			FVector extraFront = FVector(0, 0, 0);

			int prev = i > 1 ? i - 1 : rp->points.Num() - 1;
			int next = i < rp->points.Num() - 1 ? i + 1 : 1;
			float extraBottom = 0;
			if (!rp->exteriorWalls.Contains(i)) {
				// this means that prev wall was exterior, move our back a little forward
				if (rp->exteriorWalls.Contains(prev))
					extraFront = tan * 20;

				// this means that next wall is exterior, move our front a little back
				if (rp->exteriorWalls.Contains(next))
					extraBack = -tan * 20;
				extraBottom = 2;
			}


			
			newP.points.Add(p1 + FVector(0, 0, floorHeight) + extraFront);
			newP.points.Add(p1 + FVector(0, 0, extraBottom) + extraFront);
			newP.points.Add(p2 + FVector(0, 0, extraBottom) + extraBack);
			newP.points.Add(p2 + FVector(0, 0, floorHeight) + extraBack);
			newP.points.Add(p1 + FVector(0, 0, floorHeight) + extraFront);

			TArray<FPolygon> holes;
			FVector entrancePos = FVector(-10000, -10000, -10000);
			float doorStart = 100000;
			float doorEnd = -1000000;
			if (rp->entrances.Contains(i) && FVector::Dist(rp->points[i-1], rp->points[i]) > 150) {
				entrancePos = rp->specificEntrances.Contains(i) ?  rp->specificEntrances[i] : middle(rp->points[i - 1], rp->points[i]);
				doorStart = FVector::Dist(rp->points[i - 1], entrancePos) - 137/2;
				doorEnd = doorStart + 137;
				holes.Add(getEntranceHole(rp->points[i - 1], rp->points[i], floorHeight, 297, 137, entrancePos));
			}
			if (rp->windows.Contains(i)) {
				FVector tangent = rp->points[i] - rp->points[i - 1];
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


FTransform attemptGetPosition(FRoomPolygon *r2, TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes, bool windowAllowed, int testsPerSide, FString string, FRotator offsetRot, FVector offsetPos, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map, bool onWall) {
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
			FVector pos = origin + dir + offsetPos;// *verticalOffset + offsetPos;
			FRotator rot = dir.Rotation() + offsetRot;
			FPolygon pol = getPolygon(rot, pos, string, map);

			// fit the polygon properly if possible
			float lenToMove = 0;
			for (int k = 1; k < pol.points.Num(); k++) {
				FVector res = intersection(pol.points[k - 1], pol.points[k], r2->points[place], r2->points[place - 1]);
				if (res.X != 0.0f) {
					float currentToMove = FVector::Dist(pol.points[k - 1], pol.points[k]) - FVector::Dist(pol.points[k - 1], res);
					lenToMove = std::max(lenToMove, currentToMove);
					//pos += dir * (lenToMove+5);
					//break;
				}
			}
			if (lenToMove != 0) {
				pos += (lenToMove + 30)*dir;
				pol = getPolygon(rot, pos, string, map);
			}
			if (onWall) {
				// at least two points has to be next to the wall
				int count = 0;
				for (int k = 1; k < pol.points.Num(); k++) {
					FVector curr = NearestPointOnLine(r2->points[i - 1], r2->points[i] - r2->points[i - 1], pol.points[k]);
					float dist = FMath::Pow(curr.X - pol.points[k].X, 2) + FMath::Pow(curr.Y - pol.points[k].Y, 2);
					if (dist < 1500) { // pick a number that seems reasonable
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
/**
A simplified method for trying to find a wall in a room to place the mesh
@param r2 the room to place mesh in
@param placed other placed polygons, to check for collision
@param meshes the array to which append the mesh if sucessful
@param windowAllowed whether to allow the mesh to be placed in front of windows or not
@param testsPerSide number of tries for placement on each side of the room
@param string name of mesh in instanced mesh map
@param offsetRot offset rotation for object
@param offsetPos offset position for object'
@param map the instanced mesh map for finding the bounding box for the object
@param onwall whether the object is required to be strictly next to the wall (and not sticking out)
@return whether the placement was successful or not

*/
bool attemptPlace(FRoomPolygon *r2, TArray<FPolygon> &placed, TArray<FMeshInfo> &meshes, bool windowAllowed, int testsPerSide, FString string, FRotator offsetRot,FVector offsetPos, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map, bool onWall) {
	FTransform pos = attemptGetPosition(r2, placed, meshes, windowAllowed, testsPerSide, string, offsetRot, offsetPos, map, onWall);
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
	if (res.min == 0) {
		return;
	}
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
	TArray<FPolygon> toPlace;
	for (int i = 1; i < numWidth; i++) {
		for (int j = 1; j < numHeight; j++) {
			FPolygon pol = getPolygon(normal.Rotation(), origin + i*intervalWidth*tangent + j*intervalHeight*normal, name, map);
			// make sure it's fully inside the room
			if (!testCollision(pol, placed, 0, *r2)){// && intersection(pol, *r2).X == 0.0f) {
				//placed.Add(pol);
				toPlace.Add(pol);
				meshes.Add(FMeshInfo{ name, FTransform(normal.Rotation(),origin + i*intervalWidth*tangent + j*intervalHeight*normal, FVector(1.0f, 1.0f, 1.0f)), true});
			}
		}
	}
	placed.Append(toPlace);
	//FVector start =
}



static TArray<FMeshInfo> getMeetingRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed = getBlockingVolumes(r2, 200, 200);
	FVector dir = r2->getRoomDirection();
	FVector center = r2->getCenter();
	meshes.Add(FMeshInfo{"office_meeting_table", FTransform(dir.Rotation(), center + FVector(0, 0, 2), FVector(1.0, 1.0, 1.0))});
	float offsetLen = 100;
	//for (int i = 1; i < 4; i+=2) {
	//	FRotator curr = FRotator(0, 90 * i, 0);
	//	FVector chairPos = curr.Vector() * offsetLen + center + FVector(0, 0, 2);
	//	meshes.Add(FMeshInfo{ "office_meeting_chair", FTransform(curr.GetInverse(), chairPos, FVector(1.0, 1.0, 1.0)) });
	//}

	if (randFloat() < 0.5) {
		attemptPlace(r2, placed, meshes, false, 1, "shelf", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	}

	if (randFloat() < 0.5) {
		// add whiteboard
		attemptPlace(r2, placed, meshes, false, 1, "office_whiteboard", FRotator(0, 180, 0), FVector(0, 0, 100), map, true);
	}

	attemptPlace(r2, placed, meshes, true, 1, "dispenser", FRotator(0, 0, 0), FVector(0, 0, 0), map, false);

	return meshes;
}



static TArray<FMeshInfo> getWorkingRoom(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;
	placed = getBlockingVolumes(r2, 200, 200);
	placeRows(r2, placed, meshes, FRotator(0, 180, 0), "office_cubicle", 0.0055, 0.004, map);
	//meshes.RemoveAt(meshes.Num() / 3, meshes.Num() / 3);
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

	RoomSpecification workRoom{ 100 * areaScale, 200 * areaScale, SubRoomType::work };
	TArray<RoomSpecification> optional;
	optional.Add(workRoom);
	optional.Add(workRoom);
	optional.Add(workRoom);
	needed.Add(bathroom);
	optional.Add(workRoom);

	return RoomBlueprint{ needed, optional };
}

RoomBlueprint getRestaurantBlueprint(float areaScale) {
	// one meeting room, rest working rooms
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
	FTransform res = attemptGetPosition(r2, placed, meshes, false, 3, "tv", FRotator(0, 0, 0), FVector(35, 0, 150), map, true);
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
	attemptPlace(r2, placed, meshes, false, 1, "restaurant_bar", FRotator(0, 0, 0), FVector(200, 0, 0), map, true);
	//attemptPlace(r2, placed, meshes, true, 5, "restaurant_table", FRotator(0, 0, 0), FVector(0, 0, 0), map, false);
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
	attemptPlace(r2, placed, meshes, false, 2, "toilet" , FRotator(0, 270, 0), FVector(0, 0, 0), map, false);
	FTransform res = attemptGetPosition(r2, placed, meshes, false, 2, "sink", FRotator(0, 0, 0), FVector(0, 0, 0), map, false);
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
	attemptPlace(r2, placed, meshes, true, 2, "bed", FRotator(0, 270, 0), FVector(0, 40, 70), map, false);
	attemptPlace(r2, placed, meshes, true, 1, "small_table", FRotator(0, 0, 0), FVector(0, 0, -50), map, false);
	attemptPlace(r2, placed, meshes, false, 1, "shelf", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	attemptPlace(r2, placed, meshes, false, 1, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);

	return meshes;
}

static TArray<FMeshInfo> getHallWay(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	attemptPlace(r2, placed, meshes, true, 1, "hanger", FRotator(0, 0, 0), FVector(100, 0, 20), map, false);
	return meshes;
}

static TArray<FMeshInfo> getKitchen(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	attemptPlace(r2, placed, meshes, false, 2, "kitchen", FRotator(0, 90, 0), FVector(-15, 0, 0), map, true);
	meshes.Append(potentiallyGetTableAndChairs(r2, placed, map));
	attemptPlace(r2, placed, meshes, false, 1, "shelf_upper_large", FRotator(0, 270, 0), FVector(0, 0, 200), map, false);
	attemptPlace(r2, placed, meshes, false, 1, "fridge", FRotator(0, 90, 0), FVector(0, 0, 0), map, true);
	attemptPlace(r2, placed, meshes, false, 1, "oven", FRotator(0, 270, 0), FVector(0, 0, 0), map, true);
	return meshes;
}

static TArray<FMeshInfo> getCorridor(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;
	placed.Append(getBlockingVolumes(r2, 200, 100));
	attemptPlace(r2, placed, meshes, false, 1, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);

	return meshes;
}


static TArray<FMeshInfo> getCloset(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	attemptPlace(r2, placed, meshes, false, 1, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);
	attemptPlace(r2, placed, meshes, false, 1, "shelf_upper_large", FRotator(0, 270, 0), FVector(0, 0, 200), map, true);


	return meshes;
}

static TArray<FMeshInfo> getStoreFront(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	for (int i = 0; i < 5; i++) {
		attemptPlace(r2, placed, meshes, false, 1, "counter", FRotator(0, 270, 0), FVector(40, 0, 0), map, true);
	}
	//attemptPlace(r2, placed, meshes, 50, false, 5, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);
	///attemptPlace(r2, placed, meshes, 45.0f, false, 1, "shelf_upper_large", FRotator(0, 270, 0), FVector(0, 0, 200), map, true);


	return meshes;
}

static TArray<FMeshInfo> getStoreBack(FRoomPolygon *r2, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	TArray<FMeshInfo> meshes;
	TArray<FPolygon> placed;

	placed.Append(getBlockingVolumes(r2, 200, 100));
	//attemptPlace(r2, placed, meshes, 50, false, 5, "wardrobe", FRotator(0, 0, 0), FVector(0, 0, 0), map, true);
	//attemptPlace(r2, placed, meshes, 45.0f, false, 1, "shelf_upper_large", FRotator(0, 270, 0), FVector(0, 0, 200), map, true);


	return meshes;
}


FRoomInfo placeBalcony(FRoomPolygon *p, int place, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map) {
	FRoomInfo r;

	float width = 500;
	float length = 200;
	float height = 150;

	FVector tangent = p->points[place] - p->points[place - 1];
	FVector normal = getNormal(p->points[place], p->points[place - 1], false);
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
	floor.points.Add(start);

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
			FVector dir1 = getNormal(r2->points[i], r2->points[i - 1], true);
			dir1.Normalize();
			FVector dir2 = r2->points[i] - r2->points[i - 1];
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

	return r;
}

FRoomInfo ARoomBuilder::buildRestaurant(FRoomPolygon *f, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool shellOnly, FRandomStream stream) {
	FRoomInfo r;
	TArray<FRoomPolygon*> roomPols = f->getRooms(getRestaurantBlueprint(1.0f));
	for (FRoomPolygon* r2 : roomPols) {
		for (int i : r2->windows) {
			if (FVector::Dist(f->points[i], f->points[i-1]) > 150.0f)
			r2->entrances.Add(i);
		}
		for (int i : r2->entrances) {
			FVector doorPos = r2->specificEntrances.Contains(i) ? r2->specificEntrances[i] : middle(r2->points[i], r2->points[i - 1]);
			FVector dir1 = getNormal(r2->points[i], r2->points[i - 1], true);
			dir1.Normalize();
			FVector dir2 = r2->points[i] - r2->points[i - 1];
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
			FVector doorPos = r2->specificEntrances.Contains(i) ? r2->specificEntrances[i] : middle(r2->points[i], r2->points[i - 1]);
			FVector dir1 = getNormal(r2->points[i], r2->points[i - 1], true);
			dir1.Normalize();
			FVector dir2 = r2->points[i] - r2->points[i - 1];
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
						if (FVector::DistSquared(p->points[place], p->points[place - 1]) > 10000) {
							p->entrances.Add(place);
							FVector mid = middle(p->points[place], p->points[place - 1]);
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
				FVector dir1 = getNormal(r2->points[i], r2->points[i - 1], true);
				dir1.Normalize();
				FVector dir2 = r2->points[i] - r2->points[i - 1];
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

	//TArray<FRoomPolygon> toReturn;

	//for (FRoomPolygon *p : roomPols) {
	//	toReturn.Add(*p);
	//}


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
	case RoomType::office: return buildOffice(f, floor, height, 1/* 0.0042*/, 340.0f, 190.0f, map, shellOnly);
	case RoomType::apartment: return buildApartment(f, floor, height, 0.003, 200.0f, 200.0f, map, potentialBalcony, shellOnly);
	case RoomType::store: return buildStore(f, height, map, shellOnly);
	case RoomType::restaurant: return buildRestaurant(f, height, map, shellOnly, stream);
	}
	return FRoomInfo();
}
