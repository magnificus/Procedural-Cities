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


static TArray<FMeshInfo> getMeetingRoom(FRoomPolygon r2, float beginning, float height) {
	TArray<FMeshInfo> meshes;

	FVector dir = r2.getRoomDirection();
	FVector center = r2.getCenter() + FVector(0, 0, beginning);
	meshes.Add(FMeshInfo{"office_meeting_table", FTransform(dir.Rotation(), center, FVector(1.0, 1.0, 1.0))});
	float offsetLen = 100;
	for (int i = 0; i < 4; i+=2) {
		FRotator curr = FRotator(0, 90 * i, 0);
		FVector chairPos = curr.Vector() * offsetLen + center;
		meshes.Add(FMeshInfo{ "office_meeting_chair", FTransform(curr, chairPos, FVector(1.0, 1.0, 1.0)) });
	}


	return meshes;
}

static RoomInfo buildOffice(FRoomPolygon f, int floor, float beginning, float height) {

	float meetingRoomProb = 0.2;
	float workingRoomProb = 0.5;


	RoomInfo r;
	r.beginning = beginning;
	r.height = height;
	r.rooms = f.refine(200, 0);
	for (FRoomPolygon r2 : r.rooms) {
		//r.meshes.Add(FMeshInfo{ "bazinga", r2.getCenter() + FVector(0, 0, beginning)});
		r.meshes.Add(FMeshInfo{ "office_lamp", FTransform(r2.getCenter() + FVector(0, 0, beginning + height - 45))});
		if (randFloat() < meetingRoomProb) {
			r.meshes.Append(getMeetingRoom(r2, beginning, height));
		}
		else if (randFloat() < workingRoomProb) {

		}
	}
	return r;
}


RoomInfo ARoomBuilder::buildRoom(FRoomPolygon f, RoomType type, int floor, float beginning, float height) {
	if (!f.canRefine) {
		RoomInfo r;
		r.beginning = beginning;
		r.height = height;
		TArray<FRoomPolygon> pols;
		pols.Add(f);
		r.rooms = pols;
		return r;
	}
	switch (type) {
	case RoomType::office: return buildOffice(f, floor, beginning, height);
	}
	return RoomInfo();
}
