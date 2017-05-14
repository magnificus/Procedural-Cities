// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "ProcMeshActor.h"


// Sets default values
AProcMeshActor::AProcMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	exteriorMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("exteriorMesh"));
	interiorMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("interiorMesh"));
	windowMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("windowMesh"));
	floorMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("floorMesh"));
	roofMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("roofMesh"));

	RootComponent = exteriorMesh;
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

	//TArray<FVector> vertices;

	//vertices.Add(FVector(0, 0, 0));
	//vertices.Add(FVector(0, 1000, 0));
	//vertices.Add(FVector(0, 0, 1000));

	//TArray<int32> Triangles;
	//Triangles.Add(0);
	//Triangles.Add(1);
	//Triangles.Add(2);

	//TArray<FVector> normals;
	//normals.Add(FVector(1, 0, 0));
	//normals.Add(FVector(1, 0, 0));
	//normals.Add(FVector(1, 0, 0));

	//TArray<FVector2D> UV0;
	//UV0.Add(FVector2D(0, 0));
	//UV0.Add(FVector2D(10, 0));
	//UV0.Add(FVector2D(0, 10));

	//TArray<FLinearColor> vertexColors;
	//vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	//vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	//vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	//TArray<FProcMeshTangent> tangents;
	//tangents.Add(FProcMeshTangent(0, 1, 0));
	//tangents.Add(FProcMeshTangent(0, 1, 0));
	//tangents.Add(FProcMeshTangent(0, 1, 0));


	//mesh->SetMaterial(1, mat);
	//exteriorMesh->SetMaterial(1, exteriorMat);
	//interiorMesh->SetMaterial(1, interiorMat);
	//windowMesh->SetMaterial(1, windowMat);
	//floorMesh->SetMaterial(1, floorMat);
	//roofMesh->SetMaterial(1, roofMat);


	//mesh->CreateMeshSection_LinearColor(1, vertices, Triangles, normals, UV0, vertexColors, tangents, false);
}



void AProcMeshActor::buildPolygons(TArray<FPolygon> &pols, FVector offset, UProceduralMeshComponent* mesh, UMaterialInterface *mat) {
	if (pols.Num() == 0) {
		return;
	}
	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector2D> UV;

	TArray<FColor> vertexColors;
	TArray<FProcMeshTangent> tangents;

	int current = 0;
	for (FPolygon pol : pols) {

		// UVS are found by getting the coordinates of points on the plane which they span up
		FVector e1 = pol.points[1] - pol.points[0];
		FVector n = FVector::CrossProduct(e1, pol.points[2] - pol.points[0]);
		FVector e2 = FVector::CrossProduct(e1, n);



		FVector origin = pol.points[0];

		for (FVector f : pol.points) {
			vertices.Add(f + offset);
			float x = FVector::DotProduct(e1, f - origin);
			float y = FVector::DotProduct(e2, f - origin);
			UV.Add(FVector2D(x*texScaleMultiplier, y*texScaleMultiplier));
		}

		//FVector middle = pol.getCenter();

		for (int i = 2; i < pol.points.Num(); i++) {
			triangles.Add(current);
			triangles.Add(i - 1 + current);
			triangles.Add(i + current);

			triangles.Add(i + current);
			triangles.Add(i - 1 + current);
			triangles.Add(0 + current);

		}

		current += pol.points.Num();
	}



	TArray<FVector> normals;

	//mesh->MarkRenderStateDirty();
	mesh->SetMaterial(1, mat);
	mesh->CreateMeshSection(1, vertices, triangles, normals, UV, vertexColors, tangents, true);
}

// uses fan triangulation, doesn't work with convex shapes, builds faces in both directions
void AProcMeshActor::buildPolygons(TArray<FMaterialPolygon> pols, FVector offset) {

	TArray<FPolygon> exterior;
	TArray<FPolygon> interior;
	TArray<FPolygon> windows;
	TArray<FPolygon> floors;
	TArray<FPolygon> roofs;
	for (FMaterialPolygon &p : pols) {
		switch (p.type) {
		case PolygonType::exterior:
			exterior.Add(p);
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
		}
	}
	buildPolygons(exterior, offset, exteriorMesh, exteriorMat);
	buildPolygons(interior, offset, interiorMesh, interiorMat);
	buildPolygons(windows, offset, windowMesh, windowMat);
	buildPolygons(floors, offset, floorMesh, floorMat);
	buildPolygons(roofs, offset, roofMesh, roofMat);
	
}


void AProcMeshActor::buildTriangle(FVector p1, FVector p2, FVector p3) {
	// 4 faces for a wall, two triangles in each direction

	TArray<FVector> vertices;
	vertices.Add(p1);
	vertices.Add(p2);
	vertices.Add(p3);

	TArray<int32> Triangles;
	Triangles.Add(0);
	Triangles.Add(1);
	Triangles.Add(2);

	Triangles.Add(2);
	Triangles.Add(1);
	Triangles.Add(0);

	TArray<FVector> normals;
	TArray<FVector2D> UV0;
	TArray<FLinearColor> vertexColors;
	TArray<FProcMeshTangent> tangents;

	exteriorMesh->CreateMeshSection_LinearColor(currIndex++, vertices, Triangles, normals, UV0, vertexColors, tangents, false);

}

void AProcMeshActor::buildWall(FVector p1, FVector p2, FVector p3, FVector p4) {
	// 4 faces for a wall, two triangles in each direction

	TArray<FVector> vertices;
	vertices.Add(p1);
	vertices.Add(p2);
	vertices.Add(p3);
	vertices.Add(p4);

	TArray<int32> Triangles;
	Triangles.Add(0);
	Triangles.Add(1);
	Triangles.Add(2);

	Triangles.Add(2);
	Triangles.Add(1);
	Triangles.Add(3);


	Triangles.Add(2);
	Triangles.Add(1);
	Triangles.Add(0);

	Triangles.Add(3);
	Triangles.Add(1);
	Triangles.Add(2);

	TArray<FVector> normals;
	TArray<FVector2D> UV0;
	TArray<FLinearColor> vertexColors;
	TArray<FProcMeshTangent> tangents;

	exteriorMesh->CreateMeshSection_LinearColor(currIndex++, vertices, Triangles, normals, UV0, vertexColors, tangents, true);

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

}

