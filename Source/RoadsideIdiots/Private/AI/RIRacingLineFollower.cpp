#include "AI/RIRacingLineFollower.h"

#include "AI/RIAIController.h"
#include "Vehicle/RIBikePawn.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Traffic/RITrafficVehicle.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"

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

    ActiveRivalPassTarget.Reset();
    RivalPassLaneOffset = BaseLaneOffset;
    RivalPassHoldRemaining = 0.0f;
    RivalPassCooldownRemaining = 0.0f;

    ActiveTrafficTarget.Reset();
    TrafficPassLaneOffset = BaseLaneOffset;
    TrafficPassHoldRemaining = 0.0f;
    TrafficPassCooldownRemaining = 0.0f;
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

float ARIRacingLineFollower::ComputeRoutePreviewCurvature(
    const int32 SegmentIndex,
    const float SegmentAlpha,
    const float PreviewDistanceCm) const
{
    if (RoutePoints.Num() < 3) return 0.0f;

    const float Distance = FMath::Max(1000.0f, PreviewDistanceCm);
    const FVector P0 = SampleRouteAhead(SegmentIndex, SegmentAlpha, 0.0f, 0.0f, nullptr);
    const FVector P1 = SampleRouteAhead(SegmentIndex, SegmentAlpha, Distance * 0.33f, 0.0f, nullptr);
    const FVector P2 = SampleRouteAhead(SegmentIndex, SegmentAlpha, Distance * 0.66f, 0.0f, nullptr);
    const FVector P3 = SampleRouteAhead(SegmentIndex, SegmentAlpha, Distance, 0.0f, nullptr);

    auto CurvatureFromThree = [](const FVector& A, const FVector& B, const FVector& C)
    {
        const float AB = FVector::Dist2D(A, B);
        const float BC = FVector::Dist2D(B, C);
        const float AC = FVector::Dist2D(A, C);
        const float Denominator = AB * BC * AC;
        if (Denominator <= KINDA_SMALL_NUMBER) return 0.0f;

        FVector BA = B - A;
        FVector CA = C - A;
        BA.Z = 0.0f;
        CA.Z = 0.0f;
        const float DoubleArea = FMath::Abs(FVector::CrossProduct(BA, CA).Z);
        return (2.0f * DoubleArea) / Denominator;
    };

    return FMath::Max(
        CurvatureFromThree(P0, P1, P2),
        CurvatureFromThree(P1, P2, P3));
}

