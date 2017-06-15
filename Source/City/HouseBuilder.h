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

public:	
	// Sets default values for this actor's properties
	AHouseBuilder();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
	TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
	bool generateRoofs = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
	int makeInterestingAttempts = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ProcMesh)
		TSubclassOf<class AProcMeshActor> procMeshActorClass;

	AProcMeshActor *procMeshActor;
	bool firstTime = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Info)
		FHousePolygon housePol;


	UFUNCTION(BlueprintCallable, Category = "Generation")
	FHouseInfo getHouseInfoSimple(FHousePolygon f, float floorHeight, float maxRoomArea);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	FHouseInfo getHouseInfo(FHousePolygon f, float floorHeight, float maxRoomArea, bool shellOnly);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void buildHouse(FHousePolygon f, float floorHeight, float maxRoomArea, bool shellOnly, bool simple, bool fullReplacement);

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
