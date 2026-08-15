#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "RIAIController.generated.h"

class ARIBikePawn;

enum class ERITacticalIntent : uint8
{
    None,
    SidePressure,
    Block,
    PeelTrap,
    EggShot
};

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

    bool AssignTacticalIntent(ARIBikePawn* Target, ERITacticalIntent Intent, float DurationSeconds);
    bool IsTacticalIntentActive() const;
    float GetTacticalTimeRemaining() const { return TacticalTimeRemaining; }
    float GetTacticalCooldownRemaining() const { return TacticalCooldownRemaining; }
    ERITacticalIntent GetTacticalIntent() const { return TacticalIntent; }
    ARIBikePawn* GetTacticalTarget() const { return TacticalTarget.Get(); }

    FString GetPersonalityLabel() const { return PersonalityLabel; }
    float GetGrudgeTimeRemaining() const { return GrudgeTimeRemaining; }
    bool IsHoldingGrudgeAgainst(const ARIBikePawn* Target) const;

    void SetDirectorRoleLabel(const FString& InLabel)
    {
        if (!InLabel.IsEmpty()) PersonalityLabel = InLabel;
    }

private:
    void ConfigurePersonality();
    void TryUseComedyItems();
    void EndTacticalIntent(float CooldownSeconds = 0.0f);

    ARIBikePawn* FindBestItemVictim() const;
    bool FindUsefulPickupTarget(FVector& OutTarget) const;
    float ComputeAvoidanceShift(const FVector& BikeLocation, const FVector& Forward, const FVector& Right) const;
    float ComputeCrowdSpeedScale(const FVector& BikeLocation, const FVector& Forward, const FVector& Right) const;
    FVector ComputeRaceLookAheadTarget(const FVector& BikeLocation, float SpeedKph, FVector& OutTangent, FVector& OutRight, float& OutTurnSeverity);
    void ApplyTacticalTargeting(FVector& InOutTargetPoint, const FVector& BikeLocation, const FVector& RouteRight);

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

    TWeakObjectPtr<ARIBikePawn> TacticalTarget;
    ERITacticalIntent TacticalIntent = ERITacticalIntent::None;
    float TacticalTimeRemaining = 0.0f;
    float TacticalCooldownRemaining = 0.0f;
    bool bTacticalItemCommitted = false;
    float TacticalSideSign = 1.0f;

    FVector CachedPickupTarget = FVector::ZeroVector;
    bool bHasCachedPickupTarget = false;
    float CachedAvoidanceShift = 0.0f;
    float SmoothedAvoidanceShift = 0.0f;
    float CachedCrowdSpeedScale = 1.0f;
    float SenseRefreshRemaining = 0.0f;
    float ItemDecisionRemaining = 0.0f;
    float LowMotionTime = 0.0f;
    float SmoothedSteering = 0.0f;
    float SmoothedThrottle = 0.0f;
    float SmoothedBrake = 0.0f;

    UPROPERTY(EditAnywhere, Category="AI Tuning") float TargetSpeedKph = 112.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning") float WaypointReachDistance = 520.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float MinLookAheadDistance = 620.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float MaxLookAheadDistance = 1550.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float SteeringInterpSpeed = 5.5f;

    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float GrudgeDurationSeconds = 4.5f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float GrudgeCatchupSpeedKph = 121.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float AttackRange = 235.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float AttackCooldownSeconds = 1.60f;

    UPROPERTY(EditAnywhere, Category="AI Tuning|Awareness") float PickupSeekRange = 1600.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Awareness") float AvoidanceStrength = 0.92f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Awareness") float SenseRefreshIntervalSeconds = 0.16f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Awareness") float CrowdLookAhead = 1050.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Awareness") float CrowdSideClearance = 235.0f;

    UPROPERTY(EditAnywhere, Category="AI Tuning|Items") float ItemDecisionIntervalSeconds = 0.28f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Items") float EggUseCooldownSeconds = 4.5f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Items") float PeelUseCooldownSeconds = 5.0f;
};
