// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseLibrary.h"
#pragma once

/**
 * 
 */

UENUM(BlueprintType)
enum class MeshType : uint8
{
	bed 	UMETA(DisplayName = "Bed"),
	small_table UMETA(DisplayName = "Small Table"),
	toilet UMETA(DisplayName = "Toilet"),
	shelf UMETA(DisplayName = "Shelf"),
	bazinga UMETA(DisplayName = "Bazinga"),
	fridge UMETA(DisplayName = "Fridge"),
	chair UMETA(DisplayName = "Chair"),
	sink UMETA(DisplayName = "Sink"),
	shelf_upper_large UMETA(DisplayName = "Large Upper Shelf")
	//shelf UMETA(DisplayName = "shelf"),
	//shelf UMETA(DisplayName = "shelf"),


};

class CITY_API MeshPolygonReference
{
public:
	MeshPolygonReference();
	static FPolygon getAppropriatePolygon(MeshType m, FVector origin, FRotator dir);
	static FPolygon getBedPolygon(FVector origin, FRotator dir);
	static FPolygon getShelfPolygon(FVector origin, FRotator dir);
	static FPolygon getSmallTablePolygon(FVector origin, FRotator dir);
	static FPolygon getToiletPolygon(FVector origin, FRotator dir);
	static FPolygon getSinkPolygon(FVector origin, FRotator dir);

	~MeshPolygonReference();
};
