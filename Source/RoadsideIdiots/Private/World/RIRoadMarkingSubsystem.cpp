#include "World/RIRoadMarkingSubsystem.h"

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
    constexpr int32 RIMarkingPointCount = 80;
    constexpr float RIMarkingRadiusX = 9000.0f;
    constexpr float RIMarkingRadiusY = 5000.0f;
    constexpr float RIMarkingRoadWidth = 1200.0f;

    FVector RIMarkingRoutePoint(const int32 Index)
    {
        const float Angle = 2.0f * PI * static_cast<float>(Index) / static_cast<float>(RIMarkingPointCount);
        return FVector(
            FMath::Cos(Angle) * RIMarkingRadiusX,
            FMath::Sin(Angle) * RIMarkingRadiusY,
            0.0f);
    }
}

bool URIRoadMarkingSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !bBuilt && !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIRoadMarkingSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIRoadMarkingSubsystem, STATGROUP_Tickables);
}

void URIRoadMarkingSubsystem::Tick(const float DeltaTime)
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

    BuildMarkings();
    bBuilt = true;
}

void URIRoadMarkingSubsystem::BuildMarkings()
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

    USceneComponent* SceneRoot = NewObject<USceneComponent>(RootActor, TEXT("RoadMarkingRoot"));
    if (!SceneRoot) return;
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

    UInstancedStaticMeshComponent* EdgeLayer = CreateLayer(
        TEXT("RoadEdgeLines"),
        FLinearColor(0.91f, 0.93f, 0.88f, 1.0f));
    UInstancedStaticMeshComponent* CenterLayer = CreateLayer(
        TEXT("RoadCenterDashes"),
        FLinearColor(0.94f, 0.72f, 0.12f, 1.0f));
    UInstancedStaticMeshComponent* CheckerLightLayer = CreateLayer(
        TEXT("StartCheckerLight"),
        FLinearColor(0.94f, 0.95f, 0.96f, 1.0f));
    UInstancedStaticMeshComponent* CheckerDarkLayer = CreateLayer(
        TEXT("StartCheckerDark"),
        FLinearColor(0.055f, 0.062f, 0.070f, 1.0f));

    if (!EdgeLayer || !CenterLayer || !CheckerLightLayer || !CheckerDarkLayer)
    {
        RootActor->Destroy();
        return;
    }

    int32 EdgeInstances = 0;
    int32 DashInstances = 0;

    for (int32 Index = 0; Index < RIMarkingPointCount; ++Index)
    {
        const FVector A = RIMarkingRoutePoint(Index);
        const FVector B = RIMarkingRoutePoint((Index + 1) % RIMarkingPointCount);
        FVector Direction = B - A;
        Direction.Z = 0.0f;
        const float Length = Direction.Size();
        if (Length < 1.0f) continue;

        const FVector Forward = Direction / Length;
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal2D();
        const FVector Center = (A + B) * 0.5f;
        const FRotator Rotation = Forward.Rotation();
        constexpr float EdgeOffset = RIMarkingRoadWidth * 0.5f - 82.0f;

        for (const float Side : {-1.0f, 1.0f})
        {
            const FVector Location = Center + Right * (Side * EdgeOffset) + FVector::UpVector * 2.1f;
            EdgeLayer->AddInstance(
                FTransform(
                    Rotation,
                    Location,
                    FVector((Length + 26.0f) / 100.0f, 0.065f, 0.012f)),
                true);
            ++EdgeInstances;
        }

        // Long enough to read at speed but with regular gaps so the center line
        // becomes a strong motion/speed cue rather than a solid visual rail.
        if ((Index & 1) == 0)
        {
            CenterLayer->AddInstance(
                FTransform(
                    Rotation,
                    Center + FVector::UpVector * 2.2f,
                    FVector((Length * 0.58f) / 100.0f, 0.105f, 0.013f)),
                true);
            ++DashInstances;
        }
    }

    // Correctly visible start/finish checker. It sits just above the visual road
    // and is fully non-colliding, so it cannot recreate the old seam/bump issue.
    const FVector StartCenter = RIMarkingRoutePoint(0);
    FVector StartDirection = RIMarkingRoutePoint(1) - RIMarkingRoutePoint(RIMarkingPointCount - 1);
    StartDirection.Z = 0.0f;
    const FVector StartForward = StartDirection.GetSafeNormal2D();
    const FVector StartRight = FVector::CrossProduct(FVector::UpVector, StartForward).GetSafeNormal2D();
    const FRotator StartRotation = StartForward.Rotation();

    constexpr int32 CheckerColumns = 12;
    constexpr int32 CheckerRows = 2;
    const float TileWidth = RIMarkingRoadWidth / static_cast<float>(CheckerColumns);
    int32 CheckerInstances = 0;

    for (int32 Row = 0; Row < CheckerRows; ++Row)
    {
        for (int32 Column = 0; Column < CheckerColumns; ++Column)
        {
            const float Across = -RIMarkingRoadWidth * 0.5f + (static_cast<float>(Column) + 0.5f) * TileWidth;
            const float Along = (static_cast<float>(Row) - 0.5f) * 50.0f;
            UInstancedStaticMeshComponent* Layer = ((Column + Row) & 1) == 0
                ? CheckerLightLayer
                : CheckerDarkLayer;

            Layer->AddInstance(
                FTransform(
                    StartRotation,
                    StartCenter + StartRight * Across + StartForward * Along + FVector::UpVector * 2.4f,
                    FVector(0.46f, (TileWidth / 100.0f) * 0.94f, 0.014f)),
                true);
            ++CheckerInstances;
        }
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI ROAD MARKINGS edges=%d dashes=%d checker=%d collision=off navigation=off"),
        EdgeInstances,
        DashInstances,
        CheckerInstances);
}
