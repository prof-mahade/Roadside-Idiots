#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RICheckpoint.generated.h"

class UBoxComponent;
class ARIRaceManager;

UCLASS()
class ROADSIDEIDIOTS_API ARICheckpoint : public AActor
{
    GENERATED_BODY()
public:
    ARICheckpoint();
    void Configure(ARIRaceManager* InRaceManager, int32 InCheckpointIndex, const FVector& BoxExtent);
private:
    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> Trigger;
    UPROPERTY() TObjectPtr<ARIRaceManager> RaceManager;
    int32 CheckpointIndex = 0;
};
