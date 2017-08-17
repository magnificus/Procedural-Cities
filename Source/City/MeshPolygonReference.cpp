// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "MeshPolygonReference.h"

MeshPolygonReference::MeshPolygonReference()
{
}

FPolygon MeshPolygonReference::getStairPolygon(FVector origin, FRotator dir) {
	FPolygon pol;
	pol.points.Add(origin + dir.RotateVector(FVector(-190, -213, 0)));
	pol.points.Add(origin + dir.RotateVector(FVector(-190, 213, 0)));
	pol.points.Add(origin + dir.RotateVector(FVector(190, 213, 0)));
	pol.points.Add(origin + dir.RotateVector(FVector(190, -213, 0)));

	//pol.points.Add(origin + dir.RotateVector(FVector(190, -213, 0)));
	return pol;
}

MeshPolygonReference::~MeshPolygonReference()
{
}
