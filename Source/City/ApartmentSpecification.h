// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomBuilder.h"
//#include "RoomBuilder.cpp"

/**
 * 
 */
class CITY_API ApartmentSpecification
{
public:
	ApartmentSpecification();
	~ApartmentSpecification();
	virtual RoomBlueprint getBlueprint(float areaScale) = 0;
	virtual FRoomInfo buildApartment(FRoomPolygon *f, int floor, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony, bool shellOnly, FRandomStream stream);
	virtual void intermediateInteractWithRooms(TArray<FRoomPolygon*> &roomPols, FRoomInfo &r, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony) {};
	void placeEntranceMeshes(FRoomInfo &r, FRoomPolygon *r2);
	virtual float getWindowDensity(FRandomStream stream) = 0;
	virtual float getWindowWidth(FRandomStream stream) = 0;
	virtual float getWindowHeight(FRandomStream stream) = 0;
};

class CITY_API OfficeSpecification : public ApartmentSpecification
{
public:
	RoomBlueprint getBlueprint(float areaScale);
	float getWindowDensity(FRandomStream stream) { return 1; }
	float getWindowWidth(FRandomStream stream) { return 190; }
	float getWindowHeight(FRandomStream stream) { return 320; }

};


class CITY_API LivingSpecification : public ApartmentSpecification
{
public:
	RoomBlueprint getBlueprint(float areaScale);
	void intermediateInteractWithRooms(TArray<FRoomPolygon*> &roomPols, FRoomInfo &r, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony);
	float getWindowDensity(FRandomStream stream) { return 0.003; }
	float getWindowWidth(FRandomStream stream) { return 200.0f; }
	float getWindowHeight(FRandomStream stream) { return 200.0f; }
};

class CITY_API StoreSpecification : public ApartmentSpecification
{
public:
	RoomBlueprint getBlueprint(float areaScale);
	void intermediateInteractWithRooms(TArray<FRoomPolygon*> &roomPols, FRoomInfo &r, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony);
	float getWindowDensity(FRandomStream stream) { return stream.FRandRange(200, 300); }
	float getWindowWidth(FRandomStream stream) { return stream.FRandRange(200, 300); }
	float getWindowHeight(FRandomStream stream) { return stream.FRandRange(200, 300); }
};

class CITY_API RestaurantSpecification : public ApartmentSpecification
{
public:
	RoomBlueprint getBlueprint(float areaScale);
	void intermediateInteractWithRooms(TArray<FRoomPolygon*> &roomPols, FRoomInfo &r, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony);
	float getWindowDensity(FRandomStream stream) { return stream.FRandRange(200, 300); }
	float getWindowWidth(FRandomStream stream) { return stream.FRandRange(200, 300); }
	float getWindowHeight(FRandomStream stream) { return stream.FRandRange(200, 300); }
};
