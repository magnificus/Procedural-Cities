// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MeshPolygonReference.h"
#include "RoomBuilder.generated.h"





UCLASS()
class CITY_API ARoomBuilder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoomBuilder();

	static TArray<FMaterialPolygon> interiorPlanToPolygons(TArray<FRoomPolygon*> roomPols, float floorHeight, float windowDensity, float windowHeight, float windowWidth, int floor, bool shellOnly, bool windowFrames);

	float areaScale = 1.0f;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	static FRoomInfo buildOffice(FRoomPolygon *f, int floor, float height, float density, float windowHeight, float windowWidth,TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool shellOnly);
	static FRoomInfo buildApartment(FRoomPolygon *f, int floor, float height, float density, float windowHeight, float windowWidth,TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool balcony, bool shellOnly);
	static FRoomInfo buildStore(FRoomPolygon *f, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool shellOnly);
	static FRoomInfo buildRestaurant(FRoomPolygon *f, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool shellOnly, FRandomStream stream);

	static FRoomInfo buildRoom(FRoomPolygon *f, RoomType type, int floor, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony, bool shellOnly, FRandomStream stream);
	static TArray<FMaterialPolygon> getSideWithHoles(FPolygon outer, TArray<FPolygon> holes, PolygonType type);
	
};
