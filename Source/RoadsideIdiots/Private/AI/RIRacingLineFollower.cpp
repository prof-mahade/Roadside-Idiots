#include "AI/RIRacingLineFollower.h"

#include "Vehicle/RIBikePawn.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Components/StaticMeshComponent.h"

ARIRacingLineFollower::ARIRacingLineFollower()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
    PrimaryActorTick.TickInterval = 0.0f;
    SetActorEnableCollision(false);
}

void ARIRacingLineFollower::Configure(
    ARIBikePawn* InBike,
    const TArray<FVector>& InRoutePoints,
    const float InLaneOffset)
{
    Bike = InBike;
    RoutePoints = InRoutePoints;
    BaseLaneOffset = FMath::Clamp(InLaneOffset, -300.0f, 300.0f);
    SmoothedLaneOffset = BaseLaneOffset;
    SmoothedSteering = 0.0f;
}

bool ARIRacingLineFollower::ProjectOntoRoute(
    const FVector& WorldLocation,
    FVector& OutProjection,
    FVector& OutTangent,
    FVector& OutRight,
    float& OutLateralOffset,
    int32& OutSegmentIndex,
    float& OutSegmentAlpha) const
{
    if (RoutePoints.Num() < 2) return false;

    float BestDistanceSq = TNumericLimits<float>::Max();
    bool bFound = false;

    for (int32 Index = 0; Index < RoutePoints.Num(); ++Index)
    {
        const FVector A = RoutePoints[Index];
        const FVector B = RoutePoints[(Index + 1) % RoutePoints.Num()];

        FVector AB = B - A;
        AB.Z = 0.0f;
        const float LengthSq = AB.SizeSquared();
        if (LengthSq <= KINDA_SMALL_NUMBER) continue;

        FVector AP = WorldLocation - A;
        AP.Z = 0.0f;
        const float Alpha = FMath::Clamp(FVector::DotProduct(AP, AB) / LengthSq, 0.0f, 1.0f);
        const FVector Projection = A + AB * Alpha;
        const float DistanceSq = FVector::DistSquared2D(WorldLocation, Projection);

        if (DistanceSq < BestDistanceSq)
        {
            BestDistanceSq = DistanceSq;
            OutProjection = Projection;
            OutTangent = AB.GetSafeNormal2D();
            OutRight = FVector::CrossProduct(FVector::UpVector, OutTangent).GetSafeNormal();

            FVector Delta = WorldLocation - Projection;
            Delta.Z = 0.0f;
            OutLateralOffset = FVector::DotProduct(Delta, OutRight);
            OutSegmentIndex = Index;
            OutSegmentAlpha = Alpha;
            bFound = true;
        }
    }

    return bFound;
}

FVector ARIRacingLineFollower::SampleRouteAhead(
    int32 SegmentIndex,
    float SegmentAlpha,
    float DistanceCm,
    float LateralOffset,
    FVector* OutTangent) const
{
    if (RoutePoints.Num() < 2)
    {
        if (OutTangent) *OutTangent = FVector::ForwardVector;
        return FVector::ZeroVector;
    }

    const int32 Count = RoutePoints.Num();
    SegmentIndex = (SegmentIndex % Count + Count) % Count;
    SegmentAlpha = FMath::Clamp(SegmentAlpha, 0.0f, 1.0f);

    float Remaining = FMath::Max(0.0f, DistanceCm);
    FVector A = RoutePoints[SegmentIndex];
    FVector B = RoutePoints[(SegmentIndex + 1) % Count];
    FVector Current = FMath::Lerp(A, B, SegmentAlpha);

    for (int32 Step = 0; Step <= Count; ++Step)
    {
        B = RoutePoints[(SegmentIndex + 1) % Count];
        FVector ToEnd = B - Current;
        ToEnd.Z = 0.0f;
        const float Available = ToEnd.Size();
        const FVector Tangent = Available > KINDA_SMALL_NUMBER
            ? ToEnd / Available
            : (RoutePoints[(SegmentIndex + 2) % Count] - B).GetSafeNormal2D();

        if (Remaining <= Available || Step == Count)
        {
            const FVector CenterPoint = Available > KINDA_SMALL_NUMBER
                ? Current + Tangent * Remaining
                : Current;
            const FVector Right = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();
            if (OutTangent) *OutTangent = Tangent;
            return CenterPoint + Right * LateralOffset;
        }

        Remaining -= Available;
        SegmentIndex = (SegmentIndex + 1) % Count;
        Current = RoutePoints[SegmentIndex];
    }

    if (OutTangent) *OutTangent = FVector::ForwardVector;
    return Current;
}

