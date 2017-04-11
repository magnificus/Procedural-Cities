#pragma once

#include "GameFramework/Actor.h"

#include "Components/SplineMeshComponent.h"
#include "BaseLibrary.h"
#include "Spawner.generated.h"






UCLASS()
class CITY_API ASpawner : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = General, meta = (AllowPrivateAccess = "true"))
		float standardWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		FVector stepLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		float changeIntensity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		float secondaryChangeIntensity;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = limits, meta = (AllowPrivateAccess = "true"))
		int32 length;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = limits, meta = (AllowPrivateAccess = "true"))
		float maxDist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		float maxAttachDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		float mainRoadBranchChance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		float secondaryRoadBranchChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		float collisionLeniency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = length, meta = (AllowPrivateAccess = "true"))
		float maxMainRoadLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = length, meta = (AllowPrivateAccess = "true"))
		float maxSecondaryRoadLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = length, meta = (AllowPrivateAccess = "true"))
		UStaticMesh* meshRoad;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = length, meta = (AllowPrivateAccess = "true"))
		UStaticMesh* meshPolygon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = length, meta = (AllowPrivateAccess = "true"))
		USplineMeshComponent* base;

	TArray<USplineMeshComponent*> splineComponents;
	
public:	
	// Sets default values for this actor's properties
	ASpawner();

	void addVertices(FRoadSegment* f);
	void addRoadForward(std::priority_queue<logicRoadSegment*, std::deque<logicRoadSegment*>, roadComparator> &queue, logicRoadSegment* previous, std::vector<logicRoadSegment*> &allsegments);
	void addRoadSide(std::priority_queue<logicRoadSegment*, std::deque<logicRoadSegment*>, roadComparator> &queue, logicRoadSegment* previous, bool left, float width, std::vector<logicRoadSegment*> &allsegments, RoadType newType);
	void addExtensions(std::priority_queue<logicRoadSegment*, std::deque<logicRoadSegment*>, roadComparator> &queue, logicRoadSegment* current, std::vector<logicRoadSegment*> &allsegments);

	bool placementCheck(TArray<FRoadSegment*> &segments, logicRoadSegment* current, TMap <int, TArray<FRoadSegment*>*> &map);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	TArray<FRoadSegment> determineRoadSegments();

	UFUNCTION(BlueprintCallable, Category = "Rendering")
	void buildRoads(TArray<FRoadSegment> segments);

	UFUNCTION(BlueprintCallable, Category = "Rendering")
	void buildPolygons(TArray<FPolygon> segments);


	UFUNCTION(BlueprintCallable, Category = "Generation")
	TArray<FPolygon> getBuildingPolygons(TArray<FRoadSegment> segments);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
};
