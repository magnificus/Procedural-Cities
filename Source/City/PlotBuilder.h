// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "HouseBuilder.h"
//#include "BaseLibrary.h"
#include "PlotBuilder.generated.h"

USTRUCT(BlueprintType)
struct FSidewalkInfo {
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FMeshInfo> staticMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FMeshInfo> instancedMeshes;
};


UCLASS()
class CITY_API APlotBuilder : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	APlotBuilder();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
		TMap<FString, UHierarchicalInstancedStaticMeshComponent*> instancedMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
		TMap<FString, UStaticMesh*> staticMap;

	UFUNCTION(BlueprintCallable, Category = "Generation")
	static TArray<FMetaPolygon> sanityCheck(TArray<FMetaPolygon> plots, TArray<FPolygon> others);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	FPlotInfo generateHousePolygons(FPlotPolygon p, int minFloors, int maxFloors, float noiseScale);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	static FPolygon generateSidewalkPolygon(FPlotPolygon p, float offsetSize);
	
	UFUNCTION(BlueprintCallable, Category = "Generation")
	TArray<FMaterialPolygon> getSideWalkPolygons(FPlotPolygon p, float width);


	UFUNCTION(BlueprintCallable, Category = "Generation")
	static FSidewalkInfo getSideWalkInfo(FPolygon sidewalk);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	TArray<FMaterialPolygon> getSimplePlotPolygonsPB(TArray<FSimplePlot> plots);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	static FCityDecoration getCityDecoration(TArray<FMetaPolygon> plots, TArray<FPolygon> roads);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//virtual void BeginDestroy() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