void ARIRacingLineFollower::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!Bike || RoutePoints.Num() < 3) return;

    URIBikeMovementComponent* Movement = Bike->GetBikeMovement();
    UStaticMeshComponent* Chassis = Bike->GetChassis();
    if (!Movement || !Chassis) return;

    if (!Bike->AreRaceControlsEnabled())
    {
        SmoothedSteering = 0.0f;
        Movement->SetSteeringInput(0.0f);
        return;
    }

    const FVector BikeLocation = Bike->GetActorLocation();
    const FVector Forward = Bike->GetActorForwardVector().GetSafeNormal2D();
    const FVector BikeRight = Bike->GetActorRightVector().GetSafeNormal2D();

    FVector Projection;
    FVector RouteTangent;
    FVector RouteRight;
    float CurrentLateralOffset = 0.0f;
    int32 RouteSegment = 0;
    float RouteAlpha = 0.0f;
    if (!ProjectOntoRoute(
        BikeLocation,
        Projection,
        RouteTangent,
        RouteRight,
        CurrentLateralOffset,
        RouteSegment,
        RouteAlpha))
    {
        Movement->SetSteeringInput(0.0f);
        return;
    }

    const FVector Velocity = Chassis->GetPhysicsLinearVelocity();
    const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.0f);
    const float SpeedCms = HorizontalVelocity.Size();
    const float SpeedKph = SpeedCms * 0.036f;
    const float LateralSpeed = FVector::DotProduct(HorizontalVelocity, RouteRight);

    // Predict where the bike is drifting, rather than waiting until its current
    // position is already beside a barrier. This is continuous and therefore
    // does not flip left/right like the old wall-ray emergency steering.
    const float PredictedLateralOffset =
        CurrentLateralOffset + LateralSpeed * LateralPredictionSeconds;

    const float SoftLimit = SafeCorridorHalfWidthCm * 0.68f;
    const float HardLimit = SafeCorridorHalfWidthCm * 1.08f;
    const float StabilityRisk = FMath::GetMappedRangeValueClamped(
        FVector2D(SoftLimit, HardLimit),
        FVector2D(0.0f, 1.0f),
        FMath::Abs(PredictedLateralOffset));

    // Normal state: follow the assigned race lane. Recovery state: progressively
    // abandon the lane and aim toward the road centre. No threshold toggling.
    const float DesiredLaneOffset = FMath::Lerp(BaseLaneOffset, 0.0f, StabilityRisk);
    const float LaneResponse = FMath::Lerp(
        LaneInterpSpeed,
        EmergencyLaneInterpSpeed,
        StabilityRisk);
    SmoothedLaneOffset = FMath::FInterpTo(
        SmoothedLaneOffset,
        DesiredLaneOffset,
        DeltaSeconds,
        LaneResponse);

    // Adaptive Pure Pursuit. At 150 km/h this naturally looks roughly 33 m
    // ahead; around a 65 km/h corner it looks about 14-15 m ahead.
    const float LookAheadCm = FMath::Clamp(
        SpeedCms * LookAheadTimeSeconds,
        MinLookAheadCm,
        MaxLookAheadCm);

    FVector CarrotTangent;
    const FVector Carrot = SampleRouteAhead(
        RouteSegment,
        RouteAlpha,
        LookAheadCm,
        SmoothedLaneOffset,
        &CarrotTangent);

    FVector ToCarrot = Carrot - BikeLocation;
    ToCarrot.Z = 0.0f;
    const float CarrotDistanceSq = FMath::Max(ToCarrot.SizeSquared(), FMath::Square(250.0f));
    const float LocalForward = FVector::DotProduct(ToCarrot, Forward);
    const float LocalRight = FVector::DotProduct(ToCarrot, BikeRight);

    // Pure Pursuit curvature: k = 2*y/L^2. UE centimetres cancel correctly:
    // (cm/s) * (1/cm) = radians/s desired yaw rate.
    const float PursuitCurvature = 2.0f * LocalRight / CarrotDistanceSq;
    const float ControlSpeedCms = FMath::Max(SpeedCms, 900.0f);
    const float DesiredYawRate = ControlSpeedCms * PursuitCurvature;

    // Keep this normalization synchronized with RIBikeMovementComponent's
    // closed-loop yaw-rate actuator. The driver requests a yaw rate; movement
    // physics decides how much torque is needed to achieve it.
    const float HandlingSpeedAlpha = FMath::Clamp(SpeedKph / 130.0f, 0.0f, 1.0f);
    const float AvailableYawRate = FMath::Lerp(1.55f, 0.82f, HandlingSpeedAlpha);
    float RawSteering = FMath::Clamp(
        DesiredYawRate / FMath::Max(0.25f, AvailableYawRate),
        -1.0f,
        1.0f);

    // If a collision has spun the motorcycle far enough that the carrot is no
    // longer clearly in front, stop feeding an unstable full-speed correction.
    if (LocalForward < 250.0f)
    {
        RawSteering *= 0.55f;
    }

    SmoothedSteering = FMath::FInterpTo(
        SmoothedSteering,
        RawSteering,
        DeltaSeconds,
        SteeringInterpSpeed);
    Movement->SetSteeringInput(SmoothedSteering);

    // Small AI-only lateral spring. This is intentionally much weaker than the
    // player's steering forces. It damps collision drift without teleporting or
    // setting transforms, so opponents can still be shoved and can still crash.
    if (Movement->IsGrounded())
    {
        const float LaneError = SmoothedLaneOffset - CurrentLateralOffset;
        const float AssistAccel = FMath::Clamp(
            LaneError * LaneAssistPositionGain - LateralSpeed * LaneAssistVelocityGain,
            -MaxLaneAssistAccelCmS2,
            MaxLaneAssistAccelCmS2);
        Chassis->AddForce(RouteRight * AssistAccel, NAME_None, true);
    }

    // Curvature regulation is a second safety net. The high-level AI still owns
    // normal throttle/brake decisions, but the racing driver may veto excessive
    // speed when the currently required tracking arc cannot be held safely.
    const float AbsCurvature = FMath::Abs(PursuitCurvature);
    float TrackingSpeedLimitKph = 155.0f;
    if (AbsCurvature > 0.000001f)
    {
        TrackingSpeedLimitKph = FMath::Clamp(
            FMath::Sqrt(MaxTrackingLateralAccelCmS2 / AbsCurvature) * 0.036f,
            45.0f,
            155.0f);
    }

    if (StabilityRisk > 0.0f)
    {
        TrackingSpeedLimitKph = FMath::Min(
            TrackingSpeedLimitKph,
            FMath::Lerp(100.0f, 42.0f, StabilityRisk));
    }

    if (LocalForward < 250.0f)
    {
        TrackingSpeedLimitKph = FMath::Min(TrackingSpeedLimitKph, 38.0f);
    }

    const float SpeedExcess = SpeedKph - TrackingSpeedLimitKph;
    if (SpeedExcess > 2.0f)
    {
        const float BrakeRequest = FMath::Clamp(SpeedExcess / 32.0f, 0.12f, 0.78f);
        Movement->SetThrottleInput(FMath::Min(Movement->GetThrottleInput(), 0.08f));
        Movement->SetBrakeInput(FMath::Max(Movement->GetBrakeInput(), BrakeRequest));
    }
}
