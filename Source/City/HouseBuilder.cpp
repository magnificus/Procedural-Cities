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
}


void AHouseBuilder::placeHouse(FHousePolygon f)
{
	//Empty the array and delete all it's components
	for (auto It = meshesArray.CreateIterator(); It; It++)
	{
		(*It)->DestroyComponent();
	}

	meshesArray.Empty();

	//Register all the components
	RegisterAllComponents();
	//The base name for all our components
	FName InitialName = FName("MyCompName");

		UStaticMeshComponent* NewComp = NewObject<UStaticMeshComponent>(this, InitialName);

		//Add a reference to our array
		meshesArray.Add(NewComp);

		FString Str = "House";

		//Convert the FString to FName
		InitialName = (*Str);

		//If the component is valid, set it's static mesh, relative location and attach it to our parent
		if (NewComp)
		{
			GLog->Log("Registering comp...");

			//Register the new component
			NewComp->RegisterComponent();

			//Set the static mesh of our component
			NewComp->SetStaticMesh(placeHolderHouseMesh->GetStaticMesh());


			FVector Location = f.polygon.center;

			NewComp->SetWorldLocation(Location);
			NewComp->SetWorldScale3D(FVector(1, 1, randFloat() * 10 + 1));
			//Attach the component to the root component
			NewComp->AttachTo(GetRootComponent(), NAME_None, EAttachLocation::KeepRelativeOffset);
		}
}

// Called when the game starts or when spawned
void AHouseBuilder::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHouseBuilder::BeginDestroy()
{
	for (auto It = meshesArray.CreateIterator(); It; It++)
	{
		(*It)->DestroyComponent();
	}
	Super::BeginDestroy();

}

// Called every frame
void AHouseBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

