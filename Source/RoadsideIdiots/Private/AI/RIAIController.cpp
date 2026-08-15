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

    if (Id.Equals(TEXT("BOT_01"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("LEECH"); TargetSpeedKph = 111.0f; GrudgeDurationSeconds = 5.2f;
        GrudgeCatchupSpeedKph = 120.0f; AttackCooldownSeconds = 1.90f; AttackRange = 240.0f;
        PickupSeekRange = 1450.0f; AvoidanceStrength = 0.95f; EggUseCooldownSeconds = 5.2f; PeelUseCooldownSeconds = 5.8f;
    }
    else if (Id.Equals(TEXT("BOT_02"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("HOTHEAD"); TargetSpeedKph = 114.0f; GrudgeDurationSeconds = 4.0f;
        GrudgeCatchupSpeedKph = 123.0f; AttackCooldownSeconds = 1.45f; AttackRange = 245.0f;
        PickupSeekRange = 1200.0f; AvoidanceStrength = 0.88f; EggUseCooldownSeconds = 4.2f; PeelUseCooldownSeconds = 5.0f;
    }
    else if (Id.Equals(TEXT("BOT_03"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("PETTY"); TargetSpeedKph = 109.0f; GrudgeDurationSeconds = 4.6f;
        GrudgeCatchupSpeedKph = 118.0f; AttackCooldownSeconds = 2.10f; AttackRange = 235.0f;
        PickupSeekRange = 1650.0f; AvoidanceStrength = 1.02f; EggUseCooldownSeconds = 4.8f; PeelUseCooldownSeconds = 4.3f;
    }
    else if (Id.Equals(TEXT("BOT_04"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("GREMLIN"); TargetSpeedKph = 108.0f; GrudgeDurationSeconds = 4.2f;
        GrudgeCatchupSpeedKph = 119.0f; AttackCooldownSeconds = 1.85f; AttackRange = 235.0f;
        PickupSeekRange = 1700.0f; AvoidanceStrength = 1.00f; EggUseCooldownSeconds = 5.0f; PeelUseCooldownSeconds = 3.8f;
    }
    else if (Id.Equals(TEXT("BOT_05"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("BRAWLER"); TargetSpeedKph = 112.0f; GrudgeDurationSeconds = 3.8f;
        GrudgeCatchupSpeedKph = 121.0f; AttackCooldownSeconds = 1.35f; AttackRange = 250.0f;
        PickupSeekRange = 1200.0f; AvoidanceStrength = 0.90f; EggUseCooldownSeconds = 5.4f; PeelUseCooldownSeconds = 5.5f;
    }
    else if (Id.Equals(TEXT("BOT_06"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("TRYHARD"); TargetSpeedKph = 116.0f; GrudgeDurationSeconds = 3.0f;
        GrudgeCatchupSpeedKph = 122.0f; AttackCooldownSeconds = 2.20f; AttackRange = 230.0f;
        PickupSeekRange = 1450.0f; AvoidanceStrength = 1.10f; EggUseCooldownSeconds = 6.0f; PeelUseCooldownSeconds = 6.2f;
    }

    const uint32 StableHash = GetTypeHash(Id);
    SenseRefreshRemaining = static_cast<float>(StableHash % 5u) * 0.025f;
    ItemDecisionRemaining = static_cast<float>((StableHash >> 3u) % 5u) * 0.040f;
    TacticalSideSign = (StableHash & 1u) == 0u ? 1.0f : -1.0f;

    UE_LOG(LogTemp, Display, TEXT("RoadsideIdiots AI24: %s role=%s race=%.0f grudge=%.1fs"),
        *Id, *PersonalityLabel, TargetSpeedKph, GrudgeDurationSeconds);
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
    LaneOffset = InLaneOffset;
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

    // Fight fatigue: an impact can still create a short grudge and opportunistic
    // item response, but it cannot keep resetting a chase while an intent is
    // already active or while the rider is in its post-fight cooldown.
    if (IsTacticalIntentActive() || TacticalCooldownRemaining > 0.0f) return;

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
        if (ForwardDot < 0.25f) return;
        const float Score = DistanceSq * FMath::Lerp(1.12f, 0.84f, FMath::Clamp(ForwardDot, 0.0f, 1.0f));
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

float ARIAIController::ComputeAvoidanceShift(const FVector& BikeLocation, const FVector& Forward, const FVector& Right) const
{
    if (!Bike || !GetWorld()) return 0.0f;
    float Shift = 0.0f;
    const FVector SelfVelocity = Bike->GetChassis() ? Bike->GetChassis()->GetPhysicsLinearVelocity() : FVector::ZeroVector;

    auto ConsiderObstacle = [&](const AActor* Obstacle, const FVector& ObstacleVelocity, const float LookAhead, const float SideClearance, const float MaxShift, const float Weight)
    {
        if (!Obstacle || Obstacle == Bike) return;
        FVector ToObstacle = Obstacle->GetActorLocation() - BikeLocation;
        ToObstacle.Z = 0.0f;
        const float Distance = ToObstacle.Size();
        if (Distance < 75.0f || Distance > LookAhead) return;
        const FVector Direction = ToObstacle / Distance;
        const float ForwardDot = FVector::DotProduct(Direction, Forward);
        if (ForwardDot < 0.20f) return;
        const float Side = FVector::DotProduct(ToObstacle, Right);
        if (FMath::Abs(Side) > SideClearance) return;
        const float ClosingSpeed = FMath::Max(0.0f, FVector::DotProduct(SelfVelocity - ObstacleVelocity, Direction));
        const float PredictedDistance = FMath::Max(60.0f, Distance - ClosingSpeed * 0.75f);
        const float Urgency = 1.0f - FMath::Clamp(PredictedDistance / LookAhead, 0.0f, 1.0f);
        const float DodgeSign = Side >= 0.0f ? -1.0f : 1.0f;
        Shift += DodgeSign * MaxShift * Urgency * Weight;
    };

    for (TActorIterator<ARITrafficVehicle> It(GetWorld()); It; ++It)
        ConsiderObstacle(*It, FVector::ZeroVector, 1450.0f, 340.0f, 330.0f, 1.0f);
    for (TActorIterator<ARIPoopHazard> It(GetWorld()); It; ++It)
        ConsiderObstacle(*It, FVector::ZeroVector, 900.0f, 205.0f, 260.0f, 1.0f);
    for (TActorIterator<ARIBananaPeelHazard> It(GetWorld()); It; ++It)
        ConsiderObstacle(*It, FVector::ZeroVector, 850.0f, 190.0f, 250.0f, 1.0f);

    for (TActorIterator<ARIBikePawn> It(GetWorld()); It; ++It)
    {
        ARIBikePawn* OtherBike = *It;
        if (!OtherBike || OtherBike == Bike) continue;
        const FVector OtherVelocity = OtherBike->GetChassis() ? OtherBike->GetChassis()->GetPhysicsLinearVelocity() : FVector::ZeroVector;
        const bool bIntentTarget = IsTacticalIntentActive() && TacticalTarget.Get() == OtherBike;
        const float Weight = bIntentTarget && TacticalIntent == ERITacticalIntent::SidePressure ? 0.48f : 1.0f;
        ConsiderObstacle(OtherBike, OtherVelocity, 1100.0f, 250.0f, 255.0f, Weight);
    }

    return FMath::Clamp(Shift * AvoidanceStrength, -260.0f, 260.0f);
}

float ARIAIController::ComputeCrowdSpeedScale(const FVector& BikeLocation, const FVector& Forward, const FVector& Right) const
{
    if (!Bike || !GetWorld()) return 1.0f;
    float SpeedScale = 1.0f;
    const FVector SelfVelocity = Bike->GetChassis() ? Bike->GetChassis()->GetPhysicsLinearVelocity() : FVector::ZeroVector;

    for (TActorIterator<ARIBikePawn> It(GetWorld()); It; ++It)
    {
        ARIBikePawn* OtherBike = *It;
        if (!OtherBike || OtherBike == Bike) continue;
        FVector ToOther = OtherBike->GetActorLocation() - BikeLocation;
        ToOther.Z = 0.0f;
        const float Distance = ToOther.Size();
        if (Distance < 1.0f || Distance > CrowdLookAhead) continue;
        const FVector Direction = ToOther / Distance;
        if (FVector::DotProduct(Direction, Forward) < 0.35f) continue;
        if (FMath::Abs(FVector::DotProduct(ToOther, Right)) > CrowdSideClearance) continue;

        const FVector OtherVelocity = OtherBike->GetChassis() ? OtherBike->GetChassis()->GetPhysicsLinearVelocity() : FVector::ZeroVector;
        const float ClosingSpeed = FMath::Max(0.0f, FVector::DotProduct(SelfVelocity - OtherVelocity, Direction));
        const float PredictedDistance = FMath::Max(100.0f, Distance - ClosingSpeed * 0.85f);
        const bool bIntentTarget = IsTacticalIntentActive() && TacticalTarget.Get() == OtherBike;
        const float MinimumScale = bIntentTarget ? 0.55f : 0.34f;
        const float LocalScale = FMath::GetMappedRangeValueClamped(
            FVector2D(150.0f, CrowdLookAhead), FVector2D(MinimumScale, 1.0f), PredictedDistance);
        SpeedScale = FMath::Min(SpeedScale, LocalScale);
    }
    return SpeedScale;
}

FVector ARIAIController::ComputeRaceLookAheadTarget(const FVector& BikeLocation, const float SpeedKph, FVector& OutTangent, FVector& OutRight, float& OutTurnSeverity)
{
    OutTangent = Bike ? Bike->GetActorForwardVector().GetSafeNormal2D() : FVector::ForwardVector;
    OutRight = FVector::CrossProduct(FVector::UpVector, OutTangent).GetSafeNormal();
    OutTurnSeverity = 0.0f;
    if (RoutePoints.Num() < 3) return BikeLocation + OutTangent * MinLookAheadDistance;

    const int32 Count = RoutePoints.Num();
    int32 NearestIndex = 0;
    float NearestDistanceSq = TNumericLimits<float>::Max();
    for (int32 Index = 0; Index < Count; ++Index)
    {
        FVector Delta = RoutePoints[Index] - BikeLocation;
        Delta.Z = 0.0f;
        const float DistanceSq = Delta.SizeSquared();
        if (DistanceSq < NearestDistanceSq)
        {
            NearestDistanceSq = DistanceSq;
            NearestIndex = Index;
        }
    }

    const float SpeedAlpha = FMath::Clamp(SpeedKph / 120.0f, 0.0f, 1.0f);
    const float LookAhead = FMath::Lerp(MinLookAheadDistance, MaxLookAheadDistance, SpeedAlpha);
    int32 SegmentStartIndex = NearestIndex;
    FVector SegmentStart = RoutePoints[SegmentStartIndex];
    float Accumulated = 0.0f;
    FVector Target = RoutePoints[(NearestIndex + 1) % Count];

    for (int32 Step = 0; Step < Count; ++Step)
    {
        const int32 SegmentEndIndex = (SegmentStartIndex + 1) % Count;
        const FVector SegmentEnd = RoutePoints[SegmentEndIndex];
        const float SegmentLength = FVector::Dist2D(SegmentStart, SegmentEnd);
        if (SegmentLength > KINDA_SMALL_NUMBER && Accumulated + SegmentLength >= LookAhead)
        {
            const float Alpha = FMath::Clamp((LookAhead - Accumulated) / SegmentLength, 0.0f, 1.0f);
            Target = FMath::Lerp(SegmentStart, SegmentEnd, Alpha);
            TargetIndex = SegmentEndIndex;
            break;
        }
        Accumulated += SegmentLength;
        SegmentStartIndex = SegmentEndIndex;
        SegmentStart = SegmentEnd;
        Target = SegmentEnd;
        TargetIndex = SegmentEndIndex;
    }

    const int32 PrevIndex = (TargetIndex - 1 + Count) % Count;
    const int32 NextIndex = (TargetIndex + 1) % Count;
    const int32 Next2Index = (TargetIndex + 2) % Count;
    OutTangent = (RoutePoints[NextIndex] - RoutePoints[PrevIndex]).GetSafeNormal2D();
    OutRight = FVector::CrossProduct(FVector::UpVector, OutTangent).GetSafeNormal();
    const FVector FutureTangent = (RoutePoints[Next2Index] - RoutePoints[TargetIndex]).GetSafeNormal2D();
    const float TurnDot = FMath::Clamp(FVector::DotProduct(OutTangent, FutureTangent), -1.0f, 1.0f);
    OutTurnSeverity = FMath::Clamp(FMath::Acos(TurnDot) / 0.80f, 0.0f, 1.0f);
    return Target + OutRight * LaneOffset;
}

void ARIAIController::ApplyTacticalTargeting(FVector& InOutTargetPoint, const FVector& BikeLocation, const FVector& RouteRight)
{
    if (!IsTacticalIntentActive()) return;
    ARIBikePawn* Target = TacticalTarget.Get();
    if (!Target) { EndTacticalIntent(); return; }

    FVector ToTarget = Target->GetActorLocation() - BikeLocation;
    ToTarget.Z = 0.0f;
    const float Distance = ToTarget.Size();
    const FVector TargetForward = Target->GetActorForwardVector().GetSafeNormal2D();
    const FVector TargetRight = Target->GetActorRightVector().GetSafeNormal2D();
    const FVector TargetVelocity = Target->GetChassis() ? Target->GetChassis()->GetPhysicsLinearVelocity() : FVector::ZeroVector;
    const FVector PredictedTarget = Target->GetActorLocation() + FVector(TargetVelocity.X, TargetVelocity.Y, 0.0f) * 0.22f;

    switch (TacticalIntent)
    {
    case ERITacticalIntent::SidePressure:
    {
        const FVector PressurePoint = PredictedTarget + TargetForward * 230.0f + TargetRight * (TacticalSideSign * 125.0f);
        InOutTargetPoint = FMath::Lerp(InOutTargetPoint, PressurePoint, 0.58f);
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
        FVector TargetToSelf = BikeLocation - Target->GetActorLocation();
        TargetToSelf.Z = 0.0f;
        const float AheadDistance = FVector::DotProduct(TargetToSelf, TargetForward);
        const float LeadDistance = AheadDistance > 180.0f ? 230.0f : 520.0f;
        InOutTargetPoint = FMath::Lerp(InOutTargetPoint,
            PredictedTarget + TargetForward * LeadDistance + TargetRight * (TacticalSideSign * 85.0f), 0.46f);
        break;
    }
    case ERITacticalIntent::PeelTrap:
    {
        InOutTargetPoint = FMath::Lerp(InOutTargetPoint,
            PredictedTarget + TargetForward * 520.0f + RouteRight * (TacticalSideSign * 55.0f), 0.42f);
        FVector TargetToSelf = BikeLocation - Target->GetActorLocation();
        TargetToSelf.Z = 0.0f;
        const float AheadDistance = FVector::DotProduct(TargetToSelf, TargetForward);
        const float SideDistance = FMath::Abs(FVector::DotProduct(TargetToSelf, TargetRight));
        if (!bTacticalItemCommitted && Bike->GetBananaPeelCount() > 0 && PeelUseCooldownRemaining <= 0.0f &&
            AheadDistance > 260.0f && AheadDistance < 900.0f && SideDistance < 260.0f)
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
        InOutTargetPoint = FMath::Lerp(InOutTargetPoint,
            PredictedTarget + TargetRight * (TacticalSideSign * 70.0f), 0.16f);
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
    default: break;
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
    if (!Bike || RoutePoints.Num() < 3) return;

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
    const float SpeedKph = FMath::Abs(Bike->GetBikeMovement()->GetForwardSpeedKph());
    FVector RouteTangent;
    FVector RouteRight;
    float TurnSeverity = 0.0f;
    FVector TargetPoint = ComputeRaceLookAheadTarget(BikeLocation, SpeedKph, RouteTangent, RouteRight, TurnSeverity);
    const FVector Forward = Bike->GetActorForwardVector().GetSafeNormal2D();

    if (SenseRefreshRemaining <= 0.0f)
    {
        SenseRefreshRemaining = FMath::Max(0.08f, SenseRefreshIntervalSeconds);
        bHasCachedPickupTarget = !IsTacticalIntentActive() && GrudgeTimeRemaining <= 0.0f && FindUsefulPickupTarget(CachedPickupTarget);
        CachedAvoidanceShift = ComputeAvoidanceShift(BikeLocation, Forward, RouteRight);
        CachedCrowdSpeedScale = ComputeCrowdSpeedScale(BikeLocation, Forward, RouteRight);
    }

    SmoothedAvoidanceShift = FMath::FInterpTo(SmoothedAvoidanceShift, CachedAvoidanceShift, DeltaSeconds, 4.2f);
    TargetPoint += RouteRight * SmoothedAvoidanceShift;

    if (!IsTacticalIntentActive() && GrudgeTimeRemaining <= 0.0f && bHasCachedPickupTarget)
        TargetPoint = FMath::Lerp(TargetPoint, CachedPickupTarget, 0.42f);

    ApplyTacticalTargeting(TargetPoint, BikeLocation, RouteRight);

    if (ItemDecisionRemaining <= 0.0f)
    {
        ItemDecisionRemaining = FMath::Max(0.14f, ItemDecisionIntervalSeconds);
        TryUseComedyItems();
    }

    FVector ToTarget = TargetPoint - BikeLocation;
    ToTarget.Z = 0.0f;
    const FVector Desired = ToTarget.GetSafeNormal();
    const float Dot = FMath::Clamp(FVector::DotProduct(Forward, Desired), -1.0f, 1.0f);
    const float Angle = FMath::Atan2(FVector::CrossProduct(Forward, Desired).Z, Dot);
    const float AbsAngle = FMath::Abs(Angle);
    const float RawSteering = FMath::Clamp(Angle / 0.62f, -1.0f, 1.0f);
    SmoothedSteering = FMath::FInterpTo(SmoothedSteering, RawSteering, DeltaSeconds, SteeringInterpSpeed);

    if (SpeedKph < 6.0f && ToTarget.SizeSquared() > FMath::Square(WaypointReachDistance * 0.45f)) LowMotionTime += DeltaSeconds;
    else LowMotionTime = 0.0f;

    if (LowMotionTime > 1.10f && LowMotionTime < 2.25f)
    {
        const float ReverseSteer = FMath::Abs(SmoothedSteering) > 0.12f ? -SmoothedSteering : (LaneOffset >= 0.0f ? -0.55f : 0.55f);
        Bike->SetControlInputs(-0.48f, ReverseSteer, 0.0f);
        return;
    }
    if (LowMotionTime >= 2.25f)
    {
        Bike->RecoverBike();
        LowMotionTime = 0.0f;
        EndTacticalIntent(6.0f);
        return;
    }

    float DesiredSpeed = TargetSpeedKph;
    if (GrudgeTimeRemaining > 0.0f || IsTacticalIntentActive())
        DesiredSpeed = FMath::Min(GrudgeCatchupSpeedKph, TargetSpeedKph + 8.0f);

    DesiredSpeed *= FMath::Lerp(1.0f, 0.68f, TurnSeverity);
    DesiredSpeed *= FMath::Clamp(CachedCrowdSpeedScale, 0.30f, 1.0f);
    if (AbsAngle > 0.72f) DesiredSpeed = FMath::Min(DesiredSpeed, 60.0f);
    else if (AbsAngle > 0.38f) DesiredSpeed = FMath::Min(DesiredSpeed, 82.0f);

    float DesiredBrake = SpeedKph > DesiredSpeed + 7.0f ? 0.60f : 0.0f;
    if (CachedCrowdSpeedScale < 0.48f && SpeedKph > 28.0f) DesiredBrake = FMath::Max(DesiredBrake, 0.72f);
    float DesiredThrottle = DesiredBrake > 0.0f ? 0.08f : 1.0f;
    if (CachedCrowdSpeedScale < 0.62f) DesiredThrottle = FMath::Min(DesiredThrottle, 0.38f);
    if (AbsAngle > 0.90f) DesiredThrottle = FMath::Min(DesiredThrottle, 0.36f);

    SmoothedThrottle = FMath::FInterpTo(SmoothedThrottle, DesiredThrottle, DeltaSeconds, 4.5f);
    SmoothedBrake = FMath::FInterpTo(SmoothedBrake, DesiredBrake, DeltaSeconds, 7.0f);
    Bike->SetControlInputs(SmoothedThrottle, SmoothedSteering, SmoothedBrake);
}
