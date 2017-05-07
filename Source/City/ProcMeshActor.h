// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "BaseLibrary.h"
#include "ProcMeshActor.generated.h"

UCLASS()
class CITY_API AProcMeshActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProcMeshActor();

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void buildTriangle(FVector p1, FVector p2, FVector p3);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void buildWall(FVector p1, FVector p2, FVector p3, FVector p4);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void buildPolygons(TArray<FPolygon> pols, FVector offset);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		UMaterial* mat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		float texScaleMultiplier = 1.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, Category = Materials)
	UProceduralMeshComponent * mesh;
	int currIndex = 1;
	
	
};
