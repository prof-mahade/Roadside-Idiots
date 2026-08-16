#include "World/RIRoadsideFacadeDetailSubsystem.h"

#include "Vehicle/RIBikePawn.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
    constexpr float RIFacadeRadiusX = 9000.0f;
    constexpr float RIFacadeRadiusY = 5000.0f;
    constexpr float RIFacadeRoadWidth = 1200.0f;

    FVector RIFacadeRouteCenter(const float Angle)
    {
        return FVector(
            FMath::Cos(Angle) * RIFacadeRadiusX,
            FMath::Sin(Angle) * RIFacadeRadiusY,
            0.0f);
    }

    FVector RIFacadeRouteForward(const float Angle)
    {
        return FVector(
            -FMath::Sin(Angle) * RIFacadeRadiusX,
            FMath::Cos(Angle) * RIFacadeRadiusY,
            0.0f).GetSafeNormal2D();
    }

    FVector RIFacadeOutward(const float Angle)
    {
        return RIFacadeRouteCenter(Angle).GetSafeNormal2D();
    }
}

bool URIRoadsideFacadeDetailSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !bBuilt && !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIRoadsideFacadeDetailSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIRoadsideFacadeDetailSubsystem, STATGROUP_Tickables);
}

void URIRoadsideFacadeDetailSubsystem::Tick(const float DeltaTime)
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

    BuildFacadeDetails();
    bBuilt = true;
}