void ARIRacingLineFollower::UpdateRivalPassPlan(
    const float DeltaSeconds,
    const FVector& BikeLocation,
    const FVector& RouteTangent,
    const float CurrentLateralOffset,
    const float SpeedCms,
    const float UpcomingTurnSeverity,
    const float StrategicLaneStrength,
    float& InOutDesiredLaneOffset,
    float& OutRivalSpeedScale)
{
    OutRivalSpeedScale = 1.0f;
    if (!Bike || !GetWorld()) return;

    RivalPassHoldRemaining = FMath::Max(0.0f, RivalPassHoldRemaining - DeltaSeconds);
    RivalPassCooldownRemaining = FMath::Max(0.0f, RivalPassCooldownRemaining - DeltaSeconds);
    const FVector RouteRight = FVector::CrossProduct(FVector::UpVector, RouteTangent).GetSafeNormal();

    auto GetRivalFrame = [&](ARIBikePawn* Rival, float& OutAlong, float& OutLateral, float& OutRelativeSpeed)
    {
        if (!Rival || Rival == Bike || !Rival->AreRaceControlsEnabled()) return false;

        FVector RivalProjection;
        FVector RivalTangent;
        FVector RivalRight;
        int32 RivalSegment = 0;
        float RivalAlpha = 0.0f;
        if (!ProjectOntoRoute(
            Rival->GetActorLocation(),
            RivalProjection,
            RivalTangent,
            RivalRight,
            OutLateral,
            RivalSegment,
            RivalAlpha))
        {
            return false;
        }

        FVector ToRival = Rival->GetActorLocation() - BikeLocation;
        ToRival.Z = 0.0f;
        OutAlong = FVector::DotProduct(ToRival, RouteTangent);

        const FVector RivalVelocity = Rival->GetChassis()
            ? Rival->GetChassis()->GetPhysicsLinearVelocity()
            : FVector::ZeroVector;
        const float RivalForwardSpeed = FVector::DotProduct(RivalVelocity, RouteTangent);
        OutRelativeSpeed = SpeedCms - RivalForwardSpeed;
        return true;
    };

    if (ActiveRivalPassTarget.IsValid())
    {
        float Along = 0.0f;
        float Lateral = 0.0f;
        float RelativeSpeed = 0.0f;
        if (!GetRivalFrame(ActiveRivalPassTarget.Get(), Along, Lateral, RelativeSpeed) ||
            Along < -340.0f || Along > RivalDetectionDistanceCm * 1.35f)
        {
            if (RivalPassHoldRemaining <= 0.0f)
            {
                ActiveRivalPassTarget.Reset();
                RivalPassCooldownRemaining = RivalPassCooldownSeconds;
            }
        }
    }

    // A deliberate combat lane request is a higher-level decision than a routine
    // overtake. Pickup/hazard hints are weak enough that normal racecraft can
    // continue around them rather than freezing the pass planner.
    const bool bStrategicManeuverOwnsLane = StrategicLaneStrength > 0.70f;
    if (bStrategicManeuverOwnsLane && ActiveRivalPassTarget.IsValid() && RivalPassHoldRemaining <= 0.0f)
    {
        ActiveRivalPassTarget.Reset();
        RivalPassCooldownRemaining = RivalPassCooldownSeconds;
    }

    ARIBikePawn* BestRival = nullptr;
    float BestScore = 0.0f;
    float BestRivalLateral = 0.0f;

    // New passes begin only when there is enough road ahead to make the decision
    // look intentional. This prevents optimistic dive-bombs into a bend.
    if (!ActiveRivalPassTarget.IsValid() &&
        RivalPassCooldownRemaining <= 0.0f &&
        !bStrategicManeuverOwnsLane &&
        UpcomingTurnSeverity <= RivalPassMaxTurnSeverity)
    {
        for (TActorIterator<ARIBikePawn> It(GetWorld()); It; ++It)
        {
            ARIBikePawn* Rival = *It;
            if (!Rival || Rival == Bike || !Rival->AreRaceControlsEnabled()) continue;

            float Along = 0.0f;
            float RivalLateral = 0.0f;
            float RelativeSpeed = 0.0f;
            if (!GetRivalFrame(Rival, Along, RivalLateral, RelativeSpeed)) continue;
            if (Along < 170.0f || Along > RivalDetectionDistanceCm) continue;

            const float LaneGap = FMath::Abs(RivalLateral - SmoothedLaneOffset);
            if (LaneGap > RivalLaneConflictCm * 1.70f) continue;

            // If we are not actually catching the other rider, stay on the
            // efficient line instead of making decorative side-to-side moves.
            if (RelativeSpeed < RivalMinimumClosingSpeedCmS && Along > 850.0f) continue;

            const float TTC = Along / FMath::Max(RelativeSpeed, 180.0f);
            if (UpcomingTurnSeverity > 0.34f && TTC > 1.20f && Along > 700.0f)
            {
                // The gap will not realistically be completed before the bend.
                // Follow for a moment and attack after the corner instead.
                continue;
            }

            const float LaneThreat = 1.0f - FMath::Clamp(
                LaneGap / FMath::Max(1.0f, RivalLaneConflictCm * 1.70f),
                0.0f,
                1.0f);
            const float DistanceUrgency = 1.0f - FMath::Clamp(Along / RivalDetectionDistanceCm, 0.0f, 1.0f);
            const float ClosingUrgency = FMath::Clamp(
                (RelativeSpeed - RivalMinimumClosingSpeedCmS) / 850.0f,
                0.0f,
                1.0f);
            const float TTCUrgency = 1.0f - FMath::Clamp(TTC / 2.2f, 0.0f, 1.0f);
            const float StraightConfidence = 1.0f - FMath::Clamp(
                UpcomingTurnSeverity / FMath::Max(0.01f, RivalPassMaxTurnSeverity),
                0.0f,
                1.0f);
            const float Score = LaneThreat * 0.34f + DistanceUrgency * 0.24f +
                ClosingUrgency * 0.18f + TTCUrgency * 0.32f + StraightConfidence * 0.10f;

            if (Score > BestScore)
            {
                BestScore = Score;
                BestRival = Rival;
                BestRivalLateral = RivalLateral;
            }
        }

        if (BestRival && BestScore > 0.31f)
        {
            const float LaneLimit = SafeCorridorHalfWidthCm - 48.0f;
            const float LeftCandidate = FMath::Clamp(
                BestRivalLateral - RivalPassClearanceCm,
                -LaneLimit,
                LaneLimit);
            const float RightCandidate = FMath::Clamp(
                BestRivalLateral + RivalPassClearanceCm,
                -LaneLimit,
                LaneLimit);

            auto CandidateScore = [&](const float Candidate)
            {
                const float Separation = FMath::Abs(Candidate - BestRivalLateral);
                if (Separation < RivalLaneConflictCm * 0.88f)
                {
                    return TNumericLimits<float>::Max();
                }

                float Score = FMath::Abs(Candidate - CurrentLateralOffset);
                Score += FMath::Square(FMath::Abs(Candidate) / FMath::Max(1.0f, LaneLimit)) * 190.0f;
                Score += FMath::Abs(Candidate - BaseLaneOffset) * 0.08f;

                // Predict where nearby riders will be when this lane change is
                // actually established, not merely where they are this frame.
                for (TActorIterator<ARIBikePawn> OtherIt(GetWorld()); OtherIt; ++OtherIt)
                {
                    ARIBikePawn* Other = *OtherIt;
                    if (!Other || Other == Bike || Other == BestRival || !Other->AreRaceControlsEnabled()) continue;

                    float OtherAlong = 0.0f;
                    float OtherLateral = 0.0f;
                    float OtherRelativeSpeed = 0.0f;
                    if (!GetRivalFrame(Other, OtherAlong, OtherLateral, OtherRelativeSpeed)) continue;

                    const FVector OtherVelocity = Other->GetChassis()
                        ? Other->GetChassis()->GetPhysicsLinearVelocity()
                        : FVector::ZeroVector;
                    const float PredictedAlong = OtherAlong - OtherRelativeSpeed * RivalPredictionSeconds;
                    const float PredictedLateral = OtherLateral +
                        FVector::DotProduct(OtherVelocity, RouteRight) * RivalPredictionSeconds;

                    if (PredictedAlong < -120.0f || PredictedAlong > RivalDetectionDistanceCm * 0.72f) continue;

                    const float CorridorGap = FMath::Abs(PredictedLateral - Candidate);
                    if (CorridorGap < RivalLaneConflictCm * 1.40f)
                    {
                        const float LongitudinalPenalty = 1.0f - FMath::Clamp(
                            FMath::Abs(PredictedAlong) / (RivalDetectionDistanceCm * 0.72f),
                            0.0f,
                            1.0f);
                        Score += 560.0f * LongitudinalPenalty;
                    }
                }

                return Score;
            };

            const float LeftScore = CandidateScore(LeftCandidate);
            const float RightScore = CandidateScore(RightCandidate);
            RivalPassLaneOffset = LeftScore <= RightScore ? LeftCandidate : RightCandidate;
            ActiveRivalPassTarget = BestRival;
            RivalPassHoldRemaining = RivalPassHoldSeconds;
        }
    }

    if (!ActiveRivalPassTarget.IsValid() || bStrategicManeuverOwnsLane) return;

    float Along = 0.0f;
    float RivalLateral = 0.0f;
    float RelativeSpeed = 0.0f;
    if (!GetRivalFrame(ActiveRivalPassTarget.Get(), Along, RivalLateral, RelativeSpeed)) return;

    if (Along > -340.0f && Along < RivalDetectionDistanceCm * 1.35f)
    {
        const float CurrentSeparation = FMath::Abs(CurrentLateralOffset - RivalLateral);

        // If a significant bend arrives before we have established overlap,
        // tuck in and follow rather than making a late dive. Once side-by-side,
        // holding the selected corridor is safer than suddenly crossing back.
        const bool bTooLateForNewMove = UpcomingTurnSeverity > RivalPassMaxTurnSeverity && Along > 620.0f;
        if (!bTooLateForNewMove || CurrentSeparation > RivalLaneConflictCm * 0.75f)
        {
            const float LaneCommitStrength = Along > 2000.0f ? 0.50f : (Along > 650.0f ? 0.78f : 0.90f);
            InOutDesiredLaneOffset = FMath::Lerp(
                InOutDesiredLaneOffset,
                RivalPassLaneOffset,
                LaneCommitStrength);
        }
        else
        {
            OutRivalSpeedScale = FMath::Min(OutRivalSpeedScale, 0.95f);
        }

        // Follow the rider ahead instead of throwing away half our speed. The
        // progressively stronger reductions are only for genuinely closing gaps.
        if (Along < 1050.0f && CurrentSeparation < RivalLaneConflictCm * 1.10f)
        {
            OutRivalSpeedScale = FMath::Min(OutRivalSpeedScale, 0.96f);
        }
        if (Along < 650.0f && CurrentSeparation < RivalLaneConflictCm * 0.92f)
        {
            OutRivalSpeedScale = FMath::Min(OutRivalSpeedScale, 0.86f);
        }
        if (Along < 360.0f && CurrentSeparation < RivalLaneConflictCm * 0.78f)
        {
            OutRivalSpeedScale = FMath::Min(OutRivalSpeedScale, 0.70f);
        }
    }
}

