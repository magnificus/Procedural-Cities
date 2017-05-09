// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BaseLibrary.h"
#include "RoomBuilder.generated.h"

USTRUCT(BlueprintType)
struct FMeshInfo {

	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform transform;
};

USTRUCT(BlueprintType)
struct FRoomInfo {

	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMaterialPolygon> pols;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMeshInfo> meshes;



	void offset(FVector offset) {
		for(FPolygon &p : pols)
			p.offset(offset);
		for (FMeshInfo &f : meshes)
			f.transform.SetTranslation(f.transform.GetTranslation() + offset);
	}

	//float beginning;
	//float height;

	//void Add(FRoomInfo r) {
	//	pols.Append(r.pols);
	//	pols.Append(r.meshes);
	//}
};
UCLASS()
class CITY_API ARoomBuilder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoomBuilder();

	static TArray<FMaterialPolygon> interiorPlanToPolygons(TArray<FRoomPolygon> roomPols, float floorHeight, float windowDensity, float windowHeight, float windowWidth);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	static FRoomInfo buildOffice(FRoomPolygon f, int floor, float height, float density, float windowHeight, float windowWidth);
	static FRoomInfo buildRoom(FRoomPolygon f, RoomType type, int floor, float height, float density, float windowHeight, float windowWidth);
	static TArray<FMaterialPolygon> getSideWithHoles(FMaterialPolygon outer, TArray<FPolygon> holes);
	
};
