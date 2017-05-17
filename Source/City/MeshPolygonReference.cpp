// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "MeshPolygonReference.h"

MeshPolygonReference::MeshPolygonReference()
{
}

FPolygon MeshPolygonReference::getBedPolygon(FVector origin, FRotator dir) {
	FPolygon bed;
	bed.points.Add(origin + dir.RotateVector(FVector(-135, -86, 0)));
	bed.points.Add(origin + dir.RotateVector(FVector(135, -86, 0)));
	bed.points.Add(origin + dir.RotateVector(FVector(135, 86, 0)));
	bed.points.Add(origin + dir.RotateVector(FVector(-135, 86, 0)));
	return bed;
}

FPolygon MeshPolygonReference::getShelfPolygon(FVector origin, FRotator dir) {
	// 180 60
	FPolygon shelf;
	shelf.points.Add(origin + dir.RotateVector(FVector(270, -30, 0)));
	shelf.points.Add(origin + dir.RotateVector(FVector(90, -30, 0)));
	shelf.points.Add(origin + dir.RotateVector(FVector(90, 30, 0)));
	shelf.points.Add(origin + dir.RotateVector(FVector(270, 30, 0)));
	return shelf;
}

FPolygon MeshPolygonReference::getSmallTablePolygon(FVector origin, FRotator dir) {
	// 102 94
	FPolygon shelf;
	shelf.points.Add(origin + dir.RotateVector(FVector(51, -47, 0)));
	shelf.points.Add(origin + dir.RotateVector(FVector(-51, -47, 0)));
	shelf.points.Add(origin + dir.RotateVector(FVector(-51, 47, 0)));
	shelf.points.Add(origin + dir.RotateVector(FVector(51, 47, 0)));
	return shelf;
}


MeshPolygonReference::~MeshPolygonReference()
{
}
