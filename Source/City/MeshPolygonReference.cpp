// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "MeshPolygonReference.h"

MeshPolygonReference::MeshPolygonReference()
{
}

FPolygon MeshPolygonReference::getBedPolygon(FVector origin, FRotator dir) {
	// 288 451
	FPolygon bed;
	bed.points.Add(origin + dir.RotateVector(FVector(-144, 0, 0)));
	bed.points.Add(origin + dir.RotateVector(FVector(144, 0, 0)));
	bed.points.Add(origin + dir.RotateVector(FVector(144, 225, 0)));
	bed.points.Add(origin + dir.RotateVector(FVector(-144, 225, 0)));
	return bed;
}
MeshPolygonReference::~MeshPolygonReference()
{
}
