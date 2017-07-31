	// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ProcMeshActor.h"
#include "BaseLibrary.h"
#include "RoomBuilder.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "ThreadedWorker.h"

#include "HouseBuilder.generated.h"

class ThreadedWorker;

UCLASS()
class CITY_API AHouseBuilder : public AActor
{
	GENERATED_BODY()

	FHousePolygon f;
	float floorHeight = 400.0;
	float maxRoomArea = 500;
	float maxHouseArea = 2000.0f;

	float minHouseArea = 100.0f;

	int makeInterestingAttempts = 4;

	bool generateRoofs = true;


public:
	// Sets default values for this actor's properties
	AHouseBuilder();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
		TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
		TMap<FString, UStaticMesh*> staticMap;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ProcMesh)
		TSubclassOf<class AProcMeshActor> procMeshActorClass;

	AProcMeshActor *procMeshActor;
	bool firstTime = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Info)
		FHousePolygon housePol;


	UFUNCTION(BlueprintCallable, Category = "Generation")
		void init(FHousePolygon f, float floorHeight, float maxRoomArea, float minHouseArea, int makeInterestingAttempts, bool generateRoofs){
		this->f = f;
		this->floorHeight = floorHeight;
		this->maxRoomArea = maxRoomArea;
		this->minHouseArea = minHouseArea;
		this->makeInterestingAttempts = makeInterestingAttempts;
		this->floorHeight = floorHeight;

	}


	UFUNCTION(BlueprintCallable, Category = "Generation")
	FHouseInfo getHouseInfoSimple();

	UFUNCTION(BlueprintCallable, Category = "Generation")
	FHouseInfo getHouseInfo(bool shellOnly);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void buildHouse(bool shellOnly, bool simple, bool fullReplacement);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void buildHouseFromInfo(FHouseInfo res, bool fullReplacement);

	static void makeInteresting(FHousePolygon &f, TArray<FSimplePlot> &toReturn, FPolygon &centerHole, FRandomStream stream);

	static TArray<FMaterialPolygon> getShaftSides(FPolygon hole, int openSide, float height);

	static FPolygon getShaftHolePolygon(FHousePolygon f);

	ThreadedWorker *worker;

	bool workerWorking = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//virtual void BeginDestroy() override;

public:	

	//static TMap<FString, UHierarchicalInstancedStaticMeshComponent*> mapStatic;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
