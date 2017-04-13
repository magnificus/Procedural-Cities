// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "HouseBuilder.h"
#include "BaseLibrary.h"
#include "PlotBuilder.generated.h"



struct FPolygon;



UCLASS()
class CITY_API APlotBuilder : public AActor
{
	GENERATED_BODY()
	TArray<AHouseBuilder*> houses;

public:	
	// Sets default values for this actor's properties
	APlotBuilder();

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void BuildPlot(FPlotPolygon p);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//virtual void BeginDestroy() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
