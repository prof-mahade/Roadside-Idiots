#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RIRacingLineFollower.generated.h"

class ARIBikePawn;
class ARITrafficVehicle;

/**
 * Low-level racing driver for AI bikes.
 *
 * The high-level ARIAIController owns personality, items and chaos decisions.
 * This actor owns only the safety-critical job of following the road smoothly.
 * Keeping those layers separate prevents combat/avoidance steering from fighting
 * the basic racing controller every frame.
 */
UCLASS()
class ROADSIDEIDIOTS_API ARIRacingLineFollower : public AActor
{
    GENERATED_BODY()

public:
    ARIRacingLineFollower();

    virtual void Tick(float DeltaSeconds) override;

    void Configure(ARIBikePawn* InBike, const TArray<FVector>& InRoutePoints, float InLaneOffset);

private:
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

    void UpdateTrafficPassPlan(
        float DeltaSeconds,
        const FVector& BikeLocation,
        const FVector& RouteTangent,
        float CurrentLateralOffset,
        float SpeedCms,
        float& InOutDesiredLaneOffset,
        float& OutTrafficSpeedScale);

    UPROPERTY()
    TObjectPtr<ARIBikePawn> Bike;

    TArray<FVector> RoutePoints;

    float BaseLaneOffset = 0.0f;
    float SmoothedLaneOffset = 0.0f;
    float SmoothedSteering = 0.0f;

    // VPR-25: pass commitment prevents the avoidance target from changing sides
    // every frame when traffic is near the centre of the bike's lane.
    TWeakObjectPtr<ARITrafficVehicle> ActiveTrafficTarget;
    float TrafficPassLaneOffset = 0.0f;
    float TrafficPassHoldRemaining = 0.0f;
    float TrafficPassCooldownRemaining = 0.0f;

    // Adaptive Pure Pursuit: distance grows with speed rather than steering at
    // a close waypoint. Values are centimetres / seconds because UE units are cm.
    float LookAheadTimeSeconds = 0.80f;
    float MinLookAheadCm = 1400.0f;
    float MaxLookAheadCm = 3600.0f;

    // This is deliberately inside the physical ~6.0 m road half-width. It is a
    // soft AI envelope, not another collision wall.
    float SafeCorridorHalfWidthCm = 440.0f;
    float LateralPredictionSeconds = 0.45f;

    float LaneInterpSpeed = 2.8f;
    float EmergencyLaneInterpSpeed = 8.5f;
    float SteeringInterpSpeed = 12.0f;

    // Traffic planning stays intentionally slower than the steering loop. Once
    // a pass side is chosen the AI commits to it long enough to actually clear
    // the car instead of left/right oscillating around the obstacle.
    float TrafficDetectionDistanceCm = 3400.0f;
    float TrafficLaneConflictCm = 190.0f;
    float TrafficPassClearanceCm = 255.0f;
    float TrafficPassHoldSeconds = 1.15f;
    float TrafficPassCooldownSeconds = 0.35f;

    // Mild AI-only lane spring. Old racing games often give computer drivers
    // hidden stability assistance; this remains force-based so collisions can
    // still move the bike, but small disturbances do not turn into wall ping-pong.
    float LaneAssistPositionGain = 0.72f;
    float LaneAssistVelocityGain = 1.35f;
    float MaxLaneAssistAccelCmS2 = 520.0f;
    float MaxRecoveryLaneAssistAccelCmS2 = 760.0f;

    float MaxTrackingLateralAccelCmS2 = 1050.0f;
};
