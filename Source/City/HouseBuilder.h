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
	
public:	
	// Sets default values for this actor's properties
	AHouseBuilder();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Demo, meta = (AllowPrivateAccess = "true"))
		UStaticMesh* placeHolderHouseMesh;

	UFUNCTION(BlueprintCallable, Category = "Generation")
		void placeHouse(FHousePolygon f);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
