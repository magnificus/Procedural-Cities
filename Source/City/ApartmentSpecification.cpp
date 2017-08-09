// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "ApartmentSpecification.h"

ApartmentSpecification::ApartmentSpecification()
{
}

ApartmentSpecification::~ApartmentSpecification()
{
}

FRoomInfo ApartmentSpecification::buildApartment(FRoomPolygon *f, int floor, float height, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony, bool shellOnly, FRandomStream stream) {
	FRoomInfo r;
	if (!f->canRefine) {
		TArray<FRoomPolygon*> pols;
		pols.Add(f);
		r.pols = ARoomBuilder::interiorPlanToPolygons(pols, height, getWindowDensity(stream), getWindowHeight(stream), getWindowWidth(stream), floor, shellOnly, getWindowFrames());
		return r;
	}

	TArray<FRoomPolygon*> roomPols = f->getRooms(getBlueprint(1.0f));
	intermediateInteractWithRooms(roomPols, r, map, potentialBalcony);
	for (FRoomPolygon *r2 : roomPols) {
		if (!shellOnly) {
			ARoomBuilder::buildSpecificRoom(r, r2, map);
			placeEntranceMeshes(r, r2);
		}
	}
	r.pols.Append(ARoomBuilder::interiorPlanToPolygons(roomPols, height, getWindowDensity(stream), getWindowHeight(stream), getWindowWidth(stream), floor, shellOnly, getWindowFrames()));
	for (FRoomPolygon* roomP : roomPols)
		delete(roomP);
	return r;
}

void ApartmentSpecification::placeEntranceMeshes(FRoomInfo &r, FRoomPolygon *r2) {
	for (int i : r2->entrances) {
		if (FVector::DistSquared(r2->points[i%r2->points.Num()], r2->points[i - 1]) < 22500)
			continue;
		FVector doorPos = r2->specificEntrances.Contains(i) ? r2->specificEntrances[i] : middle(r2->points[i%r2->points.Num()], r2->points[i - 1]);
		FVector dir1 = getNormal(r2->points[i%r2->points.Num()], r2->points[i - 1], true);
		dir1.Normalize();
		FVector dir2 = r2->points[i%r2->points.Num()] - r2->points[i - 1];
		dir2.Normalize();
		r.meshes.Add(FMeshInfo{ "door_frame", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });
		//r.meshes.Add(FMeshInfo{ "door", FTransform(dir1.Rotation(), doorPos + dir1 * 10, FVector(1.0f, 1.0f, 1.0f)) });
	}
}

RoomBlueprint OfficeSpecification::getBlueprint(float areaScale) {
	// one meeting room, rest working rooms
	TArray<RoomSpecification> needed;
	RoomSpecification meetingRoom{ 100 * areaScale, 300 * areaScale, SubRoomType::meeting };
	needed.Add(meetingRoom);
	RoomSpecification bathroom{ 30 * areaScale, 60 * areaScale, SubRoomType::bath };
	RoomSpecification workRoom{ 50 * areaScale, 300 * areaScale, SubRoomType::work };
	TArray<RoomSpecification> optional;
	optional.Add(workRoom);
	optional.Add(workRoom);
	optional.Add(workRoom);
	needed.Add(bathroom);
	optional.Add(workRoom);
	return RoomBlueprint{ needed, optional, false };
}

RoomBlueprint LivingSpecification::getBlueprint(float areaScale) {
	TArray<RoomSpecification> needed;
	RoomSpecification kitchen{40*areaScale, 90*areaScale, SubRoomType::kitchen};
	RoomSpecification bathroom{30*areaScale, 60*areaScale, SubRoomType::bath };
	RoomSpecification bedroom{50*areaScale, 100*areaScale, SubRoomType::bed };
	RoomSpecification living{100 * areaScale, 150 * areaScale, SubRoomType::living };
	RoomSpecification closet{ 10 * areaScale, 40 * areaScale, SubRoomType::closet };

	needed.Add(bedroom);
	needed.Add(living);
	needed.Add(kitchen);
	needed.Add(bathroom);

	TArray<RoomSpecification> optional;
	optional.Add(bedroom);
	optional.Add(closet);
	optional.Add(bedroom);
	optional.Add(bathroom);
	optional.Add(living);

	return RoomBlueprint{ needed, optional, true };
}
void LivingSpecification::intermediateInteractWithRooms(TArray<FRoomPolygon*> &roomPols, FRoomInfo &r, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony) {
	if (potentialBalcony) {
		for (FRoomPolygon *p : roomPols) {
			if (splitableType(p->type)) {
				// these are the balcony candidate rooms
				if (p->windows.Num() > 0) {
					for (int place : p->windows) {
						if (FVector::DistSquared(p->points[place%p->points.Num()], p->points[place - 1]) > 10000) {
							p->entrances.Add(place);
							FVector mid = middle(p->points[place%p->points.Num()], p->points[place - 1]);
							p->specificEntrances.Add(place, mid);
							r.append(ARoomBuilder::placeBalcony(p, place, map));
							return;
						}
					}
				}
			}
		}
	}
}

RoomBlueprint RestaurantSpecification::getBlueprint(float areaScale) {
	TArray<RoomSpecification> needed;
	RoomSpecification restaurant{ 50 * areaScale, 1000 * areaScale, SubRoomType::restaurant};
	needed.Add(restaurant);

	TArray<RoomSpecification> optional;
	optional.Add(restaurant);
	return RoomBlueprint{ needed, optional };

}


void placeMoreEntrances(TArray<FRoomPolygon*> &roomPols) {
	for (FRoomPolygon* r2 : roomPols) {
		for (int i : r2->windows) {
			if (FVector::Dist(r2->points[i%r2->points.Num()], r2->points[i - 1]) > 150.0f && splitableType(r2->type))
				r2->entrances.Add(i);
		}
	}
}

void RestaurantSpecification::intermediateInteractWithRooms(TArray<FRoomPolygon*> &roomPols, FRoomInfo &r, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony) {
	placeMoreEntrances(roomPols);
}

RoomBlueprint StoreSpecification::getBlueprint(float areaScale) {
	TArray<RoomSpecification> needed;
	RoomSpecification storeBack{ 50 * areaScale, 200 * areaScale, SubRoomType::storeBack };
	RoomSpecification storeFront{ 200 * areaScale, 400 * areaScale, SubRoomType::storeFront };
	RoomSpecification bathroom{ 30 * areaScale, 60 * areaScale, SubRoomType::bath };
	needed.Add(storeFront);

	TArray<RoomSpecification> optional;
	optional.Add(bathroom);
	optional.Add(storeBack);

	return RoomBlueprint{ needed, optional };
}

void StoreSpecification::intermediateInteractWithRooms(TArray<FRoomPolygon*> &roomPols, FRoomInfo &r, TMap<FString, UHierarchicalInstancedStaticMeshComponent*> &map, bool potentialBalcony) {
	placeMoreEntrances(roomPols);
}