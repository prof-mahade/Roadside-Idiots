#include "World/RIFreeVegetationSubsystem.h"

#include "Vehicle/RIBikePawn.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
    constexpr float RIVegRouteRadiusX = 9000.0f;
    constexpr float RIVegRouteRadiusY = 5000.0f;

    FVector RIVegRoutePoint(const float Angle)
    {
        return FVector(
            FMath::Cos(Angle) * RIVegRouteRadiusX,
            FMath::Sin(Angle) * RIVegRouteRadiusY,
            0.0f);
    }

    FVector RIVegOutward(const float Angle)
    {
        return RIVegRoutePoint(Angle).GetSafeNormal2D();
    }
}

bool URIFreeVegetationSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !bBuilt && !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIFreeVegetationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIFreeVegetationSubsystem, STATGROUP_Tickables);
}

void URIFreeVegetationSubsystem::Tick(const float DeltaTime)
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

    BuildVegetation();
    bBuilt = true;
}

void URIFreeVegetationSubsystem::BuildVegetation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UStaticMesh* TropicalA = LoadObject<UStaticMesh>(
        nullptr,
        TEXT("/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_01_04.tropicalPlant_01_04"));
    UStaticMesh* TropicalB = LoadObject<UStaticMesh>(
        nullptr,
        TEXT("/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_05_04.tropicalPlant_05_04"));
    UStaticMesh* BananaA = LoadObject<UStaticMesh>(
        nullptr,
        TEXT("/Game/PN_Banana/Meshes/plants/banana_01_07.banana_01_07"));
    UStaticMesh* BananaB = LoadObject<UStaticMesh>(
        nullptr,
        TEXT("/Game/PN_Banana/Meshes/plants/banana_02_05.banana_02_05"));

    const int32 TropicalAssetCount = (TropicalA ? 1 : 0) + (TropicalB ? 1 : 0);
    const int32 BananaAssetCount = (BananaA ? 1 : 0) + (BananaB ? 1 : 0);
    if (TropicalAssetCount == 0 && BananaAssetCount == 0)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("RI FREE VEGETATION skipped tropical_assets=0/2 banana_assets=0/2 reason=approved_assets_missing"));
        return;
    }

    AActor* RootActor = World->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator);
    if (!RootActor) return;
    RootActor->SetActorEnableCollision(false);

    USceneComponent* SceneRoot = NewObject<USceneComponent>(RootActor, TEXT("FreeVegetationRoot"));
    if (!SceneRoot)
    {
        RootActor->Destroy();
        return;
    }
    RootActor->AddInstanceComponent(SceneRoot);
    RootActor->SetRootComponent(SceneRoot);
    SceneRoot->SetMobility(EComponentMobility::Movable);
    SceneRoot->RegisterComponent();

    auto CreateLayer = [RootActor, SceneRoot](const FName Name, UStaticMesh* Mesh) -> UInstancedStaticMeshComponent*
    {
        if (!Mesh) return nullptr;

        UInstancedStaticMeshComponent* Layer = NewObject<UInstancedStaticMeshComponent>(RootActor, Name);
        if (!Layer) return nullptr;

        RootActor->AddInstanceComponent(Layer);
        Layer->SetupAttachment(SceneRoot);
        Layer->SetMobility(EComponentMobility::Movable);
        Layer->SetStaticMesh(Mesh);
        Layer->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Layer->SetCollisionProfileName(TEXT("NoCollision"));
        Layer->SetGenerateOverlapEvents(false);
        Layer->SetCanEverAffectNavigation(false);
        Layer->SetCastShadow(true);
        Layer->RegisterComponent();
        return Layer;
    };

    UInstancedStaticMeshComponent* TropicalLayerA = CreateLayer(TEXT("FreeTropicalA"), TropicalA);
    UInstancedStaticMeshComponent* TropicalLayerB = CreateLayer(TEXT("FreeTropicalB"), TropicalB);
    UInstancedStaticMeshComponent* BananaLayerA = CreateLayer(TEXT("FreeBananaA"), BananaA);
    UInstancedStaticMeshComponent* BananaLayerB = CreateLayer(TEXT("FreeBananaB"), BananaB);

    int32 TropicalCount = 0;
    int32 BananaCount = 0;

    // Small ground plants around the full lap. The closest instance remains more
    // than 11 m outside route center, comfortably beyond the ~6 m road half-width
    // and barrier presentation. Collision is disabled regardless.
    constexpr int32 TropicalSlots = 38;
    for (int32 Slot = 0; Slot < TropicalSlots; ++Slot)
    {
        UInstancedStaticMeshComponent* Layer =
            (Slot % 2 == 0) ? TropicalLayerA : TropicalLayerB;
        if (!Layer)
        {
            Layer = TropicalLayerA ? TropicalLayerA : TropicalLayerB;
        }
        if (!Layer) continue;

        const float Angle = 2.0f * PI * static_cast<float>(Slot) / static_cast<float>(TropicalSlots);
        const FVector Route = RIVegRoutePoint(Angle);
        const FVector Outward = RIVegOutward(Angle);
        const float Offset = 1120.0f + 120.0f * static_cast<float>((Slot * 7) % 5);
        const float Scale = 0.72f + 0.08f * static_cast<float>((Slot * 11) % 6);
        const float Yaw = static_cast<float>((Slot * 47) % 360);
        const FVector Location = Route + Outward * Offset;

        Layer->AddInstance(
            FTransform(
                FRotator(0.0f, Yaw, 0.0f),
                Location,
                FVector(Scale)),
            true);
        ++TropicalCount;
    }

    // Taller banana plants create recognisable clusters around the market/rural
    // half without enclosing the road in vegetation. They stay further away than
    // the ground plants so their larger silhouettes never obscure racing lines.
    constexpr int32 BananaSlots = 10;
    for (int32 Slot = 0; Slot < BananaSlots; ++Slot)
    {
        UInstancedStaticMeshComponent* Layer =
            (Slot % 2 == 0) ? BananaLayerA : BananaLayerB;
        if (!Layer)
        {
            Layer = BananaLayerA ? BananaLayerA : BananaLayerB;
        }
        if (!Layer) continue;

        const float T = static_cast<float>(Slot) / static_cast<float>(BananaSlots - 1);
        const float Angle = 0.52f * PI + T * 1.18f * PI;
        const FVector Route = RIVegRoutePoint(Angle);
        const FVector Outward = RIVegOutward(Angle);
        const float Offset = 1450.0f + 165.0f * static_cast<float>((Slot * 5) % 4);
        const float Scale = 0.80f + 0.07f * static_cast<float>((Slot * 3) % 5);
        const float Yaw = static_cast<float>((Slot * 73 + 19) % 360);
        const FVector Location = Route + Outward * Offset;

        Layer->AddInstance(
            FTransform(
                FRotator(0.0f, Yaw, 0.0f),
                Location,
                FVector(Scale)),
            true);
        ++BananaCount;
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI FREE VEGETATION tropical=%d banana=%d tropical_assets=%d/2 banana_assets=%d/2 collision=off navigation=off source=approved_free"),
        TropicalCount,
        BananaCount,
        TropicalAssetCount,
        BananaAssetCount);
}
