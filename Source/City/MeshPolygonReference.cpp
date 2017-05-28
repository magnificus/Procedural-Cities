// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "MeshPolygonReference.h"

MeshPolygonReference::MeshPolygonReference()
{
}

//
//FPolygon MeshPolygonReference::getAppropriatePolygon(MeshType m, FVector origin, FRotator dir) {
//	switch (m) {
//	case MeshType::bed: return getBedPolygon(origin, dir);
//	case MeshType::small_table: return getSmallTablePolygon(origin, dir);
//	case MeshType::shelf: return getShelfPolygon(origin, dir);
//	case MeshType::toilet: return getToiletPolygon(origin, dir);
//	case MeshType::sink: return getSinkPolygon(origin, dir);
//	case MeshType::shelf_upper_large: return getShelfUpperLarge(origin, dir);
//	case MeshType::fridge: return getFridge(origin, dir);
//	case MeshType::hanger: return getHanger(origin, dir);
//	}
//	return FPolygon();
//}
//
//
//FPolygon MeshPolygonReference::getBedPolygon(FVector origin, FRotator dir) {
//	FPolygon bed;
//	bed.points.Add(origin + dir.RotateVector(FVector(-135, -86, 0)));
//	bed.points.Add(origin + dir.RotateVector(FVector(135, -86, 0)));
//	bed.points.Add(origin + dir.RotateVector(FVector(135, 86, 0)));
//	bed.points.Add(origin + dir.RotateVector(FVector(-135, 86, 0)));
//	bed.points.Add(origin + dir.RotateVector(FVector(-135, -86, 0)));
//	return bed;
//}
//
//FPolygon MeshPolygonReference::getShelfPolygon(FVector origin, FRotator dir) {
//	// 180 60
//	FPolygon shelf;
//	//dir += FRotator(0, 180, 0);
//	shelf.points.Add(origin + dir.RotateVector(FVector(270, -30, 0)));
//	shelf.points.Add(origin + dir.RotateVector(FVector(90, -30, 0)));
//	shelf.points.Add(origin + dir.RotateVector(FVector(90, 30, 0)));
//	shelf.points.Add(origin + dir.RotateVector(FVector(270, 30, 0)));
//	shelf.points.Add(origin + dir.RotateVector(FVector(270, -30, 0)));
//	return shelf;
//}
//
//FPolygon MeshPolygonReference::getSmallTablePolygon(FVector origin, FRotator dir) {
//	// 102 94
//	FPolygon table;
//	table.points.Add(origin + dir.RotateVector(FVector(51, -47, 0)));
//	table.points.Add(origin + dir.RotateVector(FVector(-51, -47, 0)));
//	table.points.Add(origin + dir.RotateVector(FVector(-51, 47, 0)));
//	table.points.Add(origin + dir.RotateVector(FVector(51, 47, 0)));
//	table.points.Add(origin + dir.RotateVector(FVector(51, -47, 0)));
//	return table;
//}
//
//FPolygon MeshPolygonReference::getToiletPolygon(FVector origin, FRotator dir) {
//	FPolygon pol;
//	//dir += FRotator(0, 90, 0);
//	pol.points.Add(origin + dir.RotateVector(FVector(36, -64, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(-36, -64, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(-36, 64, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(36, 64, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(36, -64, 0)));
//	return pol;
//}
//
//FPolygon MeshPolygonReference::getSinkPolygon(FVector origin, FRotator dir) {
//	FPolygon pol;
//	//dir += FRotator(0, 90, 0);
//	pol.points.Add(origin + dir.RotateVector(FVector(60, -90, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(-60, -90, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(-60, 90, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(60, 90, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(60, -90, 0)));
//	return pol;
//}
//
//FPolygon MeshPolygonReference::getShelfUpperLarge(FVector origin, FRotator dir) {
//	FPolygon shelfUpper;
//	return shelfUpper;
//}
//
FPolygon MeshPolygonReference::getStairPolygon(FVector origin, FRotator dir) {
	FPolygon pol;
	pol.points.Add(origin + dir.RotateVector(FVector(190, -213, 0)));
	pol.points.Add(origin + dir.RotateVector(FVector(-190, -213, 0)));
	pol.points.Add(origin + dir.RotateVector(FVector(-190, 213, 0)));
	pol.points.Add(origin + dir.RotateVector(FVector(190, 213, 0)));
	pol.points.Add(origin + dir.RotateVector(FVector(190, -213, 0)));
	return pol;
}
//
//FPolygon MeshPolygonReference::getHanger(FVector origin, FRotator dir) {
//	FPolygon pol;
//	pol.points.Add(origin + dir.RotateVector(FVector(33, -33, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(-33, -33, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(-33, 33, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(33, 33, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(33, -33, 0)));
//	return pol;
//}
//
//
//FPolygon MeshPolygonReference::getFridge(FVector origin, FRotator dir) {
//	FPolygon pol;
//	pol.points.Add(origin + dir.RotateVector(FVector(35, -40, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(-35, -40, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(-35, 40, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(35, 40, 0)));
//	pol.points.Add(origin + dir.RotateVector(FVector(35, -40, 0)));
//	return pol;
//}


MeshPolygonReference::~MeshPolygonReference()
{
}