void URIRoadsideFacadeDetailSubsystem::BuildFacadeDetails()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(
        nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!CubeMesh) return;

    AActor* RootActor = World->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator);
    if (!RootActor) return;
    RootActor->SetActorEnableCollision(false);

    USceneComponent* SceneRoot = NewObject<USceneComponent>(RootActor, TEXT("RoadsideFacadeDetailRoot"));
    if (!SceneRoot)
    {
        RootActor->Destroy();
        return;
    }
    RootActor->AddInstanceComponent(SceneRoot);
    RootActor->SetRootComponent(SceneRoot);
    SceneRoot->SetMobility(EComponentMobility::Movable);
    SceneRoot->RegisterComponent();

    auto CreateLayer = [RootActor, SceneRoot, CubeMesh, BaseMaterial](
        const FName Name,
        const FLinearColor& Color) -> UInstancedStaticMeshComponent*
    {
        UInstancedStaticMeshComponent* Layer = NewObject<UInstancedStaticMeshComponent>(RootActor, Name);
        if (!Layer) return nullptr;

        RootActor->AddInstanceComponent(Layer);
        Layer->SetupAttachment(SceneRoot);
        Layer->SetMobility(EComponentMobility::Movable);
        Layer->SetStaticMesh(CubeMesh);
        Layer->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Layer->SetCollisionProfileName(TEXT("NoCollision"));
        Layer->SetGenerateOverlapEvents(false);
        Layer->SetCanEverAffectNavigation(false);
        Layer->SetCastShadow(false);
        Layer->RegisterComponent();

        if (BaseMaterial)
        {
            if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Layer))
            {
                Material->SetVectorParameterValue(TEXT("Color"), Color);
                Layer->SetMaterial(0, Material);
            }
        }
        return Layer;
    };

    UInstancedStaticMeshComponent* GlassLayer = CreateLayer(
        TEXT("FacadeDarkGlass"),
        FLinearColor(0.035f, 0.070f, 0.085f, 1.0f));
    UInstancedStaticMeshComponent* DoorLayer = CreateLayer(
        TEXT("FacadeDoors"),
        FLinearColor(0.18f, 0.10f, 0.055f, 1.0f));
    UInstancedStaticMeshComponent* AwningLayer = CreateLayer(
        TEXT("FacadeAwningEdges"),
        FLinearColor(0.76f, 0.54f, 0.12f, 1.0f));
    UInstancedStaticMeshComponent* TrimLayer = CreateLayer(
        TEXT("FacadeDarkTrim"),
        FLinearColor(0.075f, 0.085f, 0.090f, 1.0f));

    if (!GlassLayer || !DoorLayer || !AwningLayer || !TrimLayer)
    {
        RootActor->Destroy();
        return;
    }

    int32 ShopPanels = 0;
    {
        const float Angle = PI * 0.66f;
        const FVector Route = RIFacadeRouteCenter(Angle);
        const FVector Forward = RIFacadeRouteForward(Angle);
        const FVector Outward = RIFacadeOutward(Angle);
        const FVector RoadFacing = -Outward;
        const FRotator Rotation = Forward.Rotation();
        const FVector MarketCenter = Route + Outward * (RIFacadeRoadWidth * 0.5f + 1850.0f);

        for (int32 Shop = 0; Shop < 6; ++Shop)
        {
            const float Along = (static_cast<float>(Shop) - 2.5f) * 455.0f;
            const FVector ShopCenter = MarketCenter + Forward * Along;
            const FVector Facade = ShopCenter + RoadFacing * 232.0f;

            DoorLayer->AddInstance(
                FTransform(
                    Rotation,
                    Facade - Forward * 92.0f + FVector::UpVector * 108.0f,
                    FVector(0.88f, 0.035f, 1.10f)),
                true);
            ++ShopPanels;

            GlassLayer->AddInstance(
                FTransform(
                    Rotation,
                    Facade + Forward * 92.0f + FVector::UpVector * 142.0f,
                    FVector(0.90f, 0.032f, 0.66f)),
                true);
            ++ShopPanels;

            // A narrow valance at the front lip makes the existing awning read as
            // intentional shop architecture rather than a floating roof cube.
            AwningLayer->AddInstance(
                FTransform(
                    Rotation,
                    ShopCenter + RoadFacing * 318.0f + FVector::UpVector * 204.0f,
                    FVector(2.95f, 0.032f, 0.11f)),
                true);
            ++ShopPanels;

            // Small vertical separator between storefront bays.
            TrimLayer->AddInstance(
                FTransform(
                    Rotation,
                    Facade + FVector::UpVector * 128.0f,
                    FVector(0.055f, 0.040f, 1.32f)),
                true);
            ++ShopPanels;
        }
    }

    int32 BusWindows = 0;
    {
        const float Angle = PI * 1.28f;
        const FVector Route = RIFacadeRouteCenter(Angle);
        const FVector Forward = RIFacadeRouteForward(Angle);
        const FVector Outward = RIFacadeOutward(Angle);
        const FVector RoadFacing = -Outward;
        const FRotator Rotation = Forward.Rotation();

        const FVector Bus = Route + Outward * (RIFacadeRoadWidth * 0.5f + 1160.0f) - Forward * 860.0f;
        for (int32 Window = 0; Window < 5; ++Window)
        {
            const float Along = (static_cast<float>(Window) - 2.0f) * 132.0f;
            GlassLayer->AddInstance(
                FTransform(
                    Rotation,
                    Bus + Forward * Along + RoadFacing * 150.0f + FVector::UpVector * 190.0f,
                    FVector(0.52f, 0.030f, 0.34f)),
                true);
            ++BusWindows;
        }

        // Door outline and lower belt trim on the road-facing side.
        TrimLayer->AddInstance(
            FTransform(
                Rotation,
                Bus + Forward * 365.0f + RoadFacing * 151.0f + FVector::UpVector * 126.0f,
                FVector(0.035f, 0.035f, 0.92f)),
            true);
        TrimLayer->AddInstance(
            FTransform(
                Rotation,
                Bus + RoadFacing * 151.0f + FVector::UpVector * 118.0f,
                FVector(4.75f, 0.030f, 0.055f)),
            true);
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI WORLD FACADE_DETAILS shop_panels=%d bus_windows=%d collision=off navigation=off"),
        ShopPanels,
        BusWindows);
}