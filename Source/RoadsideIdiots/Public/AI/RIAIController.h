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

    FString GetPersonalityLabel() const { return PersonalityLabel; }
    float GetGrudgeTimeRemaining() const { return GrudgeTimeRemaining; }
    bool IsHoldingGrudgeAgainst(const ARIBikePawn* Target) const;

private:
    void ConfigurePersonality();
    void TryUseComedyItems();
    ARIBikePawn* FindBestItemVictim() const;
    bool FindUsefulPickupTarget(FVector& OutTarget) const;
    float ComputeAvoidanceShift(const FVector& BikeLocation, const FVector& Forward, const FVector& Right) const;

    UPROPERTY() TObjectPtr<ARIBikePawn> Bike;
    TArray<FVector> RoutePoints;
    int32 TargetIndex = 0;
    float LaneOffset = 0.0f;

    TWeakObjectPtr<ARIBikePawn> GrudgeTarget;
    float GrudgeTimeRemaining = 0.0f;
    float AttackCooldownRemaining = 0.0f;
    float EggUseCooldownRemaining = 0.0f;
    float PeelUseCooldownRemaining = 0.0f;
    FString PersonalityLabel = TEXT("IDIOT");

    // Cached awareness keeps expensive TActorIterator scans off the 20 Hz
    // steering loop. Movement remains responsive while sensing can scale better.
    FVector CachedPickupTarget = FVector::ZeroVector;
    bool bHasCachedPickupTarget = false;
    float CachedAvoidanceShift = 0.0f;
    float SenseRefreshRemaining = 0.0f;
    float ItemDecisionRemaining = 0.0f;
    float LowMotionTime = 0.0f;

    UPROPERTY(EditAnywhere, Category="AI Tuning") float TargetSpeedKph = 132.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning") float WaypointReachDistance = 520.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float GrudgeDurationSeconds = 8.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float GrudgeCatchupSpeedKph = 145.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float AttackRange = 235.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float AttackCooldownSeconds = 1.60f;

    UPROPERTY(EditAnywhere, Category="AI Tuning|Awareness") float PickupSeekRange = 1600.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Awareness") float AvoidanceStrength = 0.80f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Awareness") float SenseRefreshIntervalSeconds = 0.18f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Items") float ItemDecisionIntervalSeconds = 0.22f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Items") float EggUseCooldownSeconds = 3.6f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Items") float PeelUseCooldownSeconds = 4.2f;
};
