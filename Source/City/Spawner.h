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

	// the maximum length a road can be extended in order to attach to another road
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float maxAttachDistance;

	// chance of main road to branch, allowing a new main road at 90 or 270 degrees from current
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float mainRoadBranchChance;

	// scale if noise read either from heat map or perline noise, a lower value means more "zoomed in"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float noiseScale = 0.00003;
	// advantage in priority queue given to the main road in road generation, influences how prevalent main roads will be
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float mainRoadAdvantage = 0.1;

	// range of negative heatmap for main road
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float mainRoadDetrimentRange = 1000000;
	// impact of negative heatmap for main road
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float mainRoadDetrimentImpact = 0.01;

	// collision overlap to allow without calling it a collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float collisionLeniency;

	// longest length allowed for a main road
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float maxMainRoadLength;

	// longest length allowed for a secondary road
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = road, meta = (AllowPrivateAccess = "true"))
		float maxSecondaryRoadLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = mesh, meta = (AllowPrivateAccess = "true"))
		UStaticMesh* meshRoad;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = mesh, meta = (AllowPrivateAccess = "true"))
		UStaticMesh* meshPolygon;

	// maximum size of house before being split into new houses
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = house, meta = (AllowPrivateAccess = "true"))
	float maxHouseArea = 2000.0f;

	// minimum size of houses before they're turned into simple plots
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = house, meta = (AllowPrivateAccess = "true"))
	float minHouseArea = 200.0f;

	// minimum number of floors in a building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = house, meta = (AllowPrivateAccess = "true"))
		int32	minFloors = 3;
	// maximum number of floors in a building, note that buildings may be higher though, it's more of a guideline
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = house, meta = (AllowPrivateAccess = "true"))
		int32	maxFloors = 15;

	// the height of individual floors, should not really be changed too much since mesh placement can get a bit wonky looking is misaligned with floor height
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = house, meta = (AllowPrivateAccess = "true"))
		float floorHeight = 400.0f;

	// number of times a building is subjected to modification attempts of original polygon before starting to build upwards, lower values result in more blocky looking houses
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
		int makeInterestingAttempts = 4;

	// whether to generate roofs on top of buildings or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = meshes, meta = (AllowPrivateAccess = "true"))
		bool generateRoofs = true;

	UPROPERTY(EditAnywhere, Instanced, Category = "Path spline")
		USplineMeshComponent* PathSpline;

	TArray<USplineMeshComponent*> splineComponents;
	
	// whether to use provided texture as heat map or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = noise, meta = (AllowPrivateAccess = "true"))
		bool useTexture;
	// the texture to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = noise, meta = (AllowPrivateAccess = "true"))
		UTexture2D* noiseTexture;
	// the scale multipleid by noiseScale to give final scale for texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = noise, meta = (AllowPrivateAccess = "true"))
		float noiseTextureScale;

	// sidewalk offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = sidewalk, meta = (AllowPrivateAccess = "true"))
		float offsetSize = 500;

	// maximum size of apartment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = rooms, meta = (AllowPrivateAccess = "true"))
		float maxRoomSize = 500;
		

	// extra length of lines placed for plot generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = plotCreation, meta = (AllowPrivateAccess = "true"))
		float extraLen = 500;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = plotCreation, meta = (AllowPrivateAccess = "true"))
		float extraBlockingLen = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = performance, meta = (AllowPrivateAccess = "true"))
		GenerationMode generationMode;
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


	UFUNCTION(BlueprintCallable, Category = "Generation")
	TArray<FMetaPolygon> getSurroundingPolygons(TArray<FRoadSegment> segments);

	// shows the current noise function by returning transforms for visualization in engine
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
