#include "Traffic/RITrafficReadabilitySubsystem.h"

#include "Audio/RIAudioEvents.h"
#include "Traffic/RITrafficVehicle.h"
#include "Vehicle/RIBikePawn.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

bool URITrafficReadabilitySubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !IsTemplate() && World && World->IsGameWorld();
}

TStatId URITrafficReadabilitySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URITrafficReadabilitySubsystem, STATGROUP_Tickables);
}

void URITrafficReadabilitySubsystem::Tick(const float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    ScanAccumulator += FMath::Max(0.0f, DeltaTime);
    if (ScanAccumulator < 0.15f) return;
    ScanAccumulator = FMath::Fmod(ScanAccumulator, 0.15f);

    const double Now = World->GetTimeSeconds();
    if (Now - LastWarningTime < 2.8)
    {
        return;
    }

    ARIBikePawn* PlayerBike = Cast<ARIBikePawn>(UGameplayStatics::GetPlayerPawn(World, 0));
    if (!PlayerBike || !PlayerBike->AreRaceControlsEnabled()) return;

    UStaticMeshComponent* PlayerChassis = PlayerBike->GetChassis();
    if (!PlayerChassis) return;

    const FVector PlayerVelocity = PlayerChassis->GetPhysicsLinearVelocity();
    const FVector PlayerForward = PlayerBike->GetActorForwardVector().GetSafeNormal2D();
    const FVector PlayerRight = PlayerBike->GetActorRightVector().GetSafeNormal2D();

    ARITrafficVehicle* BestTraffic = nullptr;
    float BestTimeToContact = TNumericLimits<float>::Max();
    float BestDistance = 0.0f;

    for (TActorIterator<ARITrafficVehicle> It(World); It; ++It)
    {
        ARITrafficVehicle* Traffic = *It;
        if (!Traffic) continue;

        FVector ToTraffic = Traffic->GetActorLocation() - PlayerBike->GetActorLocation();
        ToTraffic.Z = 0.0f;
        const float Distance = ToTraffic.Size();
        if (Distance < 520.0f || Distance > 2200.0f) continue;

        const FVector DirectionToTraffic = ToTraffic / Distance;
        const float ForwardDot = FVector::DotProduct(DirectionToTraffic, PlayerForward);
        if (ForwardDot < 0.58f) continue;

        // Only warn for vehicles occupying roughly the player's near-future
        // corridor. A vehicle on the opposite side of the road should not honk
        // just because it is geometrically in front of the camera.
        const float LateralSeparation = FMath::Abs(FVector::DotProduct(ToTraffic, PlayerRight));
        if (LateralSeparation > 430.0f) continue;

        const FVector RelativeVelocity = PlayerVelocity - Traffic->GetTrafficVelocityEstimate();
        const float ClosingSpeedCms = FVector::DotProduct(RelativeVelocity, DirectionToTraffic);
        if (ClosingSpeedCms < 900.0f) continue;

        const float TimeToContact = Distance / FMath::Max(1.0f, ClosingSpeedCms);
        if (TimeToContact > 1.25f) continue;

        if (TimeToContact < BestTimeToContact)
        {
            BestTraffic = Traffic;
            BestTimeToContact = TimeToContact;
            BestDistance = Distance;
        }
    }

    if (!BestTraffic) return;

    float Pitch = 1.0f;
    if (BestTraffic->GetTrafficLabel().Equals(TEXT("CNG AUTO"), ESearchCase::IgnoreCase)) Pitch = 1.14f;
    else if (BestTraffic->GetTrafficLabel().Equals(TEXT("MICROBUS"), ESearchCase::IgnoreCase)) Pitch = 0.88f;
    else if (BestTraffic->GetTrafficLabel().Equals(TEXT("DELIVERY VAN"), ESearchCase::IgnoreCase)) Pitch = 0.94f;

    RIAudioEvents::Play(
        BestTraffic,
        FName(TEXT("Honk")),
        BestTraffic->GetActorLocation(),
        0.58f,
        Pitch);

    LastWarningTime = Now;

    UE_LOG(
        LogTemp,
        Verbose,
        TEXT("RI TRAFFIC WARN label=%s distance=%.0fcm ttc=%.2fs"),
        *BestTraffic->GetTrafficLabel(),
        BestDistance,
        BestTimeToContact);
}
