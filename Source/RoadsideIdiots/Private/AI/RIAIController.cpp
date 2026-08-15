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
        PersonalityLabel = TEXT("LEECH");
        GrudgeDurationSeconds = 28.0f;
        GrudgeCatchupSpeedKph = 150.0f;
        AttackCooldownSeconds = 1.85f;
        AttackRange = 245.0f;
        PickupSeekRange = 1550.0f;
        AvoidanceStrength = 0.72f;
        EggUseCooldownSeconds = 4.2f;
        PeelUseCooldownSeconds = 4.8f;
    }
    else if (Id.Equals(TEXT("BOT_02"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("HOTHEAD");
        GrudgeDurationSeconds = 10.0f;
        GrudgeCatchupSpeedKph = 153.0f;
        AttackCooldownSeconds = 1.30f;
        AttackRange = 250.0f;
        PickupSeekRange = 1250.0f;
        AvoidanceStrength = 0.52f;
        EggUseCooldownSeconds = 2.5f;
        PeelUseCooldownSeconds = 4.4f;
    }
    else if (Id.Equals(TEXT("BOT_03"), ESearchCase::IgnoreCase))
    {
        PersonalityLabel = TEXT("PETTY");
        GrudgeDurationSeconds = 15.0f;
        GrudgeCatchupSpeedKph = 145.0f;
        AttackCooldownSeconds = 2.05f;
        AttackRange = 235.0f;
        PickupSeekRange = 1750.0f;
        AvoidanceStrength = 0.92f;
        EggUseCooldownSeconds = 4.4f;
        PeelUseCooldownSeconds = 2.7f;
    }

    UE_LOG(LogTemp, Display, TEXT("RoadsideIdiots AI: %s personality=%s grudge=%.0fs attackCooldown=%.2fs egg=%.2fs peel=%.2fs"),
        *Id,
        *PersonalityLabel,
        GrudgeDurationSeconds,
        AttackCooldownSeconds,
        EggUseCooldownSeconds,
        PeelUseCooldownSeconds);
}

bool ARIAIController::IsHoldingGrudgeAgainst(const ARIBikePawn* Target) const
{
    return Target && GrudgeTimeRemaining > 0.0f && GrudgeTarget.IsValid() && GrudgeTarget.Get() == Target;
}

void ARIAIController::SetRoute(const TArray<FVector>& InRoutePoints, int32 StartTargetIndex, float InLaneOffset)
{
    RoutePoints = InRoutePoints;
    TargetIndex = RoutePoints.Num() > 0 ? FMath::Abs(StartTargetIndex) % RoutePoints.Num() : 0;
    LaneOffset = InLaneOffset;
}

void ARIAIController::NotifyProvokedBy(ARIBikePawn* InstigatorBike)
{
    if (InstigatorBike && InstigatorBike != Bike)
    {
        const bool bSameTargetAlready = GrudgeTarget.IsValid() && GrudgeTarget.Get() == InstigatorBike && GrudgeTimeRemaining > 0.0f;
        GrudgeTarget = InstigatorBike;

        if (bSameTargetAlready)
        {
            GrudgeTimeRemaining = FMath::Min(
                GrudgeTimeRemaining + 3.0f,
                GrudgeDurationSeconds * 1.35f);
        }
        else
        {
            GrudgeTimeRemaining = GrudgeDurationSeconds;
        }

        AttackCooldownRemaining = 0.25f;
    }
}

ARIBikePawn* ARIAIController::FindBestItemVictim() const
{
    if (!Bike || !GetWorld()) return nullptr;

    if (GrudgeTimeRemaining > 0.0f && GrudgeTarget.IsValid())
    {
        return GrudgeTarget.Get();
    }

    ARIBikePawn* NearestHuman = nullptr;
    float NearestHumanDistanceSq = TNumericLimits<float>::Max();
    ARIBikePawn* NearestAny = nullptr;
    float NearestAnyDistanceSq = TNumericLimits<float>::Max();

    for (TActorIterator<ARIBikePawn> It(GetWorld()); It; ++It)
    {
        ARIBikePawn* Candidate = *It;
        if (!Candidate || Candidate == Bike) continue;

        const float DistanceSq = FVector::DistSquared2D(Bike->GetActorLocation(), Candidate->GetActorLocation());
        if (DistanceSq < NearestAnyDistanceSq)
        {
            NearestAnyDistanceSq = DistanceSq;
            NearestAny = Candidate;
        }

        const URIParticipantComponent* Participant = Candidate->GetParticipantComponent();
        if (Participant && Participant->IsHumanControlled() && DistanceSq < NearestHumanDistanceSq)
        {
            NearestHumanDistanceSq = DistanceSq;
            NearestHuman = Candidate;
        }
    }

    return NearestHuman ? NearestHuman : NearestAny;
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
        if (DistanceSq > MaxDistanceSq || DistanceSq < FMath::Square(120.0f)) return;

        const FVector Direction = ToPickup.GetSafeNormal();
        const float ForwardDot = FVector::DotProduct(Direction, Forward);
        if (ForwardDot < 0.20f) return;

        const float Score = DistanceSq * FMath::Lerp(1.15f, 0.82f, FMath::Clamp(ForwardDot, 0.0f, 1.0f));
        if (Score < BestScore)
        {
            BestScore = Score;
            OutTarget = Pickup->GetActorLocation();
            bFound = true;
        }
    };

    for (TActorIterator<ARIBananaPickup> It(GetWorld()); It; ++It)
    {
        ConsiderPickup(*It, HealthFraction < 0.88f || Bike->GetBananaPeelCount() < 1);
    }

    for (TActorIterator<ARIRottenEggPickup> It(GetWorld()); It; ++It)
    {
        ConsiderPickup(*It, Bike->GetRottenEggCount() < Bike->GetMaxRottenEggs());
    }

    return bFound;
}

