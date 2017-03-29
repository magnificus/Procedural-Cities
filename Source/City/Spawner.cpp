// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "Spawner.h"
#include "stdlib.h"


// Sets default values
ASpawner::ASpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}


float randFloat() {
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}
TArray<FRoadSegment> ASpawner::executeLSystem()
{
	FVector origin;

	TArray<FRoadSegment> segments;

	//.

	FRotator rot = FRotator(0, 0, 0);
	FRotator rot2nd = FRotator(0, 0, 0);
	FVector prevEnd = FVector(stepLength);
	FVector prevStart = FVector(0,0,0);

	for (int i = 0; i < length; i++) {
		rot2nd += FRotator(0, changeIntensity*(randFloat() - 0.5f), 0);
		rot += rot2nd;
		rot2nd.Yaw = rot2nd.Yaw > 10.0f ? 10.0f : rot2nd.Yaw;
		rot2nd.Yaw = rot2nd.Yaw < -10.0f ? -10.0f : rot2nd.Yaw;
		FRoadSegment f;
		f.beginTangent = prevEnd - prevStart;
		prevStart = prevEnd;
		f.start = prevEnd;
		f.end = f.start + rot.RotateVector(stepLength);
		prevEnd = f.end;

		f.width = 2;

		segments.Add(f);

	}
	
	return segments;
}

// Called when the game starts or when spawned
void ASpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

