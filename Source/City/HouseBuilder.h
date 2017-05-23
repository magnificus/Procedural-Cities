// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
//#include "PlotBuilder.h"
#include "BaseLibrary.h"
#include "RoomBuilder.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "HouseBuilder.generated.h"


UCLASS()
class CITY_API AHouseBuilder : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AHouseBuilder();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
	TMap<FString, UHierarchicalInstancedStaticMeshComponent*> map;


	UFUNCTION(BlueprintCallable, Category = "Generation")
	FRoomInfo getHouseInfo(FHousePolygon f, int floors, float floorHeight, float maxRoomArea);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//virtual void BeginDestroy() override;

public:	

	//static TMap<FString, UHierarchicalInstancedStaticMeshComponent*> mapStatic;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
