// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "BaseLibrary.h"
#include "RuntimeMeshComponent.h"

#include "ProcMeshActor.generated.h"

UCLASS()
class CITY_API AProcMeshActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProcMeshActor();

	UFUNCTION(BlueprintCallable, Category = "Generation")
	bool buildPolygons(TArray<FMaterialPolygon> pols, FVector offset);

	bool clearMeshes(bool fullReplacement);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		UMaterial* exteriorMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		UMaterial* sndExteriorMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		UMaterial* windowMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		UMaterial* windowFrameMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		UMaterial* occlusionWindowMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		UMaterial* interiorMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		UMaterial* floorMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		UMaterial* roofMat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		UMaterial* greenMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		UMaterial* concreteMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		UMaterial* roadMiddleMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		UMaterial* asphaltMat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = appearance, meta = (AllowPrivateAccess = "true"))
		float texScaleMultiplier = 1.0f;

	//TArray<
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	bool AProcMeshActor::buildPolygons(TArray<FPolygon> &pols, FVector offset, URuntimeMeshComponent* mesh, UMaterialInterface *mat);

	bool isWorking = false;
	int currentlyWorkingArray = 0;


	UPROPERTY(VisibleAnywhere, Category = Meshes)
	URuntimeMeshComponent* exteriorMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	URuntimeMeshComponent* interiorMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	URuntimeMeshComponent * windowMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	URuntimeMeshComponent * occlusionWindowMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	URuntimeMeshComponent * windowFrameMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	URuntimeMeshComponent* sndExteriorMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	URuntimeMeshComponent * floorMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	URuntimeMeshComponent * roofMesh;

	UPROPERTY(VisibleAnywhere, Category = Meshes)
		URuntimeMeshComponent * greenMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
		URuntimeMeshComponent * concreteMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
		URuntimeMeshComponent * roadMiddleMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
		URuntimeMeshComponent * asphaltMesh;

	TArray<URuntimeMeshComponent*> components;
	TArray<UMaterialInterface*> materials;
	TArray<TArray<FPolygon>> polygons;

	int currIndex = 1;
	
	
};
