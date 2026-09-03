#include "World/RIRoadSurfaceDetailSubsystem.h"

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
    constexpr float RISurfaceRadiusX = 9000.0f;
    constexpr float RISurfaceRadiusY = 5000.0f;

    FVector RISurfacePoint(const float Angle)
    {
        return FVector(
            FMath::Cos(Angle) * RISurfaceRadiusX,
            FMath::Sin(Angle) * RISurfaceRadiusY,
            0.0f);
    }

    FVector RISurfaceForward(const float Angle)
    {
        return FVector(
            -FMath::Sin(Angle) * RISurfaceRadiusX,
            FMath::Cos(Angle) * RISurfaceRadiusY,
            0.0f).GetSafeNormal2D();
    }
}

bool URIRoadSurfaceDetailSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !bBuilt && !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIRoadSurfaceDetailSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIRoadSurfaceDetailSubsystem, STATGROUP_Tickables);
}

void URIRoadSurfaceDetailSubsystem::Tick(const float DeltaTime)
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

    BuildSurfaceDetails();
    bBuilt = true;
}

void URIRoadSurfaceDetailSubsystem::BuildSurfaceDetails()
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

    USceneComponent* SceneRoot = NewObject<USceneComponent>(RootActor, TEXT("RoadSurfaceDetailRoot"));
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

    UInstancedStaticMeshComponent* PatchLayer = CreateLayer(
        TEXT("AsphaltRepairPatches"),
        FLinearColor(0.24f, 0.25f, 0.26f, 1.0f));
    UInstancedStaticMeshComponent* SkidLayer = CreateLayer(
        TEXT("AsphaltSkidStreaks"),
        FLinearColor(0.075f, 0.078f, 0.080f, 1.0f));

    if (!PatchLayer || !SkidLayer)
    {
        RootActor->Destroy();
        return;
    }

    int32 PatchCount = 0;
    constexpr int32 DesiredPatches = 18;
    for (int32 Index = 0; Index < DesiredPatches; ++Index)
    {
        const float Angle = 2.0f * PI * (static_cast<float>(Index) + 0.37f) / static_cast<float>(DesiredPatches);
        const FVector Forward = RISurfaceForward(Angle);
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal2D();
        const FVector Center = RISurfacePoint(Angle);

        const float Lateral = FMath::Sin(static_cast<float>(Index) * 1.73f) * 285.0f;
        const float Length = 115.0f + 22.0f * static_cast<float>((Index * 7) % 5);
        const float Width = 34.0f + 12.0f * static_cast<float>((Index * 3) % 4);
        const float YawBias = FMath::Sin(static_cast<float>(Index) * 2.1f) * 5.0f;

        PatchLayer->AddInstance(
            FTransform(
                Forward.Rotation() + FRotator(0.0f, YawBias, 0.0f),
                Center + Right * Lateral + FVector::UpVector * 2.0f,
                FVector(Length / 100.0f, Width / 100.0f, 0.010f)),
            true);
        ++PatchCount;
    }

    int32 SkidCount = 0;
    const float BrakingZones[] = {0.28f * PI, 0.78f * PI, 1.28f * PI, 1.78f * PI};
    for (int32 Zone = 0; Zone < UE_ARRAY_COUNT(BrakingZones); ++Zone)
    {
        const float BaseAngle = BrakingZones[Zone];
        const float LaneBias = (Zone & 1) == 0 ? -185.0f : 185.0f;

        for (int32 Segment = 0; Segment < 5; ++Segment)
        {
            const float Angle = BaseAngle - 0.040f + static_cast<float>(Segment) * 0.010f;
            const FVector Forward = RISurfaceForward(Angle);
            const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal2D();
            const FVector Center = RISurfacePoint(Angle) + Right * LaneBias;
            const float Length = 105.0f + static_cast<float>(Segment) * 10.0f;

            for (const float PairOffset : {-31.0f, 31.0f})
            {
                SkidLayer->AddInstance(
                    FTransform(
                        Forward.Rotation(),
                        Center + Right * PairOffset + FVector::UpVector * 2.15f,
                        FVector(Length / 100.0f, 0.055f, 0.008f)),
                    true);
                ++SkidCount;
            }
        }
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI ROAD SURFACE_DETAIL patches=%d skid_streaks=%d collision=off navigation=off"),
        PatchCount,
        SkidCount);
}