float ARIAIController::ComputeAvoidanceShift(const FVector& BikeLocation, const FVector& Forward, const FVector& Right) const
{
    if (!Bike || !GetWorld()) return 0.0f;

    float Shift = 0.0f;

    auto ConsiderObstacle = [&](const AActor* Obstacle, const float LookAhead, const float SideClearance, const float MaxShift, const float Weight)
    {
        if (!Obstacle || Obstacle == Bike) return;

        FVector ToObstacle = Obstacle->GetActorLocation() - BikeLocation;
        ToObstacle.Z = 0.0f;
        const float Distance = ToObstacle.Size();
        if (Distance < 80.0f || Distance > LookAhead) return;

        const FVector Direction = ToObstacle / Distance;
        const float ForwardDot = FVector::DotProduct(Direction, Forward);
        if (ForwardDot < 0.35f) return;

        const float Side = FVector::DotProduct(ToObstacle, Right);
        if (FMath::Abs(Side) > SideClearance) return;

        const float Urgency = 1.0f - Distance / LookAhead;
        const float DodgeSign = Side >= 0.0f ? -1.0f : 1.0f;
        Shift += DodgeSign * MaxShift * Urgency * Weight;
    };

    for (TActorIterator<ARITrafficVehicle> It(GetWorld()); It; ++It)
    {
        ConsiderObstacle(*It, 1150.0f, 300.0f, 310.0f, 1.0f);
    }

    const float HazardWeight = (PersonalityLabel.Equals(TEXT("HOTHEAD"), ESearchCase::IgnoreCase) && GrudgeTimeRemaining > 0.0f)
        ? 0.30f
        : 1.0f;

    for (TActorIterator<ARIPoopHazard> It(GetWorld()); It; ++It)
    {
        ConsiderObstacle(*It, 850.0f, 190.0f, 250.0f, HazardWeight);
    }

    for (TActorIterator<ARIBananaPeelHazard> It(GetWorld()); It; ++It)
    {
        ConsiderObstacle(*It, 760.0f, 165.0f, 235.0f, HazardWeight);
    }

    for (TActorIterator<ARIBikePawn> It(GetWorld()); It; ++It)
    {
        ARIBikePawn* OtherBike = *It;
        if (!OtherBike || OtherBike == Bike) continue;
        if (GrudgeTimeRemaining > 0.0f && GrudgeTarget.IsValid() && OtherBike == GrudgeTarget.Get()) continue;
        ConsiderObstacle(OtherBike, 720.0f, 145.0f, 165.0f, 0.75f);
    }

    return FMath::Clamp(Shift * AvoidanceStrength, -340.0f, 340.0f);
}

void ARIAIController::TryUseComedyItems()
{
    if (!Bike) return;

    ARIBikePawn* Victim = FindBestItemVictim();
    if (!Victim) return;

    FVector ToVictim = Victim->GetActorLocation() - Bike->GetActorLocation();
    ToVictim.Z = 0.0f;
    const float Distance = ToVictim.Size();
    if (Distance < 1.0f) return;

    const FVector Direction = ToVictim / Distance;
    const float ForwardDot = FVector::DotProduct(Direction, Bike->GetActorForwardVector().GetSafeNormal2D());

    if (Bike->GetRottenEggCount() > 0 && EggUseCooldownRemaining <= 0.0f &&
        Distance > 450.0f && Distance < 2300.0f && ForwardDot > 0.56f)
    {
        if (Bike->ThrowRottenEggAt(Victim))
        {
            EggUseCooldownRemaining = EggUseCooldownSeconds;
            return;
        }
    }

    if (Bike->GetBananaPeelCount() > 0 && PeelUseCooldownRemaining <= 0.0f &&
        Distance > 260.0f && Distance < 1050.0f && ForwardDot < -0.18f)
    {
        if (Bike->DropBananaPeel())
        {
            PeelUseCooldownRemaining = PeelUseCooldownSeconds;
        }
    }
}

void ARIAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Bike || RoutePoints.Num() < 3) return;

    AttackCooldownRemaining = FMath::Max(0.0f, AttackCooldownRemaining - DeltaSeconds);
    EggUseCooldownRemaining = FMath::Max(0.0f, EggUseCooldownRemaining - DeltaSeconds);
    PeelUseCooldownRemaining = FMath::Max(0.0f, PeelUseCooldownRemaining - DeltaSeconds);

    if (GrudgeTimeRemaining > 0.0f)
    {
        GrudgeTimeRemaining = FMath::Max(0.0f, GrudgeTimeRemaining - DeltaSeconds);
    }
    else
    {
        GrudgeTarget.Reset();
    }

    const int32 Count = RoutePoints.Num();
    const FVector BikeLocation = Bike->GetActorLocation();

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

    TargetIndex = (NearestIndex + 1) % Count;
    const int32 PrevIndex = (TargetIndex - 1 + Count) % Count;
    const int32 NextIndex = (TargetIndex + 1) % Count;
    const FVector Tangent = (RoutePoints[NextIndex] - RoutePoints[PrevIndex]).GetSafeNormal2D();
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();
    FVector TargetPoint = RoutePoints[TargetIndex] + Right * LaneOffset;

    const bool bFollowingRival = GrudgeTimeRemaining > 0.0f && GrudgeTarget.IsValid();
    if (bFollowingRival)
    {
        ARIBikePawn* RivalBike = GrudgeTarget.Get();
        const FVector RivalLocation = RivalBike->GetActorLocation();
        const float RivalDistanceSq = FVector::DistSquared2D(BikeLocation, RivalLocation);

        if (RivalDistanceSq < FMath::Square(2200.0f))
        {
            TargetPoint = RivalLocation + RivalBike->GetActorForwardVector() * 110.0f;
        }

        if (RivalDistanceSq < FMath::Square(AttackRange) && AttackCooldownRemaining <= 0.0f)
        {
            FVector ToRival = RivalLocation - BikeLocation;
            ToRival.Z = 0.0f;
            const float RivalSide = FVector::DotProduct(ToRival.GetSafeNormal(), Bike->GetActorRightVector());
            if (URIInteractionComponent* Interaction = Bike->GetInteractionComponent())
            {
                const bool bConnected = Interaction->TrySideInteraction(RivalSide < 0.0f ? -1.0f : 1.0f);
                AttackCooldownRemaining = bConnected ? AttackCooldownSeconds : 0.35f;
            }
        }
    }
    else
    {
        FVector PickupTarget;
        if (FindUsefulPickupTarget(PickupTarget))
        {
            TargetPoint = FMath::Lerp(TargetPoint, PickupTarget, 0.78f);
        }
    }

    const FVector Forward = Bike->GetActorForwardVector().GetSafeNormal2D();
    TargetPoint += Right * ComputeAvoidanceShift(BikeLocation, Forward, Right);

    TryUseComedyItems();

    FVector ToTarget = TargetPoint - BikeLocation;
    ToTarget.Z = 0.0f;

    const float SpeedKph = FMath::Abs(Bike->GetBikeMovement()->GetForwardSpeedKph());

    static TMap<TWeakObjectPtr<ARIAIController>, float> LowMotionTimes;
    float& LowMotionTime = LowMotionTimes.FindOrAdd(this);
    if (SpeedKph < 7.0f && ToTarget.SizeSquared() > FMath::Square(WaypointReachDistance * 0.5f))
    {
        LowMotionTime += DeltaSeconds;
    }
    else
    {
        LowMotionTime = 0.0f;
    }

    if (LowMotionTime > 1.6f)
    {
        Bike->RecoverBike();
        LowMotionTime = 0.0f;
        return;
    }

    const FVector Desired = ToTarget.GetSafeNormal();
    const float Dot = FMath::Clamp(FVector::DotProduct(Forward, Desired), -1.0f, 1.0f);
    const float CrossZ = FVector::CrossProduct(Forward, Desired).Z;
    const float Angle = FMath::Atan2(CrossZ, Dot);
    const float AbsAngle = FMath::Abs(Angle);
    const float Steering = FMath::Clamp(Angle / 0.55f, -1.0f, 1.0f);

    float DesiredSpeed = bFollowingRival ? GrudgeCatchupSpeedKph : FMath::Min(TargetSpeedKph, 108.0f);
    if (AbsAngle > 0.70f)
    {
        DesiredSpeed = FMath::Min(DesiredSpeed, 62.0f);
    }
    else if (AbsAngle > 0.35f)
    {
        DesiredSpeed = FMath::Min(DesiredSpeed, 86.0f);
    }

    const float Brake = SpeedKph > DesiredSpeed + 6.0f ? 0.65f : 0.0f;
    float Throttle = Brake > 0.0f ? 0.10f : 1.0f;
    if (AbsAngle > 0.90f)
    {
        Throttle = FMath::Min(Throttle, 0.40f);
    }

    Bike->SetControlInputs(Throttle, Steering, Brake);
}
