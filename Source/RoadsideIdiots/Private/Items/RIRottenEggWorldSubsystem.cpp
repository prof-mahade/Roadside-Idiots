#include "Items/RIRottenEggWorldSubsystem.h"

#include "Items/RIBananaPickup.h"
#include "Items/RIRottenEggPickup.h"
#include "Vehicle/RIBikePawn.h"
#include "Core/RIParticipantComponent.h"
#include "EngineUtils.h"

bool URIRottenEggWorldSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIRottenEggWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIRottenEggWorldSubsystem, STATGROUP_Tickables);
}

ARIBikePawn* URIRottenEggWorldSubsystem::FindHumanBike() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    for (TActorIterator<ARIBikePawn> It(World); It; ++It)
    {
        ARIBikePawn* Bike = *It;
        const URIParticipantComponent* Participant = Bike ? Bike->GetParticipantComponent() : nullptr;
        if (Bike && Participant && Participant->IsHumanControlled())
        {
            return Bike;
        }
    }
    return nullptr;
}

int32 URIRottenEggWorldSubsystem::GetEggCount() const
{
    const ARIBikePawn* Bike = FindHumanBike();
    return Bike ? Bike->GetRottenEggCount() : 0;
}

int32 URIRottenEggWorldSubsystem::GetMaxEggCount() const
{
    const ARIBikePawn* Bike = FindHumanBike();
    return Bike ? Bike->GetMaxRottenEggs() : 2;
}

void URIRottenEggWorldSubsystem::TrySpawnPickups()
{
    if (bSpawnedPickups) return;

    UWorld* World = GetWorld();
    if (!World) return;

    TArray<ARIBananaPickup*> Bananas;
    for (TActorIterator<ARIBananaPickup> It(World); It; ++It)
    {
        if (*It)
        {
            Bananas.Add(*It);
        }
    }

    // The world builder creates eight pickup slots. Wait until they exist so we
    // can deterministically convert three of those slots into rotten eggs.
    // This keeps pickup density unchanged while preventing the old banana-only
    // item economy seen in telemetry.
    if (Bananas.Num() < 8)
    {
        return;
    }

    Bananas.Sort([](const ARIBananaPickup& A, const ARIBananaPickup& B)
    {
        const float AngleA = FMath::Atan2(A.GetActorLocation().Y, A.GetActorLocation().X);
        const float AngleB = FMath::Atan2(B.GetActorLocation().Y, B.GetActorLocation().X);
        return AngleA < AngleB;
    });

    constexpr int32 DesiredEggPickups = 3;
    int32 EggsSpawned = 0;

    for (int32 Index = 0; Index < DesiredEggPickups; ++Index)
    {
        const int32 BananaIndex = FMath::FloorToInt(
            (static_cast<float>(Index) + 0.5f) *
            static_cast<float>(Bananas.Num()) /
            static_cast<float>(DesiredEggPickups));

        ARIBananaPickup* Anchor = Bananas[FMath::Clamp(BananaIndex, 0, Bananas.Num() - 1)];
        if (!Anchor) continue;

        const FVector SpawnLocation = Anchor->GetActorLocation();
        const FRotator SpawnRotation = Anchor->GetActorRotation();

        if (World->SpawnActor<ARIRottenEggPickup>(
            ARIRottenEggPickup::StaticClass(),
            SpawnLocation,
            SpawnRotation))
        {
            // Replace rather than add beside the banana. The old side-offset egg
            // pickups were easy to miss and increased total pickup clutter.
            Anchor->Destroy();
            ++EggsSpawned;
        }
    }

    if (EggsSpawned == DesiredEggPickups)
    {
        bSpawnedPickups = true;
        UE_LOG(
            LogTemp,
            Display,
            TEXT("RI ITEMS BALANCE banana_slots=5 egg_slots=3 input_owner=bike"));
    }
}

void URIRottenEggWorldSubsystem::Tick(float DeltaTime)
{
    TrySpawnPickups();
}
