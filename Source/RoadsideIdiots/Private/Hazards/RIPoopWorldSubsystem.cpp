#include "Hazards/RIPoopWorldSubsystem.h"

#include "Hazards/RIPoopHazard.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    constexpr float RouteRadiusX = 9000.0f;
    constexpr float RouteRadiusY = 5000.0f;
    constexpr float HazardHeight = 18.0f;

    FTransform MakeHazardTransform(const float AngleRadians, const float LaneOffset)
    {
        const FVector Center(
            FMath::Cos(AngleRadians) * RouteRadiusX,
            FMath::Sin(AngleRadians) * RouteRadiusY,
            HazardHeight);

        const FVector Tangent(
            -FMath::Sin(AngleRadians) * RouteRadiusX,
            FMath::Cos(AngleRadians) * RouteRadiusY,
            0.0f);

        const FVector Forward = Tangent.GetSafeNormal2D();
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        return FTransform(Forward.Rotation(), Center + Right * LaneOffset);
    }
}

bool URIPoopWorldSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIPoopWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIPoopWorldSubsystem, STATGROUP_Tickables);
}

void URIPoopWorldSubsystem::TrySpawnMapHazards()
{
    if (bSpawned) return;

    UWorld* World = GetWorld();
    if (!World) return;

    struct FPoopSeed
    {
        float Angle;
        float LaneOffset;
        ERIPoopHazardType Type;
    };

    // Prototype rural-ish mix. Later maps can replace this seed table with
    // their own hazard profile without changing ARIPoopHazard behavior.
    const FPoopSeed Seeds[] =
    {
        {0.42f,  175.0f, ERIPoopHazardType::Dog},
        {1.48f, -245.0f, ERIPoopHazardType::Cow},
        {2.38f,  -55.0f, ERIPoopHazardType::Dog},
        {3.62f,  255.0f, ERIPoopHazardType::Cow},
        {4.55f,   45.0f, ERIPoopHazardType::Dog},
        {5.55f, -205.0f, ERIPoopHazardType::Cow}
    };

    for (const FPoopSeed& Seed : Seeds)
    {
        const FTransform SpawnTransform = MakeHazardTransform(Seed.Angle, Seed.LaneOffset);
        AActor* DeferredActor = UGameplayStatics::BeginDeferredActorSpawnFromClass(
            World,
            ARIPoopHazard::StaticClass(),
            SpawnTransform,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
            nullptr);

        if (ARIPoopHazard* Hazard = Cast<ARIPoopHazard>(DeferredActor))
        {
            Hazard->Configure(Seed.Type);
            UGameplayStatics::FinishSpawningActor(Hazard, SpawnTransform);

            if (Seed.Type == ERIPoopHazardType::Cow)
            {
                ++SpawnedCowPoopCount;
            }
            else
            {
                ++SpawnedDogPoopCount;
            }
        }
    }

    bSpawned = true;
}

void URIPoopWorldSubsystem::Tick(float DeltaTime)
{
    TrySpawnMapHazards();
}
