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


static RoomInfo buildOffice(FRoomPolygon f, int floor, float beginning, float height) {
	RoomInfo r;
	r.beginning = beginning;
	r.height = height;
	r.rooms = f.refine(200, 0);
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
