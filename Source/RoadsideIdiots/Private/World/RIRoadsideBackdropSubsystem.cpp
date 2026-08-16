#include "World/RIRoadsideBackdropSubsystem.h"

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
    constexpr float RIBackdropRadiusX = 13800.0f;
    constexpr float RIBackdropRadiusY = 8600.0f;

    FVector RIBackdropPoint(const float Angle)
    {
        return FVector(
            FMath::Cos(Angle) * RIBackdropRadiusX,
            FMath::Sin(Angle) * RIBackdropRadiusY,
            0.0f);
    }

    FVector RIBackdropTangent(const float Angle)
    {
        return FVector(
            -FMath::Sin(Angle) * RIBackdropRadiusX,
            FMath::Cos(Angle) * RIBackdropRadiusY,
            0.0f).GetSafeNormal2D();
    }
}

bool URIRoadsideBackdropSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !bBuilt && !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIRoadsideBackdropSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIRoadsideBackdropSubsystem, STATGROUP_Tickables);
}

void URIRoadsideBackdropSubsystem::Tick(const float DeltaTime)
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

    BuildBackdrop();
    bBuilt = true;
}

void URIRoadsideBackdropSubsystem::BuildBackdrop()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(
        nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!CubeMesh) return;

    AActor* RootActor = World->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator);
    if (!RootActor) return;
    RootActor->SetActorEnableCollision(false);

    USceneComponent* SceneRoot = NewObject<USceneComponent>(RootActor, TEXT("RoadsideBackdropRoot"));
    if (!SceneRoot)
    {
        RootActor->Destroy();
        return;
    }
    RootActor->AddInstanceComponent(SceneRoot);
    RootActor->SetRootComponent(SceneRoot);
    SceneRoot->SetMobility(EComponentMobility::Movable);
    SceneRoot->RegisterComponent();

    auto CreateLayer = [RootActor, SceneRoot, BaseMaterial](
        const FName Name,
        UStaticMesh* MeshAsset,
        const FLinearColor& Color) -> UInstancedStaticMeshComponent*
    {
        if (!MeshAsset) return nullptr;

        UInstancedStaticMeshComponent* Layer = NewObject<UInstancedStaticMeshComponent>(RootActor, Name);
        if (!Layer) return nullptr;

        RootActor->AddInstanceComponent(Layer);
        Layer->SetupAttachment(SceneRoot);
        Layer->SetMobility(EComponentMobility::Movable);
        Layer->SetStaticMesh(MeshAsset);
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

    UInstancedStaticMeshComponent* WarmBuildings = CreateLayer(
        TEXT("BackdropWarmBuildings"),
        CubeMesh,
        FLinearColor(0.44f, 0.31f, 0.21f, 1.0f));
    UInstancedStaticMeshComponent* CoolBuildings = CreateLayer(
        TEXT("BackdropCoolBuildings"),
        CubeMesh,
        FLinearColor(0.28f, 0.34f, 0.37f, 1.0f));
    UInstancedStaticMeshComponent* LightBuildings = CreateLayer(
        TEXT("BackdropLightBuildings"),
        CubeMesh,
        FLinearColor(0.53f, 0.50f, 0.40f, 1.0f));
    UInstancedStaticMeshComponent* TreeTrunks = CreateLayer(
        TEXT("BackdropTreeTrunks"),
        CubeMesh,
        FLinearColor(0.20f, 0.12f, 0.06f, 1.0f));
    UInstancedStaticMeshComponent* TreeCanopies = CreateLayer(
        TEXT("BackdropTreeCanopies"),
        SphereMesh,
        FLinearColor(0.075f, 0.25f, 0.075f, 1.0f));
    UInstancedStaticMeshComponent* WaterTanks = CreateLayer(
        TEXT("BackdropWaterTanks"),
        CylinderMesh,
        FLinearColor(0.085f, 0.095f, 0.105f, 1.0f));

    if (!WarmBuildings || !CoolBuildings || !LightBuildings || !TreeTrunks || !TreeCanopies)
    {
        RootActor->Destroy();
        return;
    }

    int32 BuildingCount = 0;
    int32 TreeCount = 0;
    int32 TankCount = 0;

    // More built-up backdrop on roughly half the lap. Heights and widths use a
    // deterministic pattern so the skyline is varied but stable between runs.
    constexpr int32 BuildingSlots = 22;
    for (int32 Slot = 0; Slot < BuildingSlots; ++Slot)
    {
        const float T = static_cast<float>(Slot) / static_cast<float>(BuildingSlots - 1);
        const float Angle = 0.08f * PI + T * 0.98f * PI;
        const FVector Base = RIBackdropPoint(Angle);
        const FVector Forward = RIBackdropTangent(Angle);
        const FRotator Rotation = Forward.Rotation();

        const float Width = 2.8f + 0.55f * static_cast<float>((Slot * 7) % 5);
        const float Depth = 2.2f + 0.40f * static_cast<float>((Slot * 3) % 4);
        const float Height = 2.4f + 0.62f * static_cast<float>((Slot * 5) % 6);
        const float AlongJitter = static_cast<float>((Slot % 3) - 1) * 145.0f;
        const FVector Center = Base + Forward * AlongJitter + FVector::UpVector * (Height * 50.0f);

        UInstancedStaticMeshComponent* Layer = nullptr;
        switch (Slot % 3)
        {
        case 0: Layer = WarmBuildings; break;
        case 1: Layer = CoolBuildings; break;
        default: Layer = LightBuildings; break;
        }

        Layer->AddInstance(FTransform(Rotation, Center, FVector(Width, Depth, Height)), true);
        ++BuildingCount;

        // Occasional dark rooftop tanks add a familiar low-rise rooftop rhythm
        // without introducing detailed cultural props or extra asset dependencies.
        if (WaterTanks && (Slot % 4) == 1)
        {
            const FVector TankCenter = Center + FVector::UpVector * (Height * 50.0f + 52.0f);
            WaterTanks->AddInstance(
                FTransform(
                    FRotator::ZeroRotator,
                    TankCenter,
                    FVector(0.40f, 0.40f, 0.70f)),
                true);
            ++TankCount;
        }
    }

    // Rural half of the course gets a broad irregular tree belt rather than a
    // second building wall. These are deliberately distant and low-contrast.
    constexpr int32 TreeSlots = 30;
    for (int32 Slot = 0; Slot < TreeSlots; ++Slot)
    {
        const float T = static_cast<float>(Slot) / static_cast<float>(TreeSlots - 1);
        const float Angle = 1.07f * PI + T * 0.93f * PI;
        FVector Base = RIBackdropPoint(Angle);
        const FVector Forward = RIBackdropTangent(Angle);

        const float AlongJitter = static_cast<float>((Slot % 5) - 2) * 95.0f;
        const float HeightScale = 0.82f + 0.08f * static_cast<float>((Slot * 5) % 5);
        Base += Forward * AlongJitter;

        TreeTrunks->AddInstance(
            FTransform(
                FRotator::ZeroRotator,
                Base + FVector::UpVector * (125.0f * HeightScale),
                FVector(0.15f, 0.15f, 2.50f * HeightScale)),
            true);

        TreeCanopies->AddInstance(
            FTransform(
                FRotator(0.0f, static_cast<float>((Slot * 29) % 360), 0.0f),
                Base + FVector::UpVector * (295.0f * HeightScale),
                FVector(1.55f, 1.30f, 0.90f) * HeightScale),
            true);
        ++TreeCount;
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI WORLD BACKDROP buildings=%d trees=%d tanks=%d collision=off navigation=off"),
        BuildingCount,
        TreeCount,
        TankCount);
}