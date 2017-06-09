// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "ProcMeshActor.h"
#include "polypartition.h"

// Sets default values
AProcMeshActor::AProcMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	exteriorMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("exteriorMesh"));
	sndExteriorMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("sndExteriorMesh"));
	interiorMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("interiorMesh"));
	windowMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("windowMesh"));
	windowFrameMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("windowFrameMesh"));
	occlusionWindowMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("occlusionWindowMesh"));
	floorMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("floorMesh"));
	roofMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("roofMesh"));
	greenMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("greenMesh"));
	concreteMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("concreteMesh"));
	roadMiddleMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("roadMiddleMesh"));


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



bool AProcMeshActor::buildPolygons(TArray<FPolygon> &pols, FVector offset, UProceduralMeshComponent* mesh, UMaterialInterface *mat) {
	if (pols.Num() == 0) {
		return true;
	}

	//if (mat == greenMat)
	//	UE_LOG(LogTemp, Warning, TEXT("BUILDING GREEN POLYGONS"));

	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector2D> UV;

	TArray<FColor> vertexColors;
	TArray<FProcMeshTangent> tangents;

	int current = 0;
	for (FPolygon &pol : pols) {

		//pol.reverse();

		// local coordinates are found by getting the coordinates of points on the plane which they span up
		FVector e1 = pol.points[1] - pol.points[0];
		e1.Normalize();
		//if (FVector::DotProduct(e1, FVector(1, 0, 0)) < 0) {
		//	e1 = -e1;
		//}
		FVector n = FVector::CrossProduct(e1, pol.points[2] - pol.points[0]);
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
			TPPLPoint newP{ x, y, current + i};
			poly[i] = newP;
			vertices.Add(point);

		}
		TPPLPartition part;
		poly.SetOrientation(TPPL_CCW);
		int res = part.Triangulate_EC(&poly, &inTriangles);

		if (res == 0) {
			UE_LOG(LogTemp, Warning, TEXT("Triangulation failed!"));
			//return false;
		}
		for (auto i : inTriangles) {
			triangles.Add(i[0].id);
			triangles.Add(i[1].id);
			triangles.Add(i[2].id);
		}
		current += pol.points.Num();
	}



	TArray<FVector> normals;

	mesh->SetMaterial(1, mat);
	mesh->SetCullDistance(100);

	mesh->CreateMeshSection(1, vertices, triangles, normals, UV, vertexColors, tangents, true);
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
	int a = buildPolygons(exterior, offset, exteriorMesh, exteriorMat);
	a += buildPolygons(exteriorSnd, offset, sndExteriorMesh, sndExteriorMat);
	a += buildPolygons(interior, offset, interiorMesh, interiorMat);
	a += buildPolygons(windows, offset, windowMesh, windowMat);
	a += buildPolygons(floors, offset, floorMesh, floorMat);
	a += buildPolygons(roofs, offset, roofMesh, roofMat);
	a += buildPolygons(occlusionWindows, offset, occlusionWindowMesh, occlusionWindowMat);
	a += buildPolygons(windowFrames, offset, windowFrameMesh, windowFrameMat);
	a += buildPolygons(concrete, offset, concreteMesh, concreteMat);
	a += buildPolygons(green, offset, greenMesh, greenMat);
	a += buildPolygons(roadMiddle, offset, roadMiddleMesh, roadMiddleMat);

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

}

