#pragma once

#include "GameFramework/Actor.h"
#include "stdlib.h"
#include <queue>
#include "Spawner.generated.h"


// would be much prettier with a 3-bit solution but that doesn't work with Blueprints... :(
UENUM(BlueprintType)
enum class Direction : uint8
{
	L 	UMETA(DisplayName = "Left"),
	F 	UMETA(DisplayName = "Forward"),
	R	UMETA(DisplayName = "Right"),
	LF  UMETA(DisplayName = "Left Front"),
	LR  UMETA(DisplayName = "Left Right"),
	FR  UMETA(DisplayName = "Forward Right"),
	LFR UMETA(DisplayName = "All Directions")
};

UENUM(BlueprintType)
enum class RoadType : uint8
{
	main 	UMETA(DisplayName = "Main Road"),
	secondary UMETA(DisplayName = "Secondary Road")
};


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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		Direction dir;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		Direction out;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		RoadType type;

};

struct logicRoadSegment {
	int time;
	logicRoadSegment* previous;
	FRoadSegment* segment;
	FRotator firstDegreeRot;
	FRotator secondDegreeRot;
	int roadLength;
	bool operator<(const logicRoadSegment* rhs) const {
		return this->time < rhs->time;
	}
};



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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = limits, meta = (AllowPrivateAccess = "true"))
		int32 length;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = limits, meta = (AllowPrivateAccess = "true"))
		float maxDist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		float minAttachDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = algorithm, meta = (AllowPrivateAccess = "true"))
		float mainRoadBranchChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = length, meta = (AllowPrivateAccess = "true"))
		float maxMainRoadLength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = length, meta = (AllowPrivateAccess = "true"))
		float maxSecondaryRoadLength;
	
public:	
	// Sets default values for this actor's properties
	ASpawner();


	void addRoadForward(std::priority_queue<logicRoadSegment*> &queue, logicRoadSegment* previous, std::vector<logicRoadSegment*> &allsegments);
	void addRoadSide(std::priority_queue<logicRoadSegment*> &queue, logicRoadSegment* previous, bool left, float width, std::vector<logicRoadSegment*> &allsegments, RoadType newType);
	void addExtensions(std::priority_queue<logicRoadSegment*> &queue, logicRoadSegment* current, std::vector<logicRoadSegment*> &allsegments);

	bool placementCheck(TArray<FRoadSegment*> &segments, logicRoadSegment* current);

	UFUNCTION(BlueprintCallable, Category = "Generation")
	TArray<FRoadSegment> determineRoadSegments();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
};
