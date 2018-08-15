#pragma once

#include "CoreMinimal.h"
#include "RoomBuilder.h"


/**
 * In here and in ApartmentSpecification.cpp the specifications for the different apartments are defined. Expanding the number of apartments should be easy by just following the same structure, make sure you use the new specifications when generating interiors in HouseBuilder as well.
 */
class CITY_API ApartmentSpecification
{
public:
	ApartmentSpecification();
	virtual ~ApartmentSpecification();
	virtual RoomBlueprint getBlueprint(float areaScale) = 0;
	virtual FRoomInfo buildApartment(FRoomPolygon *f, int floor, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony, bool shellOnly, FRandomStream stream);
	virtual void intermediateInteractWithRooms(TArray<FRoomPolygon*> &roomPols, FRoomInfo &r, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony) {};
	void placeEntranceMeshes(FRoomInfo &r, FRoomPolygon *r2);
	virtual float getWindowDensity(FRandomStream stream) = 0;
	virtual float getWindowWidth(FRandomStream stream) = 0;
	virtual float getWindowHeight(FRandomStream stream) = 0;
	virtual bool getWindowFrames() = 0;
	virtual float getMaxApartmentSize() = 0;
};

class CITY_API OfficeSpecification : public ApartmentSpecification
{
public:
	RoomBlueprint getBlueprint(float areaScale);
	float getWindowDensity(FRandomStream stream) { return 1; }
	float getWindowWidth(FRandomStream stream) { return stream.FRandRange(200, 400); }
	float getWindowHeight(FRandomStream stream) { return stream.FRandRange(200, 300); }
	bool getWindowFrames() { return false; }
	float getMaxApartmentSize() { return 600; }

};


class CITY_API LivingSpecification : public ApartmentSpecification
{
public:
	RoomBlueprint getBlueprint(float areaScale);
	void intermediateInteractWithRooms(TArray<FRoomPolygon*> &roomPols, FRoomInfo &r, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony);
	float getWindowDensity(FRandomStream stream) { return 0.003; }
	float getWindowWidth(FRandomStream stream) { return 200.0f; }
	float getWindowHeight(FRandomStream stream) { return 200.0f; }
	bool getWindowFrames() { return true;}
	float getMaxApartmentSize() { return 400; }

};

class CITY_API StoreSpecification : public ApartmentSpecification
{
public:
	RoomBlueprint getBlueprint(float areaScale);
	void intermediateInteractWithRooms(TArray<FRoomPolygon*> &roomPols, FRoomInfo &r, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony);
	float getWindowDensity(FRandomStream stream) { return 1; }
	float getWindowWidth(FRandomStream stream) { return stream.FRandRange(200, 400); }
	float getWindowHeight(FRandomStream stream) { return stream.FRandRange(200, 300); }
	bool getWindowFrames() { return true; }
	float getMaxApartmentSize() { return 5000; }


};

class CITY_API RestaurantSpecification : public ApartmentSpecification
{
public:
	RoomBlueprint getBlueprint(float areaScale);
	void intermediateInteractWithRooms(TArray<FRoomPolygon*> &roomPols, FRoomInfo &r, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony);
	float getWindowDensity(FRandomStream stream) { return 1; }
	float getWindowWidth(FRandomStream stream) { return stream.FRandRange(200, 400); }
	float getWindowHeight(FRandomStream stream) { return stream.FRandRange(200, 300); }
	bool getWindowFrames() { return true; }
	float getMaxApartmentSize() { return 5000; }

};
