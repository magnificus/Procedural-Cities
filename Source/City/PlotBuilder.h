// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "HouseBuilder.h"
#include "BaseLibrary.h"
#include "PlotBuilder.generated.h"

UCLASS()
class CITY_API APlotBuilder : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	APlotBuilder();

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = sidewalk, meta = (AllowPrivateAccess = "true"))
	//	float	offsetSize = 500;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = sidewalk, meta = (AllowPrivateAccess = "true"))
	//	int32	minFloors = 3;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = sidewalk, meta = (AllowPrivateAccess = "true"))
	//	int32	maxFloors = 15;

	//UFUNCTION(BlueprintCallable, Category = "Generation")
	//TArray<FHousePolygon> getHousePolygons(FPlotPolygon p);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	static TArray<FHousePolygon> generateHousePolygons(FPlotPolygon p, TArray<FPolygon> others, int minFloors, int maxFloors);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	static FPolygon generateSidewalkPolygon(FPlotPolygon p, float offsetSize);

	//UFUNCTION(BlueprintCallable, Category = "Generation")
	//void BuildPlot(FPlotPolygon p);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//virtual void BeginDestroy() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
