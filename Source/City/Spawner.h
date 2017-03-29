// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Spawner.generated.h"

USTRUCT(BlueprintType)
struct FRoadSegment
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector start;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector end;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float width;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector beginTangent;

};

UCLASS()
class CITY_API ASpawner : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = data, meta = (AllowPrivateAccess = "true"))
	FString current;
	
public:	
	// Sets default values for this actor's properties
	ASpawner();

	UFUNCTION(BlueprintCallable, Category = "LSystem")
	TArray<FRoadSegment> executeLSystem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
