#include "Items/RIRottenEggWorldSubsystem.h"

#include "Items/RIBananaPickup.h"
#include "Items/RIRottenEggPickup.h"
#include "Vehicle/RIBikePawn.h"
#include "Core/RIParticipantComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

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

    if (Bananas.Num() < 3)
    {
        return;
    }

    Bananas.Sort([](const ARIBananaPickup& A, const ARIBananaPickup& B)
    {
        const FVector LA = A.GetActorLocation();
        const FVector LB = B.GetActorLocation();
        if (!FMath::IsNearlyEqual(LA.X, LB.X)) return LA.X < LB.X;
        return LA.Y < LB.Y;
    });

    const int32 DesiredPickups = FMath::Min(3, Bananas.Num());
    for (int32 Index = 0; Index < DesiredPickups; ++Index)
    {
        const int32 BananaIndex = FMath::FloorToInt((static_cast<float>(Index) + 0.5f) * Bananas.Num() / DesiredPickups);
        ARIBananaPickup* Anchor = Bananas[FMath::Clamp(BananaIndex, 0, Bananas.Num() - 1)];
        if (!Anchor) continue;

        const float SideSign = (Index % 2 == 0) ? 1.0f : -1.0f;
        const FVector SpawnLocation =
            Anchor->GetActorLocation() +
            Anchor->GetActorRightVector().GetSafeNormal2D() * (SideSign * 220.0f) +
            FVector::UpVector * 20.0f;

        World->SpawnActor<ARIRottenEggPickup>(
            ARIRottenEggPickup::StaticClass(),
            SpawnLocation,
            Anchor->GetActorRotation());
    }

    bSpawnedPickups = true;
}

void URIRottenEggWorldSubsystem::TryThrowEgg()
{
    UWorld* World = GetWorld();
    if (!World) return;

    const double Now = World->GetTimeSeconds();
    if (Now - LastThrowTime < 0.55)
    {
        return;
    }

    ARIBikePawn* Bike = FindHumanBike();
    if (!Bike) return;

    if (!Bike->ThrowRottenEggAt(nullptr))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Silver, TEXT("No rotten egg. Find the ugly green pickup first."));
        }
        return;
    }

    LastThrowTime = Now;
}

void URIRottenEggWorldSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    TrySpawnPickups();

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (PlayerController && PlayerController->WasInputKeyJustPressed(EKeys::G))
    {
        TryThrowEgg();
    }
}
