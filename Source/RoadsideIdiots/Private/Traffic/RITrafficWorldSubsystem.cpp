#include "Traffic/RITrafficWorldSubsystem.h"

#include "Traffic/RITrafficVehicle.h"
#include "Core/RIRaceSettingsSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

bool URITrafficWorldSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !IsTemplate() && World && World->IsGameWorld();
}

TStatId URITrafficWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URITrafficWorldSubsystem, STATGROUP_Tickables);
}

void URITrafficWorldSubsystem::TrySpawnTraffic()
{
    if (bTrafficSpawned) return;

    UWorld* World = GetWorld();
    if (!World) return;

    int32 TrafficCount = 3;
    if (UGameInstance* GameInstance = World->GetGameInstance())
    {
        if (const URIRaceSettingsSubsystem* Settings = GameInstance->GetSubsystem<URIRaceSettingsSubsystem>())
        {
            TrafficCount = FMath::Clamp(Settings->GetTrafficCount(), 0, 6);
        }
    }

    struct FTrafficSeed
    {
        float Angle;
        float SpeedKph;
        float LaneOffset;
        FLinearColor Color;
        bool bWanders;
        float WanderPhase;
        const TCHAR* Label;
    };

    const FTrafficSeed Seeds[] =
    {
        {0.78f, 42.0f, -255.0f, FLinearColor(0.95f, 0.72f, 0.08f, 1.0f), false, 0.0f, TEXT("SUNDAY DRIVER")},
        {2.82f, 58.0f,  245.0f, FLinearColor(0.08f, 0.38f, 0.90f, 1.0f), true,  1.4f, TEXT("TAXI")},
        {5.02f, 72.0f,   20.0f, FLinearColor(0.92f, 0.30f, 0.06f, 1.0f), false, 2.7f, TEXT("DELIVERY VAN")},
        {1.64f, 50.0f,  175.0f, FLinearColor(0.18f, 0.66f, 0.30f, 1.0f), true,  3.1f, TEXT("UNCLE'S CAR")},
        {3.76f, 64.0f, -135.0f, FLinearColor(0.64f, 0.20f, 0.76f, 1.0f), false, 4.2f, TEXT("MICROBUS")},
        {5.86f, 54.0f,  285.0f, FLinearColor(0.12f, 0.68f, 0.72f, 1.0f), true,  5.4f, TEXT("LOST DRIVER")}
    };

    const int32 SpawnCount = FMath::Min(TrafficCount, static_cast<int32>(UE_ARRAY_COUNT(Seeds)));
    for (int32 Index = 0; Index < SpawnCount; ++Index)
    {
        const FTrafficSeed& Seed = Seeds[Index];
        const FTransform SpawnTransform = ARITrafficVehicle::MakeRouteTransform(Seed.Angle, Seed.LaneOffset);
        AActor* DeferredActor = UGameplayStatics::BeginDeferredActorSpawnFromClass(
            World,
            ARITrafficVehicle::StaticClass(),
            SpawnTransform,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
            nullptr);

        if (ARITrafficVehicle* Traffic = Cast<ARITrafficVehicle>(DeferredActor))
        {
            Traffic->Configure(
                Seed.Angle,
                Seed.SpeedKph,
                Seed.LaneOffset,
                Seed.Color,
                Seed.bWanders,
                Seed.WanderPhase,
                Seed.Label);
            UGameplayStatics::FinishSpawningActor(Traffic, SpawnTransform);
        }
    }

    bTrafficSpawned = true;
}

void URITrafficWorldSubsystem::Tick(float DeltaTime)
{
    TrySpawnTraffic();
}
