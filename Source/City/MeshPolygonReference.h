// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseLibrary.h"
#pragma once

/**
 * 
 */



class CITY_API MeshPolygonReference
{
public:
	MeshPolygonReference();
	static FPolygon getBedPolygon(FVector origin, FRotator dir);
	static FPolygon getShelfPolygon(FVector origin, FRotator dir);

	~MeshPolygonReference();
};
