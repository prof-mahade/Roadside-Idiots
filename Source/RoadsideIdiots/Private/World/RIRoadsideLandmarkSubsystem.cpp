#include "World/RIRoadsideLandmarkSubsystem.h"

#include "Vehicle/RIBikePawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
    constexpr float RIRouteRadiusX = 9000.0f;
    constexpr float RIRouteRadiusY = 5000.0f;
    constexpr float RIRoadWidth = 1200.0f;

    FVector RIRouteCenter(const float Angle)
    {
        return FVector(
            FMath::Cos(Angle) * RIRouteRadiusX,
            FMath::Sin(Angle) * RIRouteRadiusY,
            0.0f);
    }

    FVector RIRouteForward(const float Angle)
    {
        return FVector(
            -FMath::Sin(Angle) * RIRouteRadiusX,
            FMath::Cos(Angle) * RIRouteRadiusY,
            0.0f).GetSafeNormal2D();
    }

    FVector RIRouteRight(const float Angle)
    {
        return FVector::CrossProduct(FVector::UpVector, RIRouteForward(Angle)).GetSafeNormal2D();
    }

    FVector RIRouteOutward(const float Angle)
    {
        return RIRouteCenter(Angle).GetSafeNormal2D();
    }
}

bool URIRoadsideLandmarkSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !bBuilt && !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIRoadsideLandmarkSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIRoadsideLandmarkSubsystem, STATGROUP_Tickables);
}

void URIRoadsideLandmarkSubsystem::Tick(const float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || bBuilt) return;

    // The configured race world is created only after the player leaves the setup
    // screen. Waiting for a bike keeps the setup menu world clean and guarantees
    // the authoritative course already exists before this visual-only layer.
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

    BuildLandmarks();
    bBuilt = true;
}

AStaticMeshActor* URIRoadsideLandmarkSubsystem::SpawnDecoration(
    UStaticMesh* MeshAsset,
    const FVector& Location,
    const FRotator& Rotation,
    const FVector& Scale,
    const FLinearColor& Color)
{
    UWorld* World = GetWorld();
    if (!World || !MeshAsset) return nullptr;

    AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (!Actor) return nullptr;

    Actor->SetActorEnableCollision(false);
    Actor->SetActorScale3D(Scale);

    UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
    if (!Mesh) return Actor;

    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetStaticMesh(MeshAsset);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->SetCollisionProfileName(TEXT("NoCollision"));
    Mesh->SetGenerateOverlapEvents(false);

    if (BaseMaterial)
    {
        if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Mesh))
        {
            Material->SetVectorParameterValue(TEXT("Color"), Color);
            Mesh->SetMaterial(0, Material);
        }
    }

    return Actor;
}

