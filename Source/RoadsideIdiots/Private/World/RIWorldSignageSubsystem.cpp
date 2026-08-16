#include "World/RIWorldSignageSubsystem.h"

#include "Vehicle/RIBikePawn.h"
#include "Components/TextRenderComponent.h"
#include "Engine/TextRenderActor.h"
#include "EngineUtils.h"

namespace
{
    constexpr float RISignRadiusX = 9000.0f;
    constexpr float RISignRadiusY = 5000.0f;
    constexpr float RISignRoadWidth = 1200.0f;

    FVector RISignRouteCenter(const float Angle)
    {
        return FVector(
            FMath::Cos(Angle) * RISignRadiusX,
            FMath::Sin(Angle) * RISignRadiusY,
            0.0f);
    }

    FVector RISignRouteForward(const float Angle)
    {
        return FVector(
            -FMath::Sin(Angle) * RISignRadiusX,
            FMath::Cos(Angle) * RISignRadiusY,
            0.0f).GetSafeNormal2D();
    }

    FVector RISignRouteOutward(const float Angle)
    {
        return RISignRouteCenter(Angle).GetSafeNormal2D();
    }
}

bool URIWorldSignageSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !bBuilt && !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIWorldSignageSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIWorldSignageSubsystem, STATGROUP_Tickables);
}

void URIWorldSignageSubsystem::Tick(const float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || bBuilt) return;

    bool bRaceWorldExists = false;
    for (TActorIterator<ARIBikePawn> It(World); It; ++It)
    {
        if (*It)
        {
            bRaceWorldExists = true;
            break;
        }
    }
    if (!bRaceWorldExists) return;

    BuildSigns();
    bBuilt = true;
}

void URIWorldSignageSubsystem::BuildSigns()
{
    UWorld* World = GetWorld();
    if (!World) return;

    int32 SignCount = 0;
    auto SpawnTextSign = [World, &SignCount](
        const FString& TextValue,
        const FVector& Location,
        const FVector& FacingDirection,
        const FColor& Color,
        const float WorldSize,
        const float XScale = 1.0f)
    {
        FVector SafeFacing = FacingDirection.GetSafeNormal2D();
        if (SafeFacing.IsNearlyZero()) SafeFacing = FVector::ForwardVector;

        ATextRenderActor* Sign = World->SpawnActor<ATextRenderActor>(Location, SafeFacing.Rotation());
        if (!Sign) return;

        Sign->SetActorEnableCollision(false);
        UTextRenderComponent* Text = Sign->GetTextRender();
        if (!Text)
        {
            Sign->Destroy();
            return;
        }

        Text->SetText(FText::FromString(TextValue));
        Text->SetHorizontalAlignment(EHTA_Center);
        Text->SetWorldSize(WorldSize);
        Text->SetXScale(XScale);
        Text->SetTextRenderColor(Color);
        Text->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Text->SetGenerateOverlapEvents(false);
        Text->SetCanEverAffectNavigation(false);
        Text->SetCastShadow(false);
        ++SignCount;
    };

    // Start gantry branding faces against race direction so it reads as the
    // player approaches the line at the end of each lap.
    {
        constexpr float Angle = 0.0f;
        const FVector Center = RISignRouteCenter(Angle);
        const FVector Forward = RISignRouteForward(Angle);
        SpawnTextSign(
            TEXT("ROADSIDE IDIOTS"),
            Center + FVector::UpVector * 575.0f,
            -Forward,
            FColor(255, 198, 45),
            82.0f,
            0.82f);
    }

    // Market label faces inward toward the road and sits above the colorful shop
    // cluster, making that section readable before the individual props resolve.
    {
        constexpr float Angle = PI * 0.66f;
        const FVector Route = RISignRouteCenter(Angle);
        const FVector Outward = RISignRouteOutward(Angle);
        SpawnTextSign(
            TEXT("TEA STOP"),
            Route + Outward * (RISignRoadWidth * 0.5f + 1500.0f) + FVector::UpVector * 410.0f,
            -Outward,
            FColor(255, 225, 92),
            76.0f,
            0.90f);
    }

    // Bus-stop text sits near the shelter rather than on the roadway. It exists
    // purely as a landmark; the bus/stop geometry remains non-colliding too.
    {
        constexpr float Angle = PI * 1.28f;
        const FVector Route = RISignRouteCenter(Angle);
        const FVector Outward = RISignRouteOutward(Angle);
        SpawnTextSign(
            TEXT("BUS STOP"),
            Route + Outward * (RISignRoadWidth * 0.5f + 1120.0f) + FVector::UpVector * 390.0f,
            -Outward,
            FColor(120, 255, 185),
            72.0f,
            0.92f);
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI WORLD SIGNAGE signs=%d collision=off assets=builtin_text"),
        SignCount);
}
