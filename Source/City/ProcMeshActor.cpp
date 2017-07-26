// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "City.h"
#include "gpc.h"
//#include "gpc.c"
#include "ProcMeshActor.h"
#include "polypartition.h"
// Sets default values
AProcMeshActor::AProcMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	exteriorMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("exteriorMesh"));
	sndExteriorMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("sndExteriorMesh"));
	interiorMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("interiorMesh"));
	windowMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("windowMesh"));
	windowFrameMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("windowFrameMesh"));
	occlusionWindowMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("occlusionWindowMesh"));
	floorMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("floorMesh"));
	roofMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("roofMesh"));
	greenMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("greenMesh"));
	concreteMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("concreteMesh"));
	roadMiddleMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("roadMiddleMesh"));

	//components.Add(exteriorMesh);
	//components.Add(sndExteriorMesh);
	//components.Add(interiorMesh);
	//components.Add(windowMesh);
	//components.Add(windowFrameMesh);
	//components.Add(occlusionWindowMesh);
	//components.Add(floorMesh);
	//components.Add(roofMesh);
	//components.Add(greenMesh);
	//components.Add(concreteMesh);
	//components.Add(roadMiddleMesh);

	//materials.Add(exteriorMat);
	//materials.Add(sndExteriorMat);
	//materials.Add(interiorMat);
	//materials.Add(windowMat);
	//materials.Add(windowFrameMat);
	//materials.Add(occlusionWindowMat);
	//materials.Add(floorMat);
	//materials.Add(roofMat);
	//materials.Add(greenMat);
	//materials.Add(concreteMat);
	//materials.Add(roadMiddleMat);
	/**
	*	Create/replace a section for this procedural mesh component.
	*	@param	SectionIndex		Index of the section to create or replace.
	*	@param	Vertices			Vertex buffer of all vertex positions to use for this mesh section.
	*	@param	Triangles			Index buffer indicating which vertices make up each triangle. Length must be a multiple of 3.
	*	@param	Normals				Optional array of normal vectors for each vertex. If supplied, must be same length as Vertices array.
	*	@param	UV0					Optional array of texture co-ordinates for each vertex. If supplied, must be same length as Vertices array.
	*	@param	VertexColors		Optional array of colors for each vertex. If supplied, must be same length as Vertices array.
	*	@param	Tangents			Optional array of tangent vector for each vertex. If supplied, must be same length as Vertices array.
	*	@param	bCreateCollision	Indicates whether collision should be created for this section. This adds significant cost.
	*/
	//UFUNCTION(BlueprintCallable, Category = "Components|ProceduralMesh", meta = (AutoCreateRefTerm = "Normals,UV0,VertexColors,Tangents"))
	//	void CreateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector>& Normals,
	// const TArray<FVector2D>& UV0, const TArray<FColor>& VertexColors, const TArray<FProcMeshTangent>& Tangents, bool bCreateCollision);

}



bool AProcMeshActor::buildPolygons(TArray<FPolygon> &pols, FVector offset, URuntimeMeshComponent* mesh, UMaterialInterface *mat) {
	if (mesh->GetNumSections() > 0 || pols.Num() == 0) {
		return false;
	}

	//TArray<FPolygon> cp;
	//for (FPolygon t : pols) {
	//	t.reverse();
	//	cp.Add(t);
	//}
	//pols.Append(cp);

	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector2D> UV;
	TArray<FVector> normals;

	TArray<FColor> vertexColors;
	TArray<FRuntimeMeshTangent> tangents;

	int current = 0;
	for (FPolygon &pol : pols) {

		//pol.reverse();
		if (pol.points.Num() < 3)
			continue;
		// local coordinates are found by getting the coordinates of points on the plane which they span up
		FVector e1 = pol.points[1] - pol.points[0];
		e1.Normalize();
		//if (FVector::DotProduct(e1, FVector(1, 0, 0)) < 0) {
		//	e1 = -e1;
		//}
		FVector n = pol.normal.Size() != 0.0 ? pol.normal : FVector::CrossProduct(e1, pol.points[pol.points.Num() - 1] - pol.points[0]);
		//n = FVector(0, 0, -1);
		FVector e2 = FVector::CrossProduct(e1, n);
		e2.Normalize();


		FVector origin = pol.points[0]; //FVector(0, 0, 0);

		std::list<TPPLPoly> inTriangles;

		TPPLPoly poly;
		poly.Init(pol.points.Num());
		for (int i = 0; i < pol.points.Num(); i++) {
			FVector point = pol.points[i];
			float y = FVector::DotProduct(e1, point - origin);
			float x = FVector::DotProduct(e2, point - origin);
			UV.Add(FVector2D(x*texScaleMultiplier, y*texScaleMultiplier));
			TPPLPoint newP{ x, y, current + i };
			poly[i] = newP;
			vertices.Add(point);
			normals.Add(-n);

		}
		//exteriorMesh->clear
		TPPLPartition part;
		//poly.SetOrientation(TPPL_CCW);
		int res = part.Triangulate_EC(&poly, &inTriangles);

		if (res == 0) {
			//UE_LOG(LogTemp, Warning, TEXT("Triangulation failed!"));
			//return false;
		}
		for (auto i : inTriangles) {
			triangles.Add(i[0].id);
			triangles.Add(i[1].id);
			triangles.Add(i[2].id);
		}
		current += pol.points.Num();
	}




	mesh->SetMaterial(1, mat);
	mesh->CreateMeshSection(1, vertices, triangles, normals, UV, vertexColors, tangents, false);
	return true;
}



