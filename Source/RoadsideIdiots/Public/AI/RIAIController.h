#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "RIAIController.generated.h"

class ARIBikePawn;

UCLASS()
class ROADSIDEIDIOTS_API ARIAIController : public AAIController
{
    GENERATED_BODY()
public:
    ARIAIController();
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnPossess(APawn* InPawn) override;
    void SetRoute(const TArray<FVector>& InRoutePoints, int32 StartTargetIndex, float InLaneOffset);
    void NotifyProvokedBy(ARIBikePawn* InstigatorBike);

private:
    UPROPERTY() TObjectPtr<ARIBikePawn> Bike;
    TArray<FVector> RoutePoints;
    int32 TargetIndex = 0;
    float LaneOffset = 0.0f;

    TWeakObjectPtr<ARIBikePawn> GrudgeTarget;
    float GrudgeTimeRemaining = 0.0f;

    UPROPERTY(EditAnywhere, Category="AI Tuning") float TargetSpeedKph = 132.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning") float WaypointReachDistance = 520.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float GrudgeDurationSeconds = 8.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float GrudgeCatchupSpeedKph = 145.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float AttackRange = 235.0f;
};
