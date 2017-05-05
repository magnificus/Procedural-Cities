// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BaseLibrary.h"
#include "RoomBuilder.generated.h"

USTRUCT(BlueprintType)
struct FMeshInfo {

	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh *mesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector location;
};

struct RoomInfo {
	TArray<FRoomPolygon> rooms;
	TArray<FMeshInfo> meshes;

	float beginning;
	float height;
	//TArray<FPolygon> windows;

};
UCLASS()
class CITY_API ARoomBuilder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoomBuilder();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	static RoomInfo buildRoom(FRoomPolygon, RoomType type, int floor, float beginning, float height);
	
};
