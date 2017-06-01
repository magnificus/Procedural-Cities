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
	void buildPolygons(TArray<FMaterialPolygon> pols, FVector offset);


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
		float texScaleMultiplier = 1.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void AProcMeshActor::buildPolygons(TArray<FPolygon> &pols, FVector offset, UProceduralMeshComponent* mesh, UMaterialInterface *mat);

	UPROPERTY(VisibleAnywhere, Category = Meshes)
	UProceduralMeshComponent * exteriorMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	UProceduralMeshComponent * interiorMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	UProceduralMeshComponent * windowMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	UProceduralMeshComponent * occlusionWindowMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	UProceduralMeshComponent * windowFrameMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	UProceduralMeshComponent* sndExteriorMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	UProceduralMeshComponent * floorMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
	UProceduralMeshComponent * roofMesh;

	UPROPERTY(VisibleAnywhere, Category = Meshes)
		UProceduralMeshComponent * greenMesh;
	UPROPERTY(VisibleAnywhere, Category = Meshes)
		UProceduralMeshComponent * concreteMesh;


	int currIndex = 1;
	
	
};
