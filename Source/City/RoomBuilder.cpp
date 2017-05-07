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

TArray<FPolygon> ARoomBuilder::getSideWithHoles(FPolygon outer, TArray<FPolygon> holes) {

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
	polygons.Append(ARoomBuilder::getSideWithHoles(outer, holes));

	return polygons;
}


TArray<FPolygon> ARoomBuilder::interiorPlanToPolygons(TArray<FRoomPolygon> roomPols, float currentHeight, float floorHeight, float windowDensity, float windowHeight, float windowWidth) {
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
				toReturn.Append(getEntranceSide(rp.points[i - 1] + FVector(0, 0, currentHeight), rp.points[i] + FVector(0, 0, currentHeight), floorHeight, 300, 180));
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


static TArray<FMeshInfo> getMeetingRoom(FRoomPolygon r2, float beginning, float height) {
	TArray<FMeshInfo> meshes;

	FVector dir = r2.getRoomDirection();
	FVector center = r2.getCenter() + FVector(0, 0, beginning);
	meshes.Add(FMeshInfo{"office_meeting_table", FTransform(dir.Rotation(), center, FVector(1.0, 1.0, 1.0))});
	float offsetLen = 100;
	for (int i = 1; i < 4; i+=2) {
		FRotator curr = FRotator(0, 90 * i, 0);
		FVector chairPos = curr.Vector() * offsetLen + center;
		meshes.Add(FMeshInfo{ "office_meeting_chair", FTransform(curr, chairPos, FVector(1.0, 1.0, 1.0)) });
	}


	return meshes;
}

FRoomInfo ARoomBuilder::buildOffice(FRoomPolygon f, int floor, float beginning, float height) {

	float meetingRoomProb = 0.2;
	float workingRoomProb = 0.5;


	FRoomInfo r;
	//r.beginning = beginning;
	//r.height = height;
	TArray<FRoomPolygon> roomPols = f.refine(200, 0);
	for (FRoomPolygon r2 : roomPols) {
		//r.meshes.Add(FMeshInfo{ "bazinga", r2.getCenter() + FVector(0, 0, beginning)});
		r.meshes.Add(FMeshInfo{ "office_lamp", FTransform(r2.getCenter() + FVector(0, 0, beginning + height - 45))});
		if (randFloat() < meetingRoomProb) {
			r.meshes.Append(getMeetingRoom(r2, beginning, height));
		}
		else if (randFloat() < workingRoomProb) {

		}

	}
	r.pols.Append(interiorPlanToPolygons(roomPols, beginning, height, 0.004, 200, 300));

	return r;
}


FRoomInfo ARoomBuilder::buildRoom(FRoomPolygon f, RoomType type, int floor, float beginning, float height) {
	if (!f.canRefine) {
		FRoomInfo r;
		//r.beginning = beginning;
		//r.height = height;
		TArray<FRoomPolygon> pols;
		pols.Add(f);
		r.pols = interiorPlanToPolygons(pols, beginning, height, 0, 0, 0);
		return r;
	}
	switch (type) {
	case RoomType::office: return buildOffice(f, floor, beginning, height);
	}
	return FRoomInfo();
}
