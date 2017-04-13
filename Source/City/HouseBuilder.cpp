// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "HouseBuilder.h"

struct FPolygon;


// Sets default values
AHouseBuilder::AHouseBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	placeHolderHouseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(placeHolderHouseMesh);



	FString pathName = "StaticMesh'/Game/Geometry/Meshes/1M_Cube.1M_Cube'";
	ConstructorHelpers::FObjectFinder<UStaticMesh> mySSMesh12412(*pathName);
	placeHolderHouseMesh->SetStaticMesh(mySSMesh12412.Object);
	placeHolderHouseMesh->SetMobility(EComponentMobility::Static);


	pathName = "StaticMesh'/Game/StarterContent/Props/SM_PillarFrame.SM_PillarFrame'";
	ConstructorHelpers::FObjectFinder<UStaticMesh> buildingMesh(*pathName);
	meshPolygon = buildingMesh.Object;

}


void AHouseBuilder::placeHouse(FHousePolygon f)
{
	//Empty the array and delete all it's components
	//for (auto It = meshesArray.CreateIterator(); It; It++)
	//{
	//	(*It)->DestroyComponent();
	//}

	//meshesArray.Empty();

	////Register all the components
	//RegisterAllComponents();
	////The base name for all our components
	//FName InitialName = FName("MyCompName");

	//	UStaticMeshComponent* NewComp = NewObject<UStaticMeshComponent>(this, InitialName);

	//	//Add a reference to our array
	//	meshesArray.Add(NewComp);

	//	FString Str = "House";

	//	//Convert the FString to FName
	//	InitialName = (*Str);

	//	//If the component is valid, set it's static mesh, relative location and attach it to our parent
	//	if (NewComp)
	//	{
	//		GLog->Log("Registering comp...");

	//		//Register the new component
	//		NewComp->RegisterComponent();

	//		//Set the static mesh of our component
	//		NewComp->SetStaticMesh(placeHolderHouseMesh->GetStaticMesh());


	//		FVector Location = f.polygon.center;

	//		NewComp->SetWorldLocation(Location);
	//		//NewComp->SetWorldScale3D(FVector(30, 30, randFloat() * 150 + 30));
	//		//Attach the component to the root component
	//		NewComp->AttachTo(GetRootComponent(), NAME_None, EAttachLocation::KeepRelativeOffset);
	//	}


	for (int i = 1; i < f.polygon.points.Num(); i++) {
		USplineMeshComponent *s = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		s->SetStaticMesh(meshPolygon);
		s->SetCastShadow(false);
		s->SetStartAndEnd(f.polygon.points[i - 1], f.polygon.points[i] - f.polygon.points[i - 1], f.polygon.points[i], f.polygon.points[i] - f.polygon.points[i - 1], true);
		splineComponents.Add(s);
		s->SetHiddenInGame(false);
	}
	USplineMeshComponent *s = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
	s->SetStaticMesh(meshPolygon);
	s->SetCastShadow(false);
	s->SetStartAndEnd(f.polygon.points[f.polygon.points.Num()-1], f.polygon.points[0] - f.polygon.points[f.polygon.points.Num() - 1], f.polygon.points[0], f.polygon.points[0] - f.polygon.points[f.polygon.points.Num() - 1], true);
	splineComponents.Add(s);
	s->SetHiddenInGame(false);

}

// Called when the game starts or when spawned
void AHouseBuilder::BeginPlay()
{
	Super::BeginPlay();
	
}

//void AHouseBuilder::BeginDestroy()
//{
//	for (auto It = meshesArray.CreateIterator(); It; It++)
//	{
//		if (*It)
//			(*It)->DestroyComponent();
//	}
//	Super::BeginDestroy();
//
//}

// Called every frame
void AHouseBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