void URIRoadsideLandmarkSubsystem::BuildLandmarks()
{
    CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    BaseMaterial = LoadObject<UMaterialInterface>(
        nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    if (!CubeMesh) return;

    BuildStartFinish();
    BuildQuarterMarkers();
    BuildMarketCluster();
    BuildBusStopCluster();
    BuildPondAndFields();

    UE_LOG(
        LogTemp,
        Display,
        TEXT("RI WORLD LANDMARKS start_finish=1 quarter_markers=3 market=1 bus_stop=1 pond_fields=1 collision=off"));
}

void URIRoadsideLandmarkSubsystem::BuildStartFinish()
{
    constexpr float Angle = 0.0f;
    const FVector Center = RIRouteCenter(Angle);
    const FVector Forward = RIRouteForward(Angle);
    const FVector Right = RIRouteRight(Angle);
    const FRotator ForwardRotation = Forward.Rotation();
    const FRotator CrossRoadRotation = Right.Rotation();

    const FLinearColor Dark(0.045f, 0.055f, 0.065f, 1.0f);
    const FLinearColor Yellow(0.98f, 0.69f, 0.06f, 1.0f);
    const FLinearColor Teal(0.08f, 0.62f, 0.60f, 1.0f);
    const FLinearColor White(0.92f, 0.94f, 0.96f, 1.0f);

    // A two-row checkered start/finish stripe gives the player an immediate lap
    // landmark without introducing a physical seam in the accepted road surface.
    constexpr int32 TileCount = 10;
    const float TileWidth = RIRoadWidth / static_cast<float>(TileCount);
    for (int32 Row = 0; Row < 2; ++Row)
    {
        for (int32 Tile = 0; Tile < TileCount; ++Tile)
        {
            const float Across = -RIRoadWidth * 0.5f + (static_cast<float>(Tile) + 0.5f) * TileWidth;
            const float Along = (static_cast<float>(Row) - 0.5f) * 54.0f;
            const bool bBright = ((Tile + Row) & 1) == 0;
            SpawnDecoration(
                CubeMesh,
                Center + Right * Across + Forward * Along + FVector(0.0f, 0.0f, -6.0f),
                ForwardRotation,
                FVector(0.50f, TileWidth / 100.0f * 0.94f, 0.018f),
                bBright ? White : Dark);
        }
    }

    const float PostOffset = RIRoadWidth * 0.5f + 185.0f;
    for (const float Side : {-1.0f, 1.0f})
    {
        const FVector PostBase = Center + Right * (Side * PostOffset);
        SpawnDecoration(
            CubeMesh,
            PostBase + FVector::UpVector * 255.0f,
            FRotator::ZeroRotator,
            FVector(0.20f, 0.20f, 5.10f),
            Dark);
        SpawnDecoration(
            CubeMesh,
            PostBase + FVector::UpVector * 105.0f,
            FRotator::ZeroRotator,
            FVector(0.34f, 0.34f, 0.28f),
            Side < 0.0f ? Yellow : Teal);
    }

    SpawnDecoration(
        CubeMesh,
        Center + FVector::UpVector * 510.0f,
        CrossRoadRotation,
        FVector((RIRoadWidth + 420.0f) / 100.0f, 0.22f, 0.22f),
        Dark);

    // Large color blocks are readable from racing distance even without relying
    // on a font asset in the packaged build.
    for (int32 Panel = -2; Panel <= 2; ++Panel)
    {
        SpawnDecoration(
            CubeMesh,
            Center + Right * (static_cast<float>(Panel) * 245.0f) + FVector::UpVector * 510.0f,
            CrossRoadRotation,
            FVector(2.05f, 0.30f, 0.46f),
            (Panel & 1) == 0 ? Yellow : Teal);
    }
}

void URIRoadsideLandmarkSubsystem::BuildQuarterMarkers()
{
    const float Angles[] = {0.5f * PI, PI, 1.5f * PI};
    const FLinearColor Colors[] = {
        FLinearColor(0.15f, 0.60f, 0.92f, 1.0f),
        FLinearColor(0.94f, 0.35f, 0.12f, 1.0f),
        FLinearColor(0.28f, 0.72f, 0.28f, 1.0f)};
    const FLinearColor PoleColor(0.10f, 0.11f, 0.12f, 1.0f);

    for (int32 Marker = 0; Marker < 3; ++Marker)
    {
        const float Angle = Angles[Marker];
        const FVector Route = RIRouteCenter(Angle);
        const FVector Forward = RIRouteForward(Angle);
        const FVector Outward = RIRouteOutward(Angle);
        const FRotator Rotation = Forward.Rotation();
        const FVector Base = Route + Outward * (RIRoadWidth * 0.5f + 980.0f);

        for (const float Along : {-190.0f, 190.0f})
        {
            SpawnDecoration(
                CubeMesh,
                Base + Forward * Along + FVector::UpVector * 205.0f,
                FRotator::ZeroRotator,
                FVector(0.14f, 0.14f, 4.10f),
                PoleColor);
        }

        SpawnDecoration(
            CubeMesh,
            Base + FVector::UpVector * 365.0f,
            Rotation,
            FVector(4.45f, 0.14f, 0.72f),
            Colors[Marker]);
        SpawnDecoration(
            CubeMesh,
            Base + FVector::UpVector * 455.0f,
            Rotation,
            FVector(2.10f, 0.18f, 0.12f),
            FLinearColor(0.94f, 0.82f, 0.24f, 1.0f));
    }
}

void URIRoadsideLandmarkSubsystem::BuildMarketCluster()
{
    const float Angle = PI * 0.66f;
    const FVector Route = RIRouteCenter(Angle);
    const FVector Forward = RIRouteForward(Angle);
    const FVector Outward = RIRouteOutward(Angle);
    const FVector RoadFacing = -Outward;
    const FRotator Rotation = Forward.Rotation();

    const FVector MarketCenter = Route + Outward * (RIRoadWidth * 0.5f + 1850.0f);
    const FLinearColor WallColors[] = {
        FLinearColor(0.82f, 0.25f, 0.12f, 1.0f),
        FLinearColor(0.12f, 0.48f, 0.72f, 1.0f),
        FLinearColor(0.83f, 0.57f, 0.10f, 1.0f),
        FLinearColor(0.18f, 0.60f, 0.30f, 1.0f),
        FLinearColor(0.64f, 0.22f, 0.52f, 1.0f),
        FLinearColor(0.14f, 0.52f, 0.54f, 1.0f)};
    const FLinearColor RoofColors[] = {
        FLinearColor(0.10f, 0.22f, 0.34f, 1.0f),
        FLinearColor(0.46f, 0.10f, 0.08f, 1.0f),
        FLinearColor(0.12f, 0.30f, 0.18f, 1.0f)};

    for (int32 Shop = 0; Shop < 6; ++Shop)
    {
        const float Along = (static_cast<float>(Shop) - 2.5f) * 455.0f;
        const FVector ShopCenter = MarketCenter + Forward * Along;
        const FLinearColor Wall = WallColors[Shop];
        const FLinearColor Roof = RoofColors[Shop % 3];

        SpawnDecoration(
            CubeMesh,
            ShopCenter + FVector::UpVector * 132.0f,
            Rotation,
            FVector(3.55f, 2.30f, 2.64f),
            Wall);
        SpawnDecoration(
            CubeMesh,
            ShopCenter + FVector::UpVector * 282.0f,
            Rotation,
            FVector(3.85f, 2.62f, 0.15f),
            Roof);
        SpawnDecoration(
            CubeMesh,
            ShopCenter + RoadFacing * 245.0f + FVector::UpVector * 226.0f,
            Rotation,
            FVector(3.00f, 0.76f, 0.12f),
            Roof);
        SpawnDecoration(
            CubeMesh,
            ShopCenter + RoadFacing * 192.0f + FVector::UpVector * 73.0f,
            Rotation,
            FVector(2.35f, 0.28f, 0.72f),
            FLinearColor(0.30f, 0.16f, 0.07f, 1.0f));
    }

    // Two roadside tables and a compact parked CNG-like silhouette make the
    // market read as a lived-in stop rather than six anonymous colored cubes.
    for (const float Along : {-690.0f, 690.0f})
    {
        const FVector Table = MarketCenter + Forward * Along + RoadFacing * 420.0f;
        SpawnDecoration(CubeMesh, Table + FVector::UpVector * 72.0f, Rotation, FVector(1.45f, 0.65f, 0.10f), FLinearColor(0.34f, 0.18f, 0.08f, 1.0f));
        SpawnDecoration(CubeMesh, Table + FVector::UpVector * 36.0f, FRotator::ZeroRotator, FVector(0.10f, 0.10f, 0.72f), FLinearColor(0.22f, 0.12f, 0.06f, 1.0f));
    }

    const FVector CngCenter = Route + Outward * (RIRoadWidth * 0.5f + 1130.0f) + Forward * 980.0f;
    SpawnDecoration(CubeMesh, CngCenter + FVector::UpVector * 64.0f, Rotation, FVector(1.75f, 0.95f, 0.58f), FLinearColor(0.08f, 0.55f, 0.28f, 1.0f));
    SpawnDecoration(CubeMesh, CngCenter - Forward * 22.0f + FVector::UpVector * 125.0f, Rotation, FVector(1.18f, 0.82f, 0.64f), FLinearColor(0.05f, 0.24f, 0.16f, 1.0f));
    if (SphereMesh)
    {
        SpawnDecoration(SphereMesh, CngCenter + Forward * 78.0f + FVector::UpVector * 28.0f, FRotator::ZeroRotator, FVector(0.32f, 0.16f, 0.32f), FLinearColor(0.025f, 0.028f, 0.032f, 1.0f));
        SpawnDecoration(SphereMesh, CngCenter - Forward * 72.0f + Outward * 48.0f + FVector::UpVector * 28.0f, FRotator::ZeroRotator, FVector(0.32f, 0.16f, 0.32f), FLinearColor(0.025f, 0.028f, 0.032f, 1.0f));
        SpawnDecoration(SphereMesh, CngCenter - Forward * 72.0f - Outward * 48.0f + FVector::UpVector * 28.0f, FRotator::ZeroRotator, FVector(0.32f, 0.16f, 0.32f), FLinearColor(0.025f, 0.028f, 0.032f, 1.0f));
    }
}

void URIRoadsideLandmarkSubsystem::BuildBusStopCluster()
{
    const float Angle = PI * 1.28f;
    const FVector Route = RIRouteCenter(Angle);
    const FVector Forward = RIRouteForward(Angle);
    const FVector Outward = RIRouteOutward(Angle);
    const FVector RoadFacing = -Outward;
    const FRotator Rotation = Forward.Rotation();

    const FVector Shelter = Route + Outward * (RIRoadWidth * 0.5f + 1450.0f);
    const FLinearColor ShelterColor(0.14f, 0.32f, 0.48f, 1.0f);
    const FLinearColor RoofColor(0.80f, 0.24f, 0.10f, 1.0f);

    SpawnDecoration(CubeMesh, Shelter + FVector::UpVector * 145.0f, Rotation, FVector(3.8f, 0.15f, 2.90f), ShelterColor);
    SpawnDecoration(CubeMesh, Shelter + RoadFacing * 115.0f + FVector::UpVector * 292.0f, Rotation, FVector(4.25f, 1.25f, 0.14f), RoofColor);
    SpawnDecoration(CubeMesh, Shelter + RoadFacing * 92.0f + FVector::UpVector * 62.0f, Rotation, FVector(2.65f, 0.42f, 0.16f), FLinearColor(0.32f, 0.17f, 0.07f, 1.0f));

    for (const float Along : {-315.0f, 315.0f})
    {
        SpawnDecoration(CubeMesh, Shelter + Forward * Along + RoadFacing * 102.0f + FVector::UpVector * 145.0f, FRotator::ZeroRotator, FVector(0.12f, 0.12f, 2.90f), FLinearColor(0.10f, 0.11f, 0.12f, 1.0f));
    }

    // A parked local-bus silhouette, deliberately well outside the race corridor.
    const FVector Bus = Route + Outward * (RIRoadWidth * 0.5f + 1160.0f) - Forward * 860.0f;
    SpawnDecoration(CubeMesh, Bus + FVector::UpVector * 112.0f, Rotation, FVector(5.60f, 1.48f, 1.12f), FLinearColor(0.88f, 0.62f, 0.12f, 1.0f));
    SpawnDecoration(CubeMesh, Bus + FVector::UpVector * 190.0f, Rotation, FVector(4.55f, 1.38f, 0.52f), FLinearColor(0.12f, 0.26f, 0.34f, 1.0f));
    SpawnDecoration(CubeMesh, Bus + RoadFacing * 148.0f + FVector::UpVector * 110.0f, Rotation, FVector(4.85f, 0.12f, 0.18f), FLinearColor(0.82f, 0.16f, 0.08f, 1.0f));

    if (SphereMesh)
    {
        for (const float Along : {-330.0f, 330.0f})
        {
            for (const float Side : {-1.0f, 1.0f})
            {
                SpawnDecoration(
                    SphereMesh,
                    Bus + Forward * Along + Outward * (Side * 128.0f) + FVector::UpVector * 38.0f,
                    FRotator::ZeroRotator,
                    FVector(0.42f, 0.18f, 0.42f),
                    FLinearColor(0.025f, 0.028f, 0.032f, 1.0f));
            }
        }
    }

    // Tall stop sign silhouette acts as a strong racing-distance marker.
    const FVector SignBase = Shelter + Forward * 520.0f + RoadFacing * 55.0f;
    SpawnDecoration(CubeMesh, SignBase + FVector::UpVector * 180.0f, FRotator::ZeroRotator, FVector(0.10f, 0.10f, 3.60f), FLinearColor(0.12f, 0.12f, 0.12f, 1.0f));
    SpawnDecoration(CubeMesh, SignBase + FVector::UpVector * 350.0f, Rotation, FVector(0.95f, 0.14f, 0.70f), FLinearColor(0.10f, 0.62f, 0.42f, 1.0f));
}

void URIRoadsideLandmarkSubsystem::BuildPondAndFields()
{
    const float Angle = PI * 1.76f;
    const FVector Route = RIRouteCenter(Angle);
    const FVector Forward = RIRouteForward(Angle);
    const FVector Outward = RIRouteOutward(Angle);
    const FRotator Rotation = Forward.Rotation();

    const FVector Pond = Route + Outward * (RIRoadWidth * 0.5f + 2550.0f);
    const FLinearColor Water(0.08f, 0.42f, 0.52f, 1.0f);
    const FLinearColor Bank(0.28f, 0.48f, 0.12f, 1.0f);

    SpawnDecoration(CubeMesh, Pond + FVector(0.0f, 0.0f, 2.0f), Rotation, FVector(13.5f, 7.2f, 0.045f), Water);
    SpawnDecoration(CubeMesh, Pond + Outward * 735.0f + FVector::UpVector * 12.0f, Rotation, FVector(13.8f, 0.42f, 0.12f), Bank);
    SpawnDecoration(CubeMesh, Pond - Outward * 735.0f + FVector::UpVector * 12.0f, Rotation, FVector(13.8f, 0.42f, 0.12f), Bank);
    SpawnDecoration(CubeMesh, Pond + Forward * 1365.0f + FVector::UpVector * 12.0f, Rotation, FVector(0.42f, 7.5f, 0.12f), Bank);
    SpawnDecoration(CubeMesh, Pond - Forward * 1365.0f + FVector::UpVector * 12.0f, Rotation, FVector(0.42f, 7.5f, 0.12f), Bank);

    // Contrasting field strips next to the water give the far side of the lap a
    // broad rural silhouette instead of another wall of isolated props.
    const FLinearColor Greens[] = {
        FLinearColor(0.22f, 0.50f, 0.10f, 1.0f),
        FLinearColor(0.34f, 0.60f, 0.14f, 1.0f),
        FLinearColor(0.48f, 0.66f, 0.18f, 1.0f),
        FLinearColor(0.28f, 0.56f, 0.16f, 1.0f)};

    const FVector FieldCenter = Route + Outward * (RIRoadWidth * 0.5f + 4300.0f);
    for (int32 Strip = 0; Strip < 4; ++Strip)
    {
        SpawnDecoration(
            CubeMesh,
            FieldCenter + Forward * ((static_cast<float>(Strip) - 1.5f) * 590.0f) + FVector::UpVector * 5.0f,
            Rotation,
            FVector(5.3f, 3.4f, 0.05f),
            Greens[Strip]);
    }

    // A few broad-canopy trees frame the pond without creating a dense prop wall.
    if (SphereMesh)
    {
        for (int32 Tree = -2; Tree <= 2; ++Tree)
        {
            const FVector TreeBase = Pond + Forward * (static_cast<float>(Tree) * 520.0f) + Outward * 900.0f;
            SpawnDecoration(CubeMesh, TreeBase + FVector::UpVector * 165.0f, FRotator::ZeroRotator, FVector(0.22f, 0.22f, 3.30f), FLinearColor(0.25f, 0.14f, 0.07f, 1.0f));
            SpawnDecoration(SphereMesh, TreeBase + FVector::UpVector * 365.0f, FRotator::ZeroRotator, FVector(1.85f, 1.55f, 1.05f), FLinearColor(0.08f, 0.34f, 0.09f, 1.0f));
        }
    }
}
