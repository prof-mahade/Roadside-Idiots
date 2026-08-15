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

    bool ProjectOntoRoute(
        const FVector& WorldLocation,
        FVector& OutProjection,
        FVector& OutTangent,
        FVector& OutRight,
        float& OutLateralOffset,
        int32& OutSegmentIndex,
        float& OutSegmentAlpha) const;

    FVector SampleRouteAhead(
        int32 SegmentIndex,
        float SegmentAlpha,
        float DistanceCm,
        float LateralOffset,
        FVector* OutTangent = nullptr) const;

    float ComputePreviewCurvature(int32 SegmentIndex, float SegmentAlpha, float PreviewDistanceCm) const;
    float ComputeAvoidanceShift(const FVector& BikeLocation, const FVector& PathForward, const FVector& RouteRight, float CurrentLateralOffset) const;
    float ComputeCrowdSpeedScale(const FVector& BikeLocation, const FVector& PathForward, const FVector& RouteRight) const;
    float ComputeWallTraceSteer(const FVector& BikeLocation, const FVector& Forward, float& OutWallSpeedScale) const;
    void ApplyTacticalLanePlan(float& InOutDesiredLaneOffset, const FVector& BikeLocation, float TurnSeverity);

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
    float CachedCrowdSpeedScale = 1.0f;
    float SenseRefreshRemaining = 0.0f;
    float ItemDecisionRemaining = 0.0f;
    float LowMotionTime = 0.0f;
    float SmoothedLaneOffset = 0.0f;
    float SmoothedSteering = 0.0f;
    float SmoothedThrottle = 0.0f;
    float SmoothedBrake = 0.0f;

    // VPR-24D: keep the fast straight-line target, but use a realistic corner
    // acceleration budget and start planning turns well before reaching them.
    UPROPERTY(EditAnywhere, Category="AI Tuning|Speed") float TargetSpeedKph = 146.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Speed") float MaxLateralAccelCmS2 = 1150.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Speed") float MinimumCornerSpeedKph = 50.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Speed") float CurvePreviewMinDistance = 3400.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Speed") float CurvePreviewMaxDistance = 5800.0f;

    // Prefer anticipation and damping over snapping back toward the line.
    // Short lookahead + aggressive cross-track correction was feeding the
    // left/right overcorrection cycle visible in the wall-hit tests.
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float MinLookAheadDistance = 1400.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float MaxLookAheadDistance = 3600.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float HeadingGain = 0.82f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float CrossTrackGain = 3.1f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float StanleySofteningSpeedCmS = 850.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float CurvatureFeedForwardDistance = 3000.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float SteeringCommandRadians = 0.64f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float SteeringInterpSpeed = 6.5f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float LaneChangeInterpSpeed = 3.2f;

    // AI safety corridor only. The physical/visual road remains 12 m wide.
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float SafeRoadHalfWidth = 320.0f;

    // Short emergency feeler only. Normal cornering is path-controlled.
    UPROPERTY(EditAnywhere, Category="AI Tuning|Path") float WallTraceLength = 700.0f;

    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float GrudgeDurationSeconds = 4.5f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float GrudgeCatchupSpeedKph = 150.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float AttackRange = 235.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Retaliation") float AttackCooldownSeconds = 1.60f;

    UPROPERTY(EditAnywhere, Category="AI Tuning|Awareness") float PickupSeekRange = 1600.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Awareness") float AvoidanceStrength = 0.92f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Awareness") float SenseRefreshIntervalSeconds = 0.14f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Awareness") float CrowdLookAhead = 3000.0f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Awareness") float CrowdSideClearance = 260.0f;

    UPROPERTY(EditAnywhere, Category="AI Tuning|Items") float ItemDecisionIntervalSeconds = 0.28f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Items") float EggUseCooldownSeconds = 4.5f;
    UPROPERTY(EditAnywhere, Category="AI Tuning|Items") float PeelUseCooldownSeconds = 5.0f;
};