bool AProcMeshActor::clearMeshes(bool fullReplacement) {
	if (fullReplacement) {
		exteriorMesh->ClearAllMeshSections();
		sndExteriorMesh->ClearAllMeshSections();
		greenMesh->ClearAllMeshSections();
		concreteMesh->ClearAllMeshSections();
		roofMesh->ClearAllMeshSections();
	}
	interiorMesh->ClearAllMeshSections();
	windowMesh->ClearAllMeshSections();
	windowFrameMesh->ClearAllMeshSections();
	occlusionWindowMesh->ClearAllMeshSections();
	floorMesh->ClearAllMeshSections();
	roadMiddleMesh->ClearAllMeshSections();
	return true;
}


// divides the polygon into the different materials used by the house
bool AProcMeshActor::buildPolygons(TArray<FMaterialPolygon> pols, FVector offset) {

	TArray<FPolygon> exterior;
	TArray<FPolygon> exteriorSnd;

	TArray<FPolygon> interior;
	TArray<FPolygon> windows;
	TArray<FPolygon> windowFrames;

	TArray<FPolygon> occlusionWindows;
	TArray<FPolygon> floors;
	TArray<FPolygon> roofs;

	TArray<FPolygon> concrete;
	TArray<FPolygon> green;

	TArray<FPolygon> roadMiddle;
	for (FMaterialPolygon &p : pols) {
		switch (p.type) {
		case PolygonType::exterior:
			exterior.Add(p);
			break;
		case PolygonType::exteriorSnd:
			exteriorSnd.Add(p);
			break;
		case PolygonType::interior:
			interior.Add(p);
			break;
		case PolygonType::window:
			windows.Add(p);
			break;
		case PolygonType::floor:
			floors.Add(p);
			break;
		case PolygonType::roof:
			roofs.Add(p);
			break;
		case PolygonType::occlusionWindow:
			occlusionWindows.Add(p);
			break;
		case PolygonType::windowFrame:
			windowFrames.Add(p);
			break;
		case PolygonType::concrete:
			concrete.Add(p);
			break;
		case PolygonType::green:
			green.Add(p);
			break;
		case PolygonType::roadMiddle:
			roadMiddle.Add(p);
			break;
		}
	}
	polygons.Empty();
	polygons.Add(exterior);
	polygons.Add(exteriorSnd);
	polygons.Add(interior);
	polygons.Add(windows);
	polygons.Add(windowFrames);
	polygons.Add(occlusionWindows);
	polygons.Add(floors);
	polygons.Add(roofs);
	polygons.Add(green);
	polygons.Add(concrete);
	polygons.Add(roadMiddle);

	components.Empty();
	components.Add(exteriorMesh);
	components.Add(sndExteriorMesh);
	components.Add(interiorMesh);
	components.Add(windowMesh);
	components.Add(windowFrameMesh);
	components.Add(occlusionWindowMesh);
	components.Add(floorMesh);
	components.Add(roofMesh);
	components.Add(greenMesh);
	components.Add(concreteMesh);
	components.Add(roadMiddleMesh);

	materials.Empty();
	materials.Add(exteriorMat);
	materials.Add(sndExteriorMat);
	materials.Add(interiorMat);
	materials.Add(windowMat);
	materials.Add(windowFrameMat);
	materials.Add(occlusionWindowMat);
	materials.Add(floorMat);
	materials.Add(roofMat);
	materials.Add(greenMat);
	materials.Add(concreteMat);
	materials.Add(roadMiddleMat);



	currentlyWorkingArray = 0;
	isWorking = true;
	//int a = buildPolygons(exterior, offset, exteriorMesh, exteriorMat);
	//a += buildPolygons(exteriorSnd, offset, sndExteriorMesh, sndExteriorMat);
	//a += buildPolygons(interior, offset, interiorMesh, interiorMat);
	//a += buildPolygons(windows, offset, windowMesh, windowMat);
	//a += buildPolygons(floors, offset, floorMesh, floorMat);
	//a += buildPolygons(roofs, offset, roofMesh, roofMat);
	//a += buildPolygons(occlusionWindows, offset, occlusionWindowMesh, occlusionWindowMat);
	//a += buildPolygons(windowFrames, offset, windowFrameMesh, windowFrameMat);
	//a += buildPolygons(concrete, offset, concreteMesh, concreteMat);
	//a += buildPolygons(green, offset, greenMesh, greenMat);
	//a += buildPolygons(roadMiddle, offset, roadMiddleMesh, roadMiddleMat);

	//if (a < 11) {
	//	//UE_LOG(LogTemp, Warning, TEXT("a: %i"), a);
	//	//Destroy();
	//	return false;
	//} 
	return true;


}


// Called when the game starts or when spawned
void AProcMeshActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProcMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (isWorking) {
		TArray<FPolygon> &current = polygons[currentlyWorkingArray];
		buildPolygons(current, FVector(0, 0, 0), components[currentlyWorkingArray], materials[currentlyWorkingArray]);
		currentlyWorkingArray++;
		if (currentlyWorkingArray >= polygons.Num())
			isWorking = false;
		//buildPolygons(current, FVector(0,0,0), components[currentlyWorkingArray], materials[currentlyWorkingArray], currentlyWorkingIndex, currentlyWorkingIndex + buildPerTick);
	}

}

