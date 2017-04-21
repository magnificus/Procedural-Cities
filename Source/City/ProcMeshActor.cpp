// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "ProcMeshActor.h"


// Sets default values
AProcMeshActor::AProcMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = mesh;
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

	TArray<FVector> vertices;

	vertices.Add(FVector(0, 0, 0));
	vertices.Add(FVector(0, 1000, 0));
	vertices.Add(FVector(0, 0, 1000));

	TArray<int32> Triangles;
	Triangles.Add(0);
	Triangles.Add(1);
	Triangles.Add(2);

	TArray<FVector> normals;
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));

	TArray<FVector2D> UV0;
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(10, 0));
	UV0.Add(FVector2D(0, 10));

	TArray<FLinearColor> vertexColors;
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	TArray<FProcMeshTangent> tangents;
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	//mesh->CreateMeshSection_LinearColor(1, vertices, Triangles, normals, UV0, vertexColors, tangents, false);
}

void AProcMeshActor::buildPolygons(TArray<FPolygon> polygons) {
	for (FPolygon p : polygons) {
		buildPolygon(p, FVector(0, 0, 0));
	}
}

// uses fan triangulation, doesn't work with convex shapes, builds faces in both directions
void AProcMeshActor::buildPolygon(FPolygon pol, FVector offset) {
	TArray<FVector> vertices;
	TArray<int32> triangles;

	FVector origin = pol.points[0];
	for (FVector f : pol.points)
		vertices.Add(f  + offset);

	for (int i = 2; i < pol.points.Num(); i++) {
		triangles.Add(0);
		triangles.Add(i - 1);
		triangles.Add(i);

		triangles.Add(i);
		triangles.Add(i - 1);
		triangles.Add(0);
	}


	TArray<FVector> normals;
	TArray<FVector2D> UV0;
	TArray<FLinearColor> vertexColors;
	TArray<FProcMeshTangent> tangents;
	mesh->CreateMeshSection_LinearColor(currIndex++, vertices, triangles, normals, UV0, vertexColors, tangents, false);

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

	mesh->CreateMeshSection_LinearColor(currIndex++, vertices, Triangles, normals, UV0, vertexColors, tangents, false);

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

	mesh->CreateMeshSection_LinearColor(currIndex++, vertices, Triangles, normals, UV0, vertexColors, tangents, false);

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

