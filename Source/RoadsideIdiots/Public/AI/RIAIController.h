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
private:
    UPROPERTY() TObjectPtr<ARIBikePawn> Bike;
    TArray<FVector> RoutePoints;
    int32 TargetIndex = 0;
    float LaneOffset = 0.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning") float TargetSpeedKph = 132.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning") float WaypointReachDistance = 520.0f;
};
