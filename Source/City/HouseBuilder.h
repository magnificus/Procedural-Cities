	// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ProcMeshActor.h"
#include "ApartmentSpecification.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "ThreadedWorker.h"
#include "atomic"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = performance, meta = (AllowPrivateAccess = "true"))
	GenerationMode generationMode;

	unsigned int maxThreads = 1;

	bool shellOnly = false;

	bool wantsToWork = false;
	bool isWorking = false;
	int currentMapIndex = 0;
	TArray<UTextRenderComponent*> texts;

public:
	static std::atomic<unsigned int> housesWorking;

	static std::atomic<unsigned int> workersWorking;

	// Sets default values for this actor's properties
	AHouseBuilder();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
		TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
	//	TMap<FString, UStaticMesh*> staticMap;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ProcMesh)
		TSubclassOf<class AProcMeshActor> procMeshActorClass;

	AProcMeshActor *procMeshActor;
	bool firstTime = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Info)
		FHousePolygon housePol;


	UFUNCTION(BlueprintCallable, Category = "Generation")
		void init(FHousePolygon f_in, float floorHeight_in, float maxRoomArea_in, float minHouseArea_in, int makeInterestingAttempts_in, bool generateRoofs_in, GenerationMode generationMode_in){
		f = f_in;
		floorHeight = floorHeight_in;
		maxRoomArea = maxRoomArea_in;
		minHouseArea = minHouseArea_in;
		makeInterestingAttempts = makeInterestingAttempts_in;
		floorHeight = floorHeight_in;
		generateRoofs = generateRoofs_in;
		generationMode = generationMode_in;
		switch (generationMode) {
		case GenerationMode::complete: maxThreads = 1000; break;
		case GenerationMode::procedural_aggressive: maxThreads = 1000; break;
		case GenerationMode::procedural_relaxed: maxThreads = 2; break;
		}
	}


	//UFUNCTION(BlueprintCallable, Category = "Generation")
	//FHouseInfo getHouseInfoSimple();

	UFUNCTION(BlueprintCallable, Category = "Generation")
	FHouseInfo getHouseInfo();

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void buildHouse(bool shellOnly);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void buildHouseFromInfo(FHouseInfo res);

	static void makeInteresting(FHousePolygon &f, TArray<FSimplePlot> &toReturn, FPolygon &centerHole, FRandomStream stream);

	static TArray<FMaterialPolygon> getShaftSides(FPolygon hole, int openSide, float height);

	static FPolygon getShaftHolePolygon(FHousePolygon f);

	ThreadedWorker *worker;

	bool workerWorking = false;
	bool workerWantsToWork = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//virtual void BeginDestroy() override;

public:	


	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};

