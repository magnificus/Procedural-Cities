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
	virtual FRoomInfo buildApartment(FRoomPolygon *f, int floor, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony, bool shellOnly, FRandomStream stream) = 0;
	void placeEntranceMeshes(FRoomInfo r, FRoomPolygon *r2);
};

class CITY_API OfficeSpecification : public ApartmentSpecification
{
public:
	virtual RoomBlueprint getBlueprint(float areaScale);
	FRoomInfo buildApartment(FRoomPolygon *f, int floor, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony, bool shellOnly, FRandomStream stream);

};


class CITY_API LivingSpecification : public ApartmentSpecification
{
public:
	virtual RoomBlueprint getBlueprint(float areaScale);
	FRoomInfo buildApartment(FRoomPolygon *f, int floor, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony, bool shellOnly, FRandomStream stream);

};

class CITY_API StoreSpecification : public ApartmentSpecification
{
public:
	virtual RoomBlueprint getBlueprint(float areaScale);
	FRoomInfo buildApartment(FRoomPolygon *f, int floor, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony, bool shellOnly, FRandomStream stream);

};

class CITY_API RestaurantSpecification : public ApartmentSpecification
{
public:
	virtual RoomBlueprint getBlueprint(float areaScale);
	FRoomInfo buildApartment(FRoomPolygon *f, int floor, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony, bool shellOnly, FRandomStream stream);

};
