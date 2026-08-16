#include "World/RIWorldSignageSubsystem.h"

#include "Vehicle/RIBikePawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextRenderActor.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

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

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(
        nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    int32 SignCount = 0;
    int32 BoardCount = 0;
    auto SpawnTextSign = [World, CubeMesh, BaseMaterial, &SignCount, &BoardCount](
        const FString& TextValue,
        const FVector& Location,
        const FVector& FacingDirection,
        const FColor& Color,
        const float WorldSize,
        const float XScale,
        const float BoardWidth,
        const float BoardHeight,
        const FLinearColor& BoardColor)
    {
        FVector SafeFacing = FacingDirection.GetSafeNormal2D();
        if (SafeFacing.IsNearlyZero()) SafeFacing = FVector::ForwardVector;
        const FRotator FacingRotation = SafeFacing.Rotation();

        if (CubeMesh)
        {
            // Thin dark board sits just behind the text plane. It is intentionally
            // collision-free and casts no shadow so landmark text stays readable
            // without becoming another physical roadside obstacle.
            AStaticMeshActor* Board = World->SpawnActor<AStaticMeshActor>(
                Location - SafeFacing * 10.0f,
                FacingRotation);
            if (Board)
            {
                Board->SetActorEnableCollision(false);
                Board->SetActorScale3D(FVector(0.10f, BoardWidth / 100.0f, BoardHeight / 100.0f));
                if (UStaticMeshComponent* Mesh = Board->GetStaticMeshComponent())
                {
                    Mesh->SetMobility(EComponentMobility::Movable);
                    Mesh->SetStaticMesh(CubeMesh);
                    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                    Mesh->SetCollisionProfileName(TEXT("NoCollision"));
                    Mesh->SetGenerateOverlapEvents(false);
                    Mesh->SetCanEverAffectNavigation(false);
                    Mesh->SetCastShadow(false);
                    if (BaseMaterial)
                    {
                        if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Mesh))
                        {
                            Material->SetVectorParameterValue(TEXT("Color"), BoardColor);
                            Mesh->SetMaterial(0, Material);
                        }
                    }
                }
                ++BoardCount;
            }
        }

        ATextRenderActor* Sign = World->SpawnActor<ATextRenderActor>(
            Location + SafeFacing * 2.0f,
            FacingRotation);
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
            0.82f,
            760.0f,
            128.0f,
            FLinearColor(0.025f, 0.035f, 0.045f, 1.0f));
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
            0.90f,
            460.0f,
            116.0f,
            FLinearColor(0.085f, 0.060f, 0.025f, 1.0f));
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
            0.92f,
            470.0f,
            112.0f,
            FLinearColor(0.025f, 0.070f, 0.055f, 1.0f));
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI WORLD SIGNAGE signs=%d boards=%d collision=off assets=builtin_text"),
        SignCount,
        BoardCount);
}