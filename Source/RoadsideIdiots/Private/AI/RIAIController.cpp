#include "AI/RIAIController.h"
#include "Vehicle/RIBikePawn.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Interaction/RIInteractionComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Core/RIHealthComponent.h"
#include "Items/RIBananaPickup.h"
#include "Items/RIBananaPeelHazard.h"
#include "Items/RIRottenEggPickup.h"
#include "Hazards/RIPoopHazard.h"
#include "Traffic/RITrafficVehicle.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"

ARIAIController::ARIAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f;
}

void ARIAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    Bike = Cast<ARIBikePawn>(InPawn);
    ConfigurePersonality();
}

void ARIAIController::ConfigurePersonality()
{
    if (!Bike || !Bike->GetParticipantComponent()) return;
    const FString Id = Bike->GetParticipantComponent()->GetParticipantId().ToString();

    // VPR-24B: personality affects the top-end pace and chaos style, but all
    // rivals now use the same competent high-speed path controller.
    if (Id.Equals(TEXT("BOT_01"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("LEECH"); TargetSpeedKph = 145.0f; GrudgeDurationSeconds = 5.2f;
        GrudgeCatchupSpeedKph = 150.0f; AttackCooldownSeconds = 1.90f; AttackRange = 240.0f;
        PickupSeekRange = 1450.0f; AvoidanceStrength = 0.95f; EggUseCooldownSeconds = 5.2f; PeelUseCooldownSeconds = 5.8f;
    }
    else if (Id.Equals(TEXT("BOT_02"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("HOTHEAD"); TargetSpeedKph = 148.0f; GrudgeDurationSeconds = 4.0f;
        GrudgeCatchupSpeedKph = 152.0f; AttackCooldownSeconds = 1.45f; AttackRange = 245.0f;
        PickupSeekRange = 1200.0f; AvoidanceStrength = 0.90f; EggUseCooldownSeconds = 4.2f; PeelUseCooldownSeconds = 5.0f;
    }
    else if (Id.Equals(TEXT("BOT_03"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("PETTY"); TargetSpeedKph = 144.0f; GrudgeDurationSeconds = 4.6f;
        GrudgeCatchupSpeedKph = 149.0f; AttackCooldownSeconds = 2.10f; AttackRange = 235.0f;
        PickupSeekRange = 1650.0f; AvoidanceStrength = 1.02f; EggUseCooldownSeconds = 4.8f; PeelUseCooldownSeconds = 4.3f;
    }
    else if (Id.Equals(TEXT("BOT_04"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("GREMLIN"); TargetSpeedKph = 142.0f; GrudgeDurationSeconds = 4.2f;
        GrudgeCatchupSpeedKph = 148.0f; AttackCooldownSeconds = 1.85f; AttackRange = 235.0f;
        PickupSeekRange = 1700.0f; AvoidanceStrength = 1.00f; EggUseCooldownSeconds = 5.0f; PeelUseCooldownSeconds = 3.8f;
    }
    else if (Id.Equals(TEXT("BOT_05"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("BRAWLER"); TargetSpeedKph = 146.0f; GrudgeDurationSeconds = 3.8f;
        GrudgeCatchupSpeedKph = 151.0f; AttackCooldownSeconds = 1.35f; AttackRange = 250.0f;
        PickupSeekRange = 1200.0f; AvoidanceStrength = 0.92f; EggUseCooldownSeconds = 5.4f; PeelUseCooldownSeconds = 5.5f;
    }
    else if (Id.Equals(TEXT("BOT_06"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("TRYHARD"); TargetSpeedKph = 151.0f; GrudgeDurationSeconds = 3.0f;
        GrudgeCatchupSpeedKph = 153.0f; AttackCooldownSeconds = 2.20f; AttackRange = 230.0f;
        PickupSeekRange = 1450.0f; AvoidanceStrength = 1.08f; EggUseCooldownSeconds = 6.0f; PeelUseCooldownSeconds = 6.2f;
    }

    const uint32 StableHash = GetTypeHash(Id);
    SenseRefreshRemaining = static_cast<float>(StableHash % 5u) * 0.025f;
    ItemDecisionRemaining = static_cast<float>((StableHash >> 3u) % 5u) * 0.040f;
    TacticalSideSign = (StableHash & 1u) == 0u ? 1.0f : -1.0f;

    UE_LOG(LogTemp, Display, TEXT("RoadsideIdiots AI24B: %s role=%s straight=%.0f kph"),
        *Id, *PersonalityLabel, TargetSpeedKph);
}

bool ARIAIController::IsHoldingGrudgeAgainst(const ARIBikePawn* Target) const
{
    return Target && GrudgeTimeRemaining > 0.0f && GrudgeTarget.IsValid() && GrudgeTarget.Get() == Target;
}

bool ARIAIController::IsTacticalIntentActive() const
{
    return TacticalIntent != ERITacticalIntent::None && TacticalTimeRemaining > 0.0f && TacticalTarget.IsValid();
}

void ARIAIController::SetRoute(const TArray<FVector>& InRoutePoints, int32 StartTargetIndex, float InLaneOffset)
{
    RoutePoints = InRoutePoints;
    TargetIndex = RoutePoints.Num() > 0 ? FMath::Abs(StartTargetIndex) % RoutePoints.Num() : 0;
    LaneOffset = FMath::Clamp(InLaneOffset, -SafeRoadHalfWidth + 55.0f, SafeRoadHalfWidth - 55.0f);
    SmoothedLaneOffset = LaneOffset;
}

bool ARIAIController::AssignTacticalIntent(ARIBikePawn* Target, const ERITacticalIntent Intent, const float DurationSeconds)
{
    if (!Target || Target == Bike || Intent == ERITacticalIntent::None || TacticalCooldownRemaining > 0.0f) return false;
    TacticalTarget = Target;
    TacticalIntent = Intent;
    TacticalTimeRemaining = FMath::Clamp(DurationSeconds, 1.2f, 4.5f);
    bTacticalItemCommitted = false;

    if (Bike)
    {
        FVector ToTarget = Target->GetActorLocation() - Bike->GetActorLocation();
        ToTarget.Z = 0.0f;
        if (!ToTarget.IsNearlyZero())
        {
            TacticalSideSign = FVector::DotProduct(ToTarget.GetSafeNormal(), Bike->GetActorRightVector()) >= 0.0f ? 1.0f : -1.0f;
        }
    }
    return true;
}

void ARIAIController::EndTacticalIntent(const float CooldownSeconds)
{
    TacticalIntent = ERITacticalIntent::None;
    TacticalTarget.Reset();
    TacticalTimeRemaining = 0.0f;
    bTacticalItemCommitted = false;
    TacticalCooldownRemaining = FMath::Max(TacticalCooldownRemaining, CooldownSeconds > 0.0f ? CooldownSeconds : 7.0f);
}

void ARIAIController::NotifyProvokedBy(ARIBikePawn* InstigatorBike)
{
    if (!InstigatorBike || InstigatorBike == Bike) return;

    const bool bSameTargetAlready = GrudgeTarget.IsValid() && GrudgeTarget.Get() == InstigatorBike && GrudgeTimeRemaining > 0.0f;
    GrudgeTarget = InstigatorBike;
    GrudgeTimeRemaining = bSameTargetAlready
        ? FMath::Min(GrudgeTimeRemaining + 0.75f, GrudgeDurationSeconds * 1.10f)
        : GrudgeDurationSeconds;

    if (IsTacticalIntentActive() || TacticalCooldownRemaining > 0.0f) return;

    float RetaliationChance = 0.35f;
    if (PersonalityLabel.Equals(TEXT("HOTHEAD"), ESearchCase::IgnoreCase)) RetaliationChance = 0.72f;
    else if (PersonalityLabel.Equals(TEXT("BRAWLER"), ESearchCase::IgnoreCase)) RetaliationChance = 0.66f;
    else if (PersonalityLabel.Equals(TEXT("PETTY"), ESearchCase::IgnoreCase)) RetaliationChance = 0.46f;
    else if (PersonalityLabel.Equals(TEXT("GREMLIN"), ESearchCase::IgnoreCase)) RetaliationChance = 0.42f;
    else if (PersonalityLabel.Equals(TEXT("LEECH"), ESearchCase::IgnoreCase)) RetaliationChance = 0.30f;
    else if (PersonalityLabel.Equals(TEXT("TRYHARD"), ESearchCase::IgnoreCase)) RetaliationChance = 0.14f;

    if (FMath::FRand() > RetaliationChance) return;

    AttackCooldownRemaining = 0.25f;
    TacticalTarget = InstigatorBike;
    TacticalIntent = ERITacticalIntent::SidePressure;
    TacticalTimeRemaining = FMath::Min(2.8f, GrudgeDurationSeconds);
    bTacticalItemCommitted = false;
}

ARIBikePawn* ARIAIController::FindBestItemVictim() const
{
    if (!Bike || !GetWorld()) return nullptr;
    if (IsTacticalIntentActive()) return TacticalTarget.Get();
    if (GrudgeTimeRemaining > 0.0f && GrudgeTarget.IsValid()) return GrudgeTarget.Get();

    ARIBikePawn* Nearest = nullptr;
    float NearestDistanceSq = TNumericLimits<float>::Max();
    for (TActorIterator<ARIBikePawn> It(GetWorld()); It; ++It)
    {
        ARIBikePawn* Candidate = *It;
        if (!Candidate || Candidate == Bike || !Candidate->AreRaceControlsEnabled()) continue;
        const float DistanceSq = FVector::DistSquared2D(Bike->GetActorLocation(), Candidate->GetActorLocation());
        if (DistanceSq < NearestDistanceSq)
        {
            NearestDistanceSq = DistanceSq;
            Nearest = Candidate;
        }
    }
    return Nearest;
}

bool ARIAIController::FindUsefulPickupTarget(FVector& OutTarget) const
{
    OutTarget = FVector::ZeroVector;
    if (!Bike || !GetWorld()) return false;

    const FVector BikeLocation = Bike->GetActorLocation();
    const FVector Forward = Bike->GetActorForwardVector().GetSafeNormal2D();
    const float MaxDistanceSq = FMath::Square(PickupSeekRange);
    float BestScore = TNumericLimits<float>::Max();
    bool bFound = false;

    const float HealthFraction = Bike->GetHealthComponent()
        ? Bike->GetHealthComponent()->GetCurrentHealth() / FMath::Max(1.0f, Bike->GetHealthComponent()->GetMaxHealth())
        : 1.0f;

    auto ConsiderPickup = [&](const AActor* Pickup, const bool bUseful)
    {
        if (!Pickup || !bUseful) return;
        FVector ToPickup = Pickup->GetActorLocation() - BikeLocation;
        ToPickup.Z = 0.0f;
        const float DistanceSq = ToPickup.SizeSquared();
        if (DistanceSq > MaxDistanceSq || DistanceSq < FMath::Square(140.0f)) return;
        const FVector Direction = ToPickup.GetSafeNormal();
        const float ForwardDot = FVector::DotProduct(Direction, Forward);
        if (ForwardDot < 0.28f) return;
        const float Score = DistanceSq * FMath::Lerp(1.10f, 0.86f, FMath::Clamp(ForwardDot, 0.0f, 1.0f));
        if (Score < BestScore)
        {
            BestScore = Score;
            OutTarget = Pickup->GetActorLocation();
            bFound = true;
        }
    };

    for (TActorIterator<ARIBananaPickup> It(GetWorld()); It; ++It)
        ConsiderPickup(*It, HealthFraction < 0.82f || Bike->GetBananaPeelCount() < 1);
    for (TActorIterator<ARIRottenEggPickup> It(GetWorld()); It; ++It)
        ConsiderPickup(*It, Bike->GetRottenEggCount() < Bike->GetMaxRottenEggs());
    return bFound;
}

bool ARIAIController::ProjectOntoRoute(
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
            FVector LateralDelta = WorldLocation - Projection;
            LateralDelta.Z = 0.0f;
            OutLateralOffset = FVector::DotProduct(LateralDelta, OutRight);
            OutSegmentIndex = Index;
            OutSegmentAlpha = Alpha;
            bFound = true;
        }
    }

    return bFound;
}

FVector ARIAIController::SampleRouteAhead(
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
    FVector Segment = B - A;
    Segment.Z = 0.0f;
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

float ARIAIController::ComputePreviewCurvature(const int32 SegmentIndex, const float SegmentAlpha, const float PreviewDistanceCm) const
{
    if (RoutePoints.Num() < 3) return 0.0f;

    const float D = FMath::Max(900.0f, PreviewDistanceCm);
    const FVector P0 = SampleRouteAhead(SegmentIndex, SegmentAlpha, 0.0f, 0.0f, nullptr);
    const FVector P1 = SampleRouteAhead(SegmentIndex, SegmentAlpha, D * 0.33f, 0.0f, nullptr);
    const FVector P2 = SampleRouteAhead(SegmentIndex, SegmentAlpha, D * 0.66f, 0.0f, nullptr);
    const FVector P3 = SampleRouteAhead(SegmentIndex, SegmentAlpha, D, 0.0f, nullptr);

    auto CurvatureFromThree = [](const FVector& A, const FVector& B, const FVector& C)
    {
        const float AB = FVector::Dist2D(A, B);
        const float BC = FVector::Dist2D(B, C);
        const float AC = FVector::Dist2D(A, C);
        const float Denom = AB * BC * AC;
        if (Denom <= KINDA_SMALL_NUMBER) return 0.0f;

        FVector BA = B - A;
        FVector CA = C - A;
        BA.Z = 0.0f;
        CA.Z = 0.0f;
        const float DoubleArea = FMath::Abs(FVector::CrossProduct(BA, CA).Z);
        return (2.0f * DoubleArea) / Denom;
    };

    return FMath::Max(CurvatureFromThree(P0, P1, P2), CurvatureFromThree(P1, P2, P3));
}

float ARIAIController::ComputeAvoidanceShift(
    const FVector& BikeLocation,
    const FVector& PathForward,
    const FVector& RouteRight,
    const float CurrentLateralOffset) const
{
    if (!Bike || !GetWorld()) return 0.0f;

    const FVector SelfVelocity = Bike->GetChassis()
        ? Bike->GetChassis()->GetPhysicsLinearVelocity()
        : FVector::ZeroVector;

    float BestThreat = 0.0f;
    float BestTargetLane = LaneOffset;

    auto ConsiderObstacle = [&](const AActor* Obstacle, const FVector& ObstacleVelocity,
        const float LookAhead, const float SideClearance, const float MaxShift, const float Weight)
    {
        if (!Obstacle || Obstacle == Bike) return;

        FVector ToObstacle = Obstacle->GetActorLocation() - BikeLocation;
        ToObstacle.Z = 0.0f;
        const float Along = FVector::DotProduct(ToObstacle, PathForward);
        if (Along < 100.0f || Along > LookAhead) return;

        const float Side = FVector::DotProduct(ToObstacle, RouteRight);
        if (FMath::Abs(Side) > SideClearance) return;

        const float Distance = ToObstacle.Size();
        if (Distance < 1.0f) return;
        const FVector Direction = ToObstacle / Distance;
        const float ClosingSpeed = FMath::Max(0.0f, FVector::DotProduct(SelfVelocity - ObstacleVelocity, Direction));
        const float TimeToCollision = Along / FMath::Max(ClosingSpeed, 650.0f);
        const float TTCUrgency = 1.0f - FMath::Clamp(TimeToCollision / 1.35f, 0.0f, 1.0f);
        const float DistanceUrgency = 1.0f - FMath::Clamp(Along / LookAhead, 0.0f, 1.0f);
        const float Threat = FMath::Clamp(FMath::Max(TTCUrgency, DistanceUrgency * 0.55f) * Weight, 0.0f, 1.0f);
        if (Threat <= BestThreat) return;

        const float LeftSpace = FMath::Max(0.0f, SafeRoadHalfWidth + CurrentLateralOffset - 55.0f);
        const float RightSpace = FMath::Max(0.0f, SafeRoadHalfWidth - CurrentLateralOffset - 55.0f);

        float DodgeSign = 0.0f;
        if (FMath::Abs(Side) < 35.0f)
        {
            DodgeSign = RightSpace >= LeftSpace ? 1.0f : -1.0f;
        }
        else
        {
            DodgeSign = Side > 0.0f ? -1.0f : 1.0f;
        }

        const float PreferredSpace = DodgeSign > 0.0f ? RightSpace : LeftSpace;
        const float OppositeSpace = DodgeSign > 0.0f ? LeftSpace : RightSpace;
        const float DesiredShiftMagnitude = MaxShift * Threat * AvoidanceStrength;
        const float RequiredSpace = DesiredShiftMagnitude + 105.0f;

        // Never obey "dodge away" blindly if that direction is the wall.
        if (PreferredSpace < RequiredSpace && OppositeSpace > PreferredSpace + 70.0f)
        {
            DodgeSign *= -1.0f;
        }

        const float TargetLane = FMath::Clamp(
            LaneOffset + DodgeSign * DesiredShiftMagnitude,
            -SafeRoadHalfWidth + 60.0f,
            SafeRoadHalfWidth - 60.0f);

        BestThreat = Threat;
        BestTargetLane = TargetLane;
    };

    for (TActorIterator<ARITrafficVehicle> It(GetWorld()); It; ++It)
    {
        ARITrafficVehicle* Traffic = *It;
        ConsiderObstacle(
            Traffic,
            Traffic ? Traffic->GetTrafficVelocityEstimate() : FVector::ZeroVector,
            3600.0f, 430.0f, 310.0f, 1.10f);
    }

    for (TActorIterator<ARIPoopHazard> It(GetWorld()); It; ++It)
        ConsiderObstacle(*It, FVector::ZeroVector, 3000.0f, 285.0f, 275.0f, 1.20f);

    for (TActorIterator<ARIBananaPeelHazard> It(GetWorld()); It; ++It)
        ConsiderObstacle(*It, FVector::ZeroVector, 3000.0f, 270.0f, 270.0f, 1.25f);

    for (TActorIterator<ARIBikePawn> It(GetWorld()); It; ++It)
    {
        ARIBikePawn* OtherBike = *It;
        if (!OtherBike || OtherBike == Bike) continue;
        const FVector OtherVelocity = OtherBike->GetChassis()
            ? OtherBike->GetChassis()->GetPhysicsLinearVelocity()
            : FVector::ZeroVector;
        const bool bIntentTarget = IsTacticalIntentActive() && TacticalTarget.Get() == OtherBike;
        const float Weight = bIntentTarget && TacticalIntent == ERITacticalIntent::SidePressure ? 0.48f : 0.90f;
        ConsiderObstacle(OtherBike, OtherVelocity, 2900.0f, 310.0f, 245.0f, Weight);
    }

    return BestTargetLane - LaneOffset;
}

float ARIAIController::ComputeCrowdSpeedScale(
    const FVector& BikeLocation,
    const FVector& PathForward,
    const FVector& RouteRight) const
{
    if (!Bike || !GetWorld()) return 1.0f;

    float SpeedScale = 1.0f;
    const FVector SelfVelocity = Bike->GetChassis()
        ? Bike->GetChassis()->GetPhysicsLinearVelocity()
        : FVector::ZeroVector;

    auto ConsiderEmergency = [&](const AActor* Other, const FVector& OtherVelocity, const float LookAhead, const float SideClearance)
    {
        if (!Other || Other == Bike) return;

        FVector ToOther = Other->GetActorLocation() - BikeLocation;
        ToOther.Z = 0.0f;
        const float Along = FVector::DotProduct(ToOther, PathForward);
        if (Along < 80.0f || Along > LookAhead) return;

        const float Side = FMath::Abs(FVector::DotProduct(ToOther, RouteRight));
        if (Side > SideClearance) return;

        const float Distance = ToOther.Size();
        if (Distance < 1.0f) return;
        const FVector Direction = ToOther / Distance;
        const float ClosingSpeed = FMath::Max(0.0f, FVector::DotProduct(SelfVelocity - OtherVelocity, Direction));
        if (ClosingSpeed < 120.0f && Along > 350.0f) return;

        const float TTC = Along / FMath::Max(ClosingSpeed, 550.0f);
        float LocalScale = 1.0f;
        if (TTC < 0.30f) LocalScale = 0.45f;
        else if (TTC < 0.52f) LocalScale = 0.64f;
        else if (TTC < 0.78f) LocalScale = 0.80f;
        else if (TTC < 1.02f) LocalScale = 0.91f;
        if (Along < 300.0f) LocalScale = FMath::Min(LocalScale, 0.55f);
        SpeedScale = FMath::Min(SpeedScale, LocalScale);
    };

    for (TActorIterator<ARITrafficVehicle> It(GetWorld()); It; ++It)
    {
        ARITrafficVehicle* Traffic = *It;
        if (Traffic) ConsiderEmergency(Traffic, Traffic->GetTrafficVelocityEstimate(), 2800.0f, 190.0f);
    }

    for (TActorIterator<ARIBikePawn> It(GetWorld()); It; ++It)
    {
        ARIBikePawn* OtherBike = *It;
        if (!OtherBike || OtherBike == Bike) continue;
        const FVector OtherVelocity = OtherBike->GetChassis()
            ? OtherBike->GetChassis()->GetPhysicsLinearVelocity()
            : FVector::ZeroVector;
        ConsiderEmergency(OtherBike, OtherVelocity, 2100.0f, 170.0f);
    }

    for (TActorIterator<ARIPoopHazard> It(GetWorld()); It; ++It)
        ConsiderEmergency(*It, FVector::ZeroVector, 1550.0f, 125.0f);

    for (TActorIterator<ARIBananaPeelHazard> It(GetWorld()); It; ++It)
        ConsiderEmergency(*It, FVector::ZeroVector, 1550.0f, 120.0f);

    return SpeedScale;
}

float ARIAIController::ComputeWallTraceSteer(
    const FVector& BikeLocation,
    const FVector& Forward,
    float& OutWallSpeedScale) const
{
    OutWallSpeedScale = 1.0f;
    if (!Bike || !GetWorld()) return 0.0f;

    const FVector TraceStart = BikeLocation + FVector::UpVector * 72.0f;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(RIAIWallFeelers), false, Bike);

    auto TraceDistance = [&](const FVector& Direction)
    {
        FHitResult Hit;
        const FVector End = TraceStart + Direction.GetSafeNormal2D() * WallTraceLength;
        if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, End, ECC_Visibility, Params) && Hit.bBlockingHit)
        {
            return Hit.Distance;
        }
        return WallTraceLength;
    };

    const FVector LeftDirection = Forward.RotateAngleAxis(-22.0f, FVector::UpVector).GetSafeNormal2D();
    const FVector RightDirection = Forward.RotateAngleAxis(22.0f, FVector::UpVector).GetSafeNormal2D();
    const float CenterDistance = TraceDistance(Forward);
    const float LeftDistance = TraceDistance(LeftDirection);
    const float RightDistance = TraceDistance(RightDirection);

    float Steer = 0.0f;

    if (CenterDistance < WallTraceLength * 0.82f)
    {
        const float Strength = 1.0f - FMath::Clamp(
            (CenterDistance - 380.0f) / FMath::Max(1.0f, WallTraceLength * 0.82f - 380.0f),
            0.0f, 1.0f);
        Steer = (RightDistance > LeftDistance ? 1.0f : -1.0f) * Strength;

        if (CenterDistance < 950.0f)
        {
            OutWallSpeedScale = FMath::GetMappedRangeValueClamped(
                FVector2D(300.0f, 950.0f), FVector2D(0.56f, 0.90f), CenterDistance);
        }
        else
        {
            OutWallSpeedScale = 0.94f;
        }
    }

    if (LeftDistance < 760.0f && RightDistance > LeftDistance + 130.0f)
    {
        const float SideStrength = (1.0f - LeftDistance / 760.0f) * 0.42f;
        Steer = FMath::Clamp(Steer + SideStrength, -1.0f, 1.0f);
    }
    if (RightDistance < 760.0f && LeftDistance > RightDistance + 130.0f)
    {
        const float SideStrength = (1.0f - RightDistance / 760.0f) * 0.42f;
        Steer = FMath::Clamp(Steer - SideStrength, -1.0f, 1.0f);
    }

    return Steer;
}

void ARIAIController::ApplyTacticalLanePlan(
    float& InOutDesiredLaneOffset,
    const FVector& BikeLocation,
    const float TurnSeverity)
{
    if (!IsTacticalIntentActive()) return;
    ARIBikePawn* Target = TacticalTarget.Get();
    if (!Target) { EndTacticalIntent(); return; }

    FVector TargetProjection;
    FVector TargetTangent;
    FVector TargetRouteRight;
    float TargetLateral = 0.0f;
    int32 TargetSegment = 0;
    float TargetAlpha = 0.0f;
    if (!ProjectOntoRoute(Target->GetActorLocation(), TargetProjection, TargetTangent, TargetRouteRight,
        TargetLateral, TargetSegment, TargetAlpha))
    {
        return;
    }

    FVector ToTarget = Target->GetActorLocation() - BikeLocation;
    ToTarget.Z = 0.0f;
    const float Distance = ToTarget.Size();

    // Hard corners belong to the racing controller. The chaos layer may fire an
    // egg, but it is not allowed to drag the bike off-line while cornering.
    if (TurnSeverity > 0.38f)
    {
        if (TacticalIntent == ERITacticalIntent::EggShot && !bTacticalItemCommitted &&
            Bike->GetRottenEggCount() > 0 && EggUseCooldownRemaining <= 0.0f && Distance <= 1000.0f)
        {
            if (Bike->ThrowRottenEggAt(Target))
            {
                bTacticalItemCommitted = true;
                EggUseCooldownRemaining = EggUseCooldownSeconds;
                EndTacticalIntent(9.0f);
            }
        }
        return;
    }

    const float LaneLimit = SafeRoadHalfWidth - 70.0f;

    switch (TacticalIntent)
    {
    case ERITacticalIntent::SidePressure:
    {
        const float TacticalLane = FMath::Clamp(TargetLateral + TacticalSideSign * 105.0f, -LaneLimit, LaneLimit);
        InOutDesiredLaneOffset = FMath::Lerp(InOutDesiredLaneOffset, TacticalLane, 0.48f);

        if (Distance < AttackRange && AttackCooldownRemaining <= 0.0f)
        {
            if (URIInteractionComponent* Interaction = Bike->GetInteractionComponent())
            {
                const float Side = FVector::DotProduct(ToTarget.GetSafeNormal(), Bike->GetActorRightVector());
                const bool bConnected = Interaction->TrySideInteraction(Side < 0.0f ? -1.0f : 1.0f);
                AttackCooldownRemaining = bConnected ? AttackCooldownSeconds : 0.45f;
                if (bConnected) EndTacticalIntent(8.0f);
            }
        }
        break;
    }

    case ERITacticalIntent::Block:
    {
        const float TacticalLane = FMath::Clamp(TargetLateral + TacticalSideSign * 70.0f, -LaneLimit, LaneLimit);
        InOutDesiredLaneOffset = FMath::Lerp(InOutDesiredLaneOffset, TacticalLane, 0.34f);
        break;
    }

    case ERITacticalIntent::PeelTrap:
    {
        const float TacticalLane = FMath::Clamp(TargetLateral + TacticalSideSign * 35.0f, -LaneLimit, LaneLimit);
        InOutDesiredLaneOffset = FMath::Lerp(InOutDesiredLaneOffset, TacticalLane, 0.38f);

        const FVector TargetForward = Target->GetActorForwardVector().GetSafeNormal2D();
        const FVector TargetRight = Target->GetActorRightVector().GetSafeNormal2D();
        FVector TargetToSelf = BikeLocation - Target->GetActorLocation();
        TargetToSelf.Z = 0.0f;
        const float AheadDistance = FVector::DotProduct(TargetToSelf, TargetForward);
        const float SideDistance = FMath::Abs(FVector::DotProduct(TargetToSelf, TargetRight));
        if (!bTacticalItemCommitted && Bike->GetBananaPeelCount() > 0 && PeelUseCooldownRemaining <= 0.0f &&
            AheadDistance > 250.0f && AheadDistance < 900.0f && SideDistance < 250.0f)
        {
            if (Bike->DropBananaPeel())
            {
                bTacticalItemCommitted = true;
                PeelUseCooldownRemaining = PeelUseCooldownSeconds;
                EndTacticalIntent(9.0f);
            }
        }
        break;
    }

    case ERITacticalIntent::EggShot:
    {
        const float TacticalLane = FMath::Clamp(TargetLateral, -LaneLimit, LaneLimit);
        InOutDesiredLaneOffset = FMath::Lerp(InOutDesiredLaneOffset, TacticalLane, 0.12f);
        if (!bTacticalItemCommitted && Bike->GetRottenEggCount() > 0 && EggUseCooldownRemaining <= 0.0f && Distance <= 1000.0f)
        {
            if (Bike->ThrowRottenEggAt(Target))
            {
                bTacticalItemCommitted = true;
                EggUseCooldownRemaining = EggUseCooldownSeconds;
                EndTacticalIntent(9.0f);
            }
        }
        break;
    }

    default:
        break;
    }
}

void ARIAIController::TryUseComedyItems()
{
    if (!Bike || IsTacticalIntentActive()) return;
    ARIBikePawn* Victim = FindBestItemVictim();
    if (!Victim) return;

    FVector ToVictim = Victim->GetActorLocation() - Bike->GetActorLocation();
    ToVictim.Z = 0.0f;
    const float Distance = ToVictim.Size();
    if (Distance < 1.0f) return;
    const FVector Direction = ToVictim / Distance;
    const float ForwardDot = FVector::DotProduct(Direction, Bike->GetActorForwardVector().GetSafeNormal2D());

    if (Bike->GetRottenEggCount() > 0 && EggUseCooldownRemaining <= 0.0f &&
        Distance < 1000.0f && ForwardDot > 0.10f && FMath::FRand() < 0.10f)
    {
        if (Bike->ThrowRottenEggAt(Victim))
        {
            EggUseCooldownRemaining = EggUseCooldownSeconds;
            return;
        }
    }

    if (Bike->GetBananaPeelCount() > 0 && PeelUseCooldownRemaining <= 0.0f &&
        Distance > 250.0f && Distance < 850.0f && ForwardDot < -0.15f && FMath::FRand() < 0.12f)
    {
        if (Bike->DropBananaPeel()) PeelUseCooldownRemaining = PeelUseCooldownSeconds;
    }
}

void ARIAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Bike || RoutePoints.Num() < 3 || !Bike->GetBikeMovement()) return;

    if (!Bike->AreRaceControlsEnabled())
    {
        LowMotionTime = 0.0f;
        SmoothedSteering = 0.0f;
        SmoothedThrottle = 0.0f;
        SmoothedBrake = 0.0f;
        Bike->SetControlInputs(0.0f, 0.0f, 0.0f);
        return;
    }

    AttackCooldownRemaining = FMath::Max(0.0f, AttackCooldownRemaining - DeltaSeconds);
    EggUseCooldownRemaining = FMath::Max(0.0f, EggUseCooldownRemaining - DeltaSeconds);
    PeelUseCooldownRemaining = FMath::Max(0.0f, PeelUseCooldownRemaining - DeltaSeconds);
    SenseRefreshRemaining = FMath::Max(0.0f, SenseRefreshRemaining - DeltaSeconds);
    ItemDecisionRemaining = FMath::Max(0.0f, ItemDecisionRemaining - DeltaSeconds);
    TacticalCooldownRemaining = FMath::Max(0.0f, TacticalCooldownRemaining - DeltaSeconds);

    if (GrudgeTimeRemaining > 0.0f) GrudgeTimeRemaining = FMath::Max(0.0f, GrudgeTimeRemaining - DeltaSeconds);
    else GrudgeTarget.Reset();

    if (TacticalTimeRemaining > 0.0f)
    {
        TacticalTimeRemaining = FMath::Max(0.0f, TacticalTimeRemaining - DeltaSeconds);
        if (TacticalTimeRemaining <= 0.0f) EndTacticalIntent(7.0f);
    }

    const FVector BikeLocation = Bike->GetActorLocation();
    const FVector Forward = Bike->GetActorForwardVector().GetSafeNormal2D();
    const float SpeedKph = FMath::Abs(Bike->GetBikeMovement()->GetForwardSpeedKph());
    const float SpeedCms = SpeedKph / 0.036f;

    FVector RouteProjection;
    FVector RouteTangent;
    FVector RouteRight;
    float CurrentLateralOffset = 0.0f;
    int32 RouteSegment = 0;
    float RouteAlpha = 0.0f;
    if (!ProjectOntoRoute(BikeLocation, RouteProjection, RouteTangent, RouteRight,
        CurrentLateralOffset, RouteSegment, RouteAlpha))
    {
        Bike->SetControlInputs(0.0f, 0.0f, 0.6f);
        return;
    }

    TargetIndex = (RouteSegment + 1) % RoutePoints.Num();

    const float SpeedAlpha = FMath::Clamp(SpeedKph / 150.0f, 0.0f, 1.0f);
    const float CurvePreviewDistance = FMath::Lerp(CurvePreviewMinDistance, CurvePreviewMaxDistance, SpeedAlpha);
    const float PreviewCurvature = ComputePreviewCurvature(RouteSegment, RouteAlpha, CurvePreviewDistance);
    const float TurnSeverity = FMath::Clamp(PreviewCurvature / 0.00035f, 0.0f, 1.0f);

    if (SenseRefreshRemaining <= 0.0f)
    {
        SenseRefreshRemaining = FMath::Max(0.08f, SenseRefreshIntervalSeconds);
        bHasCachedPickupTarget = !IsTacticalIntentActive() && GrudgeTimeRemaining <= 0.0f && FindUsefulPickupTarget(CachedPickupTarget);
        CachedAvoidanceShift = ComputeAvoidanceShift(BikeLocation, RouteTangent, RouteRight, CurrentLateralOffset);
        CachedCrowdSpeedScale = ComputeCrowdSpeedScale(BikeLocation, RouteTangent, RouteRight);
    }

    // Obstacles request a lane, never a completely unrelated world-space point.
    // This keeps avoidance subordinate to the racing line.
    const float CurveLaneChangeScale = FMath::Lerp(1.0f, 0.72f, TurnSeverity);
    float DesiredLaneOffset = LaneOffset + CachedAvoidanceShift * CurveLaneChangeScale;

    if (!IsTacticalIntentActive() && GrudgeTimeRemaining <= 0.0f && bHasCachedPickupTarget && TurnSeverity < 0.42f)
    {
        FVector PickupProjection;
        FVector PickupTangent;
        FVector PickupRight;
        float PickupLateral = 0.0f;
        int32 PickupSegment = 0;
        float PickupAlpha = 0.0f;
        if (ProjectOntoRoute(CachedPickupTarget, PickupProjection, PickupTangent, PickupRight,
            PickupLateral, PickupSegment, PickupAlpha))
        {
            DesiredLaneOffset = FMath::Lerp(DesiredLaneOffset, PickupLateral, 0.42f);
        }
    }

    ApplyTacticalLanePlan(DesiredLaneOffset, BikeLocation, TurnSeverity);

    const float BoundaryRisk = FMath::GetMappedRangeValueClamped(
        FVector2D(SafeRoadHalfWidth * 0.76f, SafeRoadHalfWidth),
        FVector2D(0.0f, 1.0f),
        FMath::Abs(CurrentLateralOffset));

    if (BoundaryRisk > 0.0f)
    {
        // Near a barrier, winning the lane argument is less important than
        // returning to usable asphalt immediately.
        DesiredLaneOffset = FMath::Lerp(DesiredLaneOffset, 0.0f, BoundaryRisk * 0.88f);
    }

    DesiredLaneOffset = FMath::Clamp(
        DesiredLaneOffset,
        -SafeRoadHalfWidth + 60.0f,
        SafeRoadHalfWidth - 60.0f);

    SmoothedLaneOffset = FMath::FInterpTo(
        SmoothedLaneOffset,
        DesiredLaneOffset,
        DeltaSeconds,
        LaneChangeInterpSpeed);

    const FVector CurrentPathPoint = RouteProjection + RouteRight * SmoothedLaneOffset;
    const FVector HeadingPoint = SampleRouteAhead(RouteSegment, RouteAlpha, 720.0f, SmoothedLaneOffset, nullptr);
    FVector DesiredHeading = HeadingPoint - CurrentPathPoint;
    DesiredHeading.Z = 0.0f;
    DesiredHeading = DesiredHeading.GetSafeNormal();
    if (DesiredHeading.IsNearlyZero()) DesiredHeading = RouteTangent;

    const float LookAheadDistance = FMath::Lerp(MinLookAheadDistance, MaxLookAheadDistance, SpeedAlpha);
    FVector PreviewTangent;
    const FVector PreviewPoint = SampleRouteAhead(
        RouteSegment,
        RouteAlpha,
        LookAheadDistance,
        SmoothedLaneOffset,
        &PreviewTangent);

    const float HeadingDot = FMath::Clamp(FVector::DotProduct(Forward, DesiredHeading), -1.0f, 1.0f);
    const float HeadingError = FMath::Atan2(FVector::CrossProduct(Forward, DesiredHeading).Z, HeadingDot);
    const float LateralError = SmoothedLaneOffset - CurrentLateralOffset;
    const float CrossTrackCorrection = FMath::Atan2(
        CrossTrackGain * LateralError,
        FMath::Max(250.0f, SpeedCms) + StanleySofteningSpeedCmS);

    const float FutureDot = FMath::Clamp(FVector::DotProduct(DesiredHeading, PreviewTangent), -1.0f, 1.0f);
    const float FutureHeadingDelta = FMath::Atan2(FVector::CrossProduct(DesiredHeading, PreviewTangent).Z, FutureDot);
    const float SignedPreviewCurvature = FutureHeadingDelta / FMath::Max(600.0f, LookAheadDistance - 720.0f);
    const float CurvatureFeedForward = FMath::Atan(SignedPreviewCurvature * CurvatureFeedForwardDistance);

    float SteeringEquivalent = HeadingGain * HeadingError + CrossTrackCorrection + CurvatureFeedForward * 0.72f;
    float RawSteering = FMath::Clamp(SteeringEquivalent / SteeringCommandRadians, -1.0f, 1.0f);

    // Corridor guardian: if physics has already pushed the bike toward an edge,
    // add an immediate inward command independent of tactical/avoidance intent.
    if (BoundaryRisk > 0.0f)
    {
        const float InwardSign = CurrentLateralOffset > 0.0f ? -1.0f : 1.0f;
        RawSteering = FMath::Clamp(RawSteering + InwardSign * BoundaryRisk * 0.62f, -1.0f, 1.0f);
    }

    float WallSpeedScale = 1.0f;
    const float WallSteer = ComputeWallTraceSteer(BikeLocation, Forward, WallSpeedScale);
    RawSteering = FMath::Clamp(RawSteering + WallSteer * 0.82f, -1.0f, 1.0f);

    SmoothedSteering = FMath::FInterpTo(
        SmoothedSteering,
        RawSteering,
        DeltaSeconds,
        SteeringInterpSpeed);

    if (ItemDecisionRemaining <= 0.0f)
    {
        ItemDecisionRemaining = FMath::Max(0.14f, ItemDecisionIntervalSeconds);
        TryUseComedyItems();
    }

    FVector ToPreview = PreviewPoint - BikeLocation;
    ToPreview.Z = 0.0f;
    if (SpeedKph < 6.0f && ToPreview.SizeSquared() > FMath::Square(320.0f)) LowMotionTime += DeltaSeconds;
    else LowMotionTime = 0.0f;

    if (LowMotionTime > 1.15f && LowMotionTime < 2.35f)
    {
        const float ReverseSteer = FMath::Abs(SmoothedSteering) > 0.12f
            ? -SmoothedSteering
            : (CurrentLateralOffset >= 0.0f ? -0.62f : 0.62f);
        Bike->SetControlInputs(-0.52f, ReverseSteer, 0.0f);
        return;
    }

    if (LowMotionTime >= 2.35f)
    {
        Bike->RecoverBike();
        LowMotionTime = 0.0f;
        EndTacticalIntent(6.0f);
        return;
    }

    float RaceTopSpeed = TargetSpeedKph;
    if (GrudgeTimeRemaining > 0.0f || IsTacticalIntentActive())
    {
        RaceTopSpeed = FMath::Min(GrudgeCatchupSpeedKph, TargetSpeedKph + 5.0f);
    }

    float CurveSpeedLimit = RaceTopSpeed;
    if (PreviewCurvature > 0.000001f)
    {
        const float CurveSpeedCms = FMath::Sqrt(MaxLateralAccelCmS2 / PreviewCurvature);
        CurveSpeedLimit = FMath::Clamp(CurveSpeedCms * 0.036f * 0.98f, MinimumCornerSpeedKph, RaceTopSpeed);
    }

    float DesiredSpeed = FMath::Min(RaceTopSpeed, CurveSpeedLimit);
    DesiredSpeed *= FMath::Clamp(CachedCrowdSpeedScale, 0.42f, 1.0f);
    DesiredSpeed *= FMath::Clamp(WallSpeedScale, 0.55f, 1.0f);

    if (BoundaryRisk > 0.20f)
    {
        DesiredSpeed = FMath::Min(DesiredSpeed, FMath::Lerp(105.0f, 62.0f, BoundaryRisk));
    }

    const float AbsHeadingError = FMath::Abs(HeadingError);
    if (AbsHeadingError > 0.82f) DesiredSpeed = FMath::Min(DesiredSpeed, 58.0f);
    else if (AbsHeadingError > 0.52f) DesiredSpeed = FMath::Min(DesiredSpeed, 82.0f);

    const float SpeedError = DesiredSpeed - SpeedKph;
    float DesiredThrottle = 0.0f;
    float DesiredBrake = 0.0f;

    if (SpeedError >= 0.0f)
    {
        DesiredThrottle = FMath::Clamp(0.52f + SpeedError / 18.0f, 0.52f, 1.0f);
    }
    else
    {
        DesiredBrake = FMath::Clamp((-SpeedError) / 28.0f, 0.12f, 0.92f);
        DesiredThrottle = 0.0f;
    }

    SmoothedThrottle = FMath::FInterpTo(SmoothedThrottle, DesiredThrottle, DeltaSeconds, 5.5f);
    SmoothedBrake = FMath::FInterpTo(SmoothedBrake, DesiredBrake, DeltaSeconds, 9.0f);

    float FinalThrottle = SmoothedThrottle;
    if (SmoothedBrake > 0.14f)
    {
        FinalThrottle = FMath::Min(FinalThrottle, 0.10f);
    }

    Bike->SetControlInputs(FinalThrottle, SmoothedSteering, SmoothedBrake);
}
