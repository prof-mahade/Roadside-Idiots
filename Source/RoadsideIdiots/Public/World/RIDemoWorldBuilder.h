#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RIDemoWorldBuilder.generated.h"

class APlayerController;
class ARIRaceManager;
class AStaticMeshActor;
class UStaticMesh;

UCLASS()
class ROADSIDEIDIOTS_API ARIDemoWorldBuilder : public AActor
{
    GENERATED_BODY()
public:
    ARIDemoWorldBuilder();
    void BuildWorld(ARIRaceManager* InRaceManager, APlayerController* PlayerController);
private:
    void BuildRoute();
    void BuildTrackGeometry();
    void BuildCheckpoints(ARIRaceManager* RaceManager);
    void SpawnRacers(ARIRaceManager* RaceManager, APlayerController* PlayerController);
    AStaticMeshActor* SpawnBox(const FVector& Location, const FRotator& Rotation, const FVector& Scale);

    UPROPERTY() TObjectPtr<UStaticMesh> CubeMesh;
    TArray<FVector> RoutePoints;
    float RoadWidth = 900.0f;
};
