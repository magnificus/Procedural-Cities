// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
//#include "PlotBuilder.h"
#include "BaseLibrary.h"
#include "HouseBuilder.generated.h"



UCLASS()
class CITY_API AHouseBuilder : public AActor
{
	GENERATED_BODY()
	TArray<USplineMeshComponent*> splineComponents;

public:	
	// Sets default values for this actor's properties
	AHouseBuilder();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Demo, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* placeHolderHouseMesh;

	TArray<UStaticMeshComponent*> meshesArray;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = length, meta = (AllowPrivateAccess = "true"))
		USplineMeshComponent* base;
	UStaticMesh* meshPolygon;

	UFUNCTION(BlueprintCallable, Category = "Generation")
		TArray<FPolygon> getHousePolygons(FHousePolygon f, int floors, float floorHeight);

	//UPROPERTY(VisibleAnywhere, Category = Materials)
	//	UProceduralMeshComponent * mesh;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//virtual void BeginDestroy() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
