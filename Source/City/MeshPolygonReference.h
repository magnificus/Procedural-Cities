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
	shelf_upper_large UMETA(DisplayName = "Large Upper Shelf"),
	hanger UMETA(DisplayName = "Hanger"),
	oven UMETA(DisplayName = "Oven"),
	wardrobe UMETA(DisplayName = "Wardrobe")
	//shelf UMETA(DisplayName = "shelf"),
	//shelf UMETA(DisplayName = "shelf"),


};

class CITY_API MeshPolygonReference
{
public:
	MeshPolygonReference();

	static FPolygon getStairPolygon(FVector origin, FRotator dir);


	~MeshPolygonReference();
};