void ARIRacingLineFollower::UpdateTrafficPassPlan(
    const float DeltaSeconds,
    const FVector& BikeLocation,
    const FVector& RouteTangent,
    const float CurrentLateralOffset,
    const float SpeedCms,
    float& InOutDesiredLaneOffset,
    float& OutTrafficSpeedScale)
{
    OutTrafficSpeedScale = 1.0f;
    if (!Bike || !GetWorld()) return;

    TrafficPassHoldRemaining = FMath::Max(0.0f, TrafficPassHoldRemaining - DeltaSeconds);
    TrafficPassCooldownRemaining = FMath::Max(0.0f, TrafficPassCooldownRemaining - DeltaSeconds);

    auto GetTrafficFrame = [&](ARITrafficVehicle* Traffic, float& OutAlong, float& OutLateral, float& OutRelativeSpeed)
    {
        if (!Traffic) return false;

        FVector TrafficProjection;
        FVector TrafficTangent;
        FVector TrafficRight;
        int32 TrafficSegment = 0;
        float TrafficAlpha = 0.0f;
        if (!ProjectOntoRoute(
            Traffic->GetActorLocation(),
            TrafficProjection,
            TrafficTangent,
            TrafficRight,
            OutLateral,
            TrafficSegment,
            TrafficAlpha))
        {
            return false;
        }

        FVector ToTraffic = Traffic->GetActorLocation() - BikeLocation;
        ToTraffic.Z = 0.0f;
        OutAlong = FVector::DotProduct(ToTraffic, RouteTangent);
        const float TrafficForwardSpeed = FVector::DotProduct(Traffic->GetTrafficVelocityEstimate(), RouteTangent);
        OutRelativeSpeed = SpeedCms - TrafficForwardSpeed;
        return true;
    };

    // Keep a selected car until it has actually been passed. This hysteresis is
    // the important bit: we do not recalculate "left or right?" every frame.
    if (ActiveTrafficTarget.IsValid())
    {
        float Along = 0.0f;
        float Lateral = 0.0f;
        float RelativeSpeed = 0.0f;
        if (!GetTrafficFrame(ActiveTrafficTarget.Get(), Along, Lateral, RelativeSpeed) ||
            Along < -360.0f || Along > TrafficDetectionDistanceCm * 1.35f)
        {
            if (TrafficPassHoldRemaining <= 0.0f)
            {
                ActiveTrafficTarget.Reset();
                TrafficPassCooldownRemaining = TrafficPassCooldownSeconds;
            }
        }
    }

    ARITrafficVehicle* BestThreat = nullptr;
    float BestThreatScore = 0.0f;
    float BestThreatLateral = 0.0f;

    // Only acquire a new vehicle when the previous pass decision has settled.
    if (!ActiveTrafficTarget.IsValid() && TrafficPassCooldownRemaining <= 0.0f)
    {
        for (TActorIterator<ARITrafficVehicle> It(GetWorld()); It; ++It)
        {
            ARITrafficVehicle* Traffic = *It;
            if (!Traffic) continue;

            float Along = 0.0f;
            float TrafficLateral = 0.0f;
            float RelativeSpeed = 0.0f;
            if (!GetTrafficFrame(Traffic, Along, TrafficLateral, RelativeSpeed)) continue;
            if (Along < 140.0f || Along > TrafficDetectionDistanceCm) continue;

            const float LaneGap = FMath::Abs(TrafficLateral - SmoothedLaneOffset);
            if (LaneGap > TrafficLaneConflictCm * 1.55f) continue;

            const float LaneThreat = 1.0f - FMath::Clamp(
                (LaneGap - TrafficLaneConflictCm * 0.45f) /
                FMath::Max(1.0f, TrafficLaneConflictCm * 1.10f),
                0.0f,
                1.0f);
            const float DistanceThreat = 1.0f - FMath::Clamp(Along / TrafficDetectionDistanceCm, 0.0f, 1.0f);
            const float TTC = Along / FMath::Max(RelativeSpeed, 220.0f);
            const float TTCThreat = 1.0f - FMath::Clamp(TTC / 2.4f, 0.0f, 1.0f);
            const float ThreatScore = LaneThreat * FMath::Max(DistanceThreat * 0.70f, TTCThreat);

            if (ThreatScore > BestThreatScore)
            {
                BestThreatScore = ThreatScore;
                BestThreat = Traffic;
                BestThreatLateral = TrafficLateral;
            }
        }

        if (BestThreat && BestThreatScore > 0.16f)
        {
            const float LaneLimit = SafeCorridorHalfWidthCm - 42.0f;
            const float LeftCandidate = FMath::Clamp(
                BestThreatLateral - TrafficPassClearanceCm,
                -LaneLimit,
                LaneLimit);
            const float RightCandidate = FMath::Clamp(
                BestThreatLateral + TrafficPassClearanceCm,
                -LaneLimit,
                LaneLimit);

            auto CandidateScore = [&](const float Candidate)
            {
                const float Separation = FMath::Abs(Candidate - BestThreatLateral);
                if (Separation < TrafficLaneConflictCm * 0.82f)
                {
                    return TNumericLimits<float>::Max();
                }

                const float MoveCost = FMath::Abs(Candidate - CurrentLateralOffset);
                const float EdgeCost = FMath::Square(FMath::Abs(Candidate) / FMath::Max(1.0f, LaneLimit)) * 210.0f;
                const float HomeLaneCost = FMath::Abs(Candidate - BaseLaneOffset) * 0.10f;
                return MoveCost + EdgeCost + HomeLaneCost;
            };

            const float LeftScore = CandidateScore(LeftCandidate);
            const float RightScore = CandidateScore(RightCandidate);
            TrafficPassLaneOffset = LeftScore <= RightScore ? LeftCandidate : RightCandidate;
            ActiveTrafficTarget = BestThreat;
            TrafficPassHoldRemaining = TrafficPassHoldSeconds;
        }
    }

    if (!ActiveTrafficTarget.IsValid()) return;

    float Along = 0.0f;
    float TrafficLateral = 0.0f;
    float RelativeSpeed = 0.0f;
    if (!GetTrafficFrame(ActiveTrafficTarget.Get(), Along, TrafficLateral, RelativeSpeed)) return;

    if (Along > -360.0f && Along < TrafficDetectionDistanceCm * 1.35f)
    {
        const float LaneCommitStrength = Along > 2600.0f ? 0.48f : (Along > 1200.0f ? 0.76f : 0.90f);
        InOutDesiredLaneOffset = FMath::Lerp(
            InOutDesiredLaneOffset,
            TrafficPassLaneOffset,
            LaneCommitStrength);

        // Passing is preferred over braking. Braking is progressive and remains
        // much less destructive to race pace than the previous double safety cap.
        const float CurrentSeparation = FMath::Abs(CurrentLateralOffset - TrafficLateral);
        if (Along < 1550.0f && CurrentSeparation < TrafficLaneConflictCm * 1.18f)
        {
            OutTrafficSpeedScale = FMath::Min(OutTrafficSpeedScale, 0.94f);
        }
        if (Along < 980.0f && CurrentSeparation < TrafficLaneConflictCm)
        {
            OutTrafficSpeedScale = FMath::Min(OutTrafficSpeedScale, 0.82f);
        }
        if (Along < 520.0f && CurrentSeparation < TrafficLaneConflictCm * 0.82f)
        {
            OutTrafficSpeedScale = FMath::Min(OutTrafficSpeedScale, 0.60f);
        }
    }
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
    const float LateralRisk = FMath::GetMappedRangeValueClamped(
        FVector2D(SoftLimit, HardLimit),
        FVector2D(0.0f, 1.0f),
        FMath::Abs(PredictedLateralOffset));

    // Collision recovery also watches heading. A bike knocked mostly sideways
    // should temporarily forget overtaking and get itself aligned with the road.
    const float RouteHeadingDot = FVector::DotProduct(Forward, RouteTangent);
    const float HeadingRisk = 1.0f - FMath::GetMappedRangeValueClamped(
        FVector2D(0.18f, 0.76f),
        FVector2D(0.0f, 1.0f),
        RouteHeadingDot);
    const float StabilityRisk = FMath::Max(LateralRisk, HeadingRisk);

    // Normal state: follow the assigned race lane. Recovery state: progressively
    // abandon the lane and aim toward the road centre. No threshold toggling.
    float DesiredLaneOffset = FMath::Lerp(BaseLaneOffset, 0.0f, StabilityRisk);

    // Connect the high-level brain to the stable driver through a bounded lane
    // request. It can ask for a maneuver, but it still cannot issue steering.
    const ARIAIController* AIController = Cast<ARIAIController>(Bike->GetController());
    float StrategicLaneStrength = 0.0f;
    if (AIController)
    {
        float StrategicLaneOffset = BaseLaneOffset;
        if (AIController->GetStrategicLaneRequest(StrategicLaneOffset, StrategicLaneStrength))
        {
            const float LaneLimit = SafeCorridorHalfWidthCm - 55.0f;
            StrategicLaneOffset = FMath::Clamp(StrategicLaneOffset, -LaneLimit, LaneLimit);
            DesiredLaneOffset = FMath::Lerp(
                DesiredLaneOffset,
                StrategicLaneOffset,
                StrategicLaneStrength * (1.0f - StabilityRisk));
        }
    }

    // Look farther than the immediate steering carrot before committing to a
    // wheel-to-wheel pass. Passing into a hard bend is usually worse racecraft
    // than following for another moment and attacking on the next open section.
    FVector ManeuverPreviewTangent;
    SampleRouteAhead(RouteSegment, RouteAlpha, 2400.0f, 0.0f, &ManeuverPreviewTangent);
    const float PreviewDot = FMath::Clamp(FVector::DotProduct(RouteTangent, ManeuverPreviewTangent), -1.0f, 1.0f);
    const float PreviewAngle = FMath::Abs(FMath::Atan2(
        FVector::CrossProduct(RouteTangent, ManeuverPreviewTangent).Z,
        PreviewDot));
    const float UpcomingTurnSeverity = FMath::Clamp(PreviewAngle / 0.72f, 0.0f, 1.0f);

    float RivalSpeedScale = 1.0f;
    UpdateRivalPassPlan(
        DeltaSeconds,
        BikeLocation,
        RouteTangent,
        CurrentLateralOffset,
        SpeedCms,
        UpcomingTurnSeverity,
        StrategicLaneStrength,
        DesiredLaneOffset,
        RivalSpeedScale);

    // Civilian traffic is the stronger safety constraint and is therefore
    // evaluated after normal racing/overtaking intent.
    float TrafficSpeedScale = 1.0f;
    UpdateTrafficPassPlan(
        DeltaSeconds,
        BikeLocation,
        RouteTangent,
        CurrentLateralOffset,
        SpeedCms,
        DesiredLaneOffset,
        TrafficSpeedScale);

    // Recovery always wins over every pass/tactical plan. Planning timers remain
    // alive so a collision does not freeze decision state, but the physical bike
    // first returns to a stable road-centre trajectory.
    if (StabilityRisk > 0.0f)
    {
        DesiredLaneOffset = FMath::Lerp(DesiredLaneOffset, 0.0f, StabilityRisk);
    }

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
    // player's steering forces. During genuine collision recovery it may become
    // somewhat stronger, but it remains force-based rather than teleporting.
    if (Movement->IsGrounded())
    {
        const float LaneError = SmoothedLaneOffset - CurrentLateralOffset;
        const float MaxAssist = FMath::Lerp(
            MaxLaneAssistAccelCmS2,
            MaxRecoveryLaneAssistAccelCmS2,
            StabilityRisk);
        const float AssistAccel = FMath::Clamp(
            LaneError * LaneAssistPositionGain - LateralSpeed * LaneAssistVelocityGain,
            -MaxAssist,
            MaxAssist);
        Chassis->AddForce(RouteRight * AssistAccel, NAME_None, true);
    }

    // Pace uses ROAD curvature, not Pure-Pursuit curvature. A lane change can
    // create a large temporary pursuit curvature on a straight; treating that as
    // a corner was one of the reasons overtaking bots unnecessarily lost speed.
    const float PaceSpeedAlpha = FMath::Clamp(SpeedKph / 150.0f, 0.0f, 1.0f);
    const float PacePreviewDistance = FMath::Lerp(PacePreviewMinCm, PacePreviewMaxCm, PaceSpeedAlpha);
    const float RoadCurvature = ComputeRoutePreviewCurvature(RouteSegment, RouteAlpha, PacePreviewDistance);

    float TrackingSpeedLimitKph = 155.0f;
    if (RoadCurvature > 0.000001f)
    {
        TrackingSpeedLimitKph = FMath::Clamp(
            FMath::Sqrt(MaxTrackingLateralAccelCmS2 / RoadCurvature) * 0.036f,
            MinimumTrackingSpeedKph,
            155.0f);
    }

    // The most restrictive nearby interaction wins. These are now mild following
    // caps rather than a second complete speed planner, so a bot can stay in a
    // pack without repeatedly falling tens of km/h below the rider it is chasing.
    TrackingSpeedLimitKph *= FMath::Clamp(
        FMath::Min(RivalSpeedScale, TrafficSpeedScale),
        0.58f,
        1.0f);

    if (StabilityRisk > 0.0f)
    {
        TrackingSpeedLimitKph = FMath::Min(
            TrackingSpeedLimitKph,
            FMath::Lerp(108.0f, 38.0f, StabilityRisk));
    }

    if (LocalForward < 250.0f)
    {
        TrackingSpeedLimitKph = FMath::Min(TrackingSpeedLimitKph, 34.0f);
    }

    const float RequestedRacePaceKph = AIController
        ? AIController->GetProfessionalPaceTargetKph()
        : 150.0f;
    const float DesiredPaceKph = FMath::Min(RequestedRacePaceKph, TrackingSpeedLimitKph);

    // The high-level controller runs first and may still contain conservative
    // legacy throttle logic. The racing driver owns the FINAL forward pace just
    // like it owns final steering. Preserve explicit reverse recovery, otherwise
    // target the safe race pace with a simple closed-loop throttle/brake request.
    const float ExistingThrottle = Movement->GetThrottleInput();
    const float ExistingBrake = Movement->GetBrakeInput();
    if (ExistingThrottle >= -0.08f)
    {
        const float PaceError = DesiredPaceKph - SpeedKph;
        float ThrottleRequest = 0.0f;
        float BrakeRequest = 0.0f;

        if (PaceError > 2.0f)
        {
            ThrottleRequest = FMath::Clamp(0.58f + PaceError / 20.0f, 0.58f, 1.0f);
        }
        else if (PaceError < -2.0f)
        {
            BrakeRequest = FMath::Clamp((-PaceError) / 30.0f, 0.10f, 0.86f);
        }
        else
        {
            // Small maintenance throttle prevents the old saw-tooth behavior of
            // coasting below the target and then accelerating hard again.
            ThrottleRequest = FMath::Clamp(0.30f + PaceError * 0.035f, 0.20f, 0.40f);
        }

        // If the high-level brain has a non-routine lane request and no active
        // moving-object pass, retain some of its emergency braking. This mainly
        // protects static peel/poop avoidance without restoring double braking
        // for normal rivals and civilian traffic.
        if (StrategicLaneStrength > 0.31f &&
            !ActiveRivalPassTarget.IsValid() &&
            !ActiveTrafficTarget.IsValid())
        {
            BrakeRequest = FMath::Max(BrakeRequest, ExistingBrake * 0.55f);
            if (BrakeRequest > 0.18f)
            {
                ThrottleRequest = FMath::Min(ThrottleRequest, 0.12f);
            }
        }

        Movement->SetThrottleInput(ThrottleRequest);
        Movement->SetBrakeInput(BrakeRequest);
    }
}