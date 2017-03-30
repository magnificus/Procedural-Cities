// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "stdlib.h"
#include <queue>
#include "Spawner.generated.h"


USTRUCT(BlueprintType)
struct FRoadSegment
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector start;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector end;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float width;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector beginTangent;

};

struct logicRoadSegment {
	int time;
	FRoadSegment* segment;
	FRotator firstDegreeRot;
	FRotator secondDegreeRot;
	bool operator<(const logicRoadSegment& rhs) const {
		return rhs.time < this->time;
	}
};



UCLASS()
class CITY_API ASpawner : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		FVector stepLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		float changeIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = limits, meta = (AllowPrivateAccess = "true"))
		int32 length;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = limits, meta = (AllowPrivateAccess = "true"))
		float maxDist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		float minAttachDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		float mainRoadBranchChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		float secondaryRoadBranchChance;
	
public:	
	// Sets default values for this actor's properties
	ASpawner();


	void addRoadForward(std::priority_queue<logicRoadSegment*> &queue, logicRoadSegment* previous);
	void addRoadSide(std::priority_queue<logicRoadSegment*> &queue, logicRoadSegment* previous, bool left, float width);
	void addExtensions(std::priority_queue<logicRoadSegment*> &queue, logicRoadSegment* current);

	bool placementCheck(TArray<FRoadSegment> &segments, logicRoadSegment* current);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	TArray<FRoadSegment> determineRoadSegments();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
};
