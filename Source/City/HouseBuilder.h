// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
//#include "PlotBuilder.h"
#include "BaseLibrary.h"
#include "RoomBuilder.h"
#include "HouseBuilder.generated.h"




USTRUCT(BlueprintType)
struct FHouseInfo {
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPolygon> polygons;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMeshInfo> meshes;
};

UCLASS()
class CITY_API AHouseBuilder : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AHouseBuilder();

	TArray<UStaticMeshComponent*> meshesArray;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = length, meta = (AllowPrivateAccess = "true"))
		USplineMeshComponent* base;
	UStaticMesh* meshPolygon;

	UFUNCTION(BlueprintCallable, Category = "Generation")
		TArray<FPolygon> getHousePolygons(FHousePolygon f, int floors, float floorHeight);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//virtual void BeginDestroy() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
