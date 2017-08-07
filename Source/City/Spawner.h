#pragma once

#include "GameFramework/Actor.h"

#include "Components/SplineMeshComponent.h"
//#include "BaseLibrary.h"
#include "PlotBuilder.h"
#include "Spawner.generated.h"



USTRUCT(BlueprintType)
struct FVisualizer{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float height;

};

UCLASS()
class CITY_API ASpawner : public AActor
{
	GENERATED_BODY()

	// width of road mesh, you don't want to mess around with this if you're not changing the road model
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float standardWidth; 

	// the length of a single road section for the main road
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		FVector primaryStepLength;
	// the length of a single road section for the secondary roads
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		FVector secondaryStepLength;

	// the maximum amount of change allowed for the main road
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float changeIntensity;
	// the maximum amount of change allowed for the secondary roads
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float secondaryChangeIntensity;

	// the total number of road segments placed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		int32 length;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float maxAttachDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float mainRoadBranchChance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float noiseScale = 0.00003;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float mainRoadAdvantage = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float mainRoadDetrimentRange = 1000000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float mainRoadDetrimentImpact = 0.01;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float collisionLeniency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float maxMainRoadLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float maxSecondaryRoadLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = mesh, meta = (AllowPrivateAccess = "true"))
		UStaticMesh* meshRoad;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = mesh, meta = (AllowPrivateAccess = "true"))
		UStaticMesh* meshPolygon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = house, meta = (AllowPrivateAccess = "true"))
	float maxHouseArea = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = house, meta = (AllowPrivateAccess = "true"))
	float minHouseArea = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = house, meta = (AllowPrivateAccess = "true"))
		int32	minFloors = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = house, meta = (AllowPrivateAccess = "true"))
		int32	maxFloors = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = house, meta = (AllowPrivateAccess = "true"))
		float floorHeight = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
		int makeInterestingAttempts = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
		bool generateRoofs = true;

	UPROPERTY(EditAnywhere, Instanced, Category = "Path spline")
		USplineMeshComponent* PathSpline;

	TArray<USplineMeshComponent*> splineComponents;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = noise, meta = (AllowPrivateAccess = "true"))
		bool useTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = noise, meta = (AllowPrivateAccess = "true"))
		UTexture2D* noiseTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = noise, meta = (AllowPrivateAccess = "true"))
		float noiseTextureScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = sidewalk, meta = (AllowPrivateAccess = "true"))
		float	offsetSize = 500;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = rooms, meta = (AllowPrivateAccess = "true"))
		float maxRoomSize = 200;
		

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = plotCreation, meta = (AllowPrivateAccess = "true"))
		float extraLen = 500;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = plotCreation, meta = (AllowPrivateAccess = "true"))
		float extraBlockingLen = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = performance, meta = (AllowPrivateAccess = "true"))
		bool loadInProcedurally = false;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = plotCreation, meta = (AllowPrivateAccess = "true"))
	//	float extraLen;
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

	UFUNCTION(BlueprintCallable, Category = "Generation")
	TArray<FMaterialPolygon> getRoadLines(TArray<FRoadSegment> segments);

	UFUNCTION(BlueprintCallable, Category = "Data")
		TArray<FPolygon> roadsToPolygons(TArray<FRoadSegment> segments);
	//UFUNCTION(BlueprintCallable, Category = "Generation")
	//void buildPlots(TArray<FPolygon> polygons);


	UFUNCTION(BlueprintCallable, Category = "Generation")
	TArray<FMetaPolygon> getSurroundingPolygons(TArray<FRoadSegment> segments);

	UFUNCTION(BlueprintCallable, Category = "Test")
	TArray<FTransform> visualizeNoise(int numSide, float noiseMultiplier, float posMultiplier);

	//UFUNCTION(BlueprintCallable, Category = "Generation")
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//virtual void OnConstruction(const FTransform& Transform) override;

	//virtual void BeginDestroy() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
};
