#include "Presentation/RITrackPresentationSubsystem.h"

#include "Camera/CameraComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Vehicle/RIBikePawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float RouteRadiusX = 9000.0f;
    constexpr float RouteRadiusY = 5000.0f;
    constexpr float RoadWidth = 1200.0f;
    constexpr int32 RouteSegments = 40;

    const FLinearColor GrassColor(0.075f, 0.135f, 0.055f, 1.0f);
    const FLinearColor AsphaltColor(0.055f, 0.060f, 0.068f, 1.0f);
    const FLinearColor ConcreteColor(0.24f, 0.27f, 0.30f, 1.0f);
    const FLinearColor EdgeYellow(0.95f, 0.62f, 0.06f, 1.0f);
    const FLinearColor LaneWhite(0.88f, 0.90f, 0.92f, 1.0f);
    const FLinearColor TreeTrunk(0.22f, 0.10f, 0.035f, 1.0f);
    const FLinearColor TreeLeafA(0.07f, 0.24f, 0.065f, 1.0f);
    const FLinearColor TreeLeafB(0.11f, 0.31f, 0.085f, 1.0f);
}

bool URITrackPresentationSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !IsTemplate() && World && World->IsGameWorld();
}

TStatId URITrackPresentationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URITrackPresentationSubsystem, STATGROUP_Tickables);
}

FVector URITrackPresentationSubsystem::RoutePoint(const float AngleRadians, const float Height) const
{
    return FVector(
        FMath::Cos(AngleRadians) * RouteRadiusX,
        FMath::Sin(AngleRadians) * RouteRadiusY,
        Height);
}

FVector URITrackPresentationSubsystem::RouteTangent(const float AngleRadians) const
{
    return FVector(
        -FMath::Sin(AngleRadians) * RouteRadiusX,
        FMath::Cos(AngleRadians) * RouteRadiusY,
        0.0f);
}

AStaticMeshActor* URITrackPresentationSubsystem::SpawnVisual(
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

    UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetStaticMesh(MeshAsset);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->SetCollisionProfileName(TEXT("NoCollision"));
    Mesh->SetGenerateOverlapEvents(false);
    Actor->SetActorEnableCollision(false);
    Actor->SetActorScale3D(Scale);

    if (BasicMaterial)
    {
        if (UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BasicMaterial, Mesh))
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
            Mesh->SetMaterial(0, DynamicMaterial);
        }
    }

    return Actor;
}

void URITrackPresentationSubsystem::TryBuildPresentation()
{
    if (bBuiltPresentation || !GetWorld()) return;

    CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    BasicMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    if (!CubeMesh || !SphereMesh || !BasicMaterial)
    {
        return;
    }

    // Purely visual ground skin. The authoritative continuous collision floor
    // remains untouched underneath this layer.
    SpawnVisual(
        CubeMesh,
        FVector(0.0f, 0.0f, 0.35f),
        FRotator::ZeroRotator,
        FVector(300.0f, 300.0f, 0.006f),
        GrassColor);

    BuildTrackSkin();
    BuildStartFinish();
    BuildRoadsideScenery();
    bBuiltPresentation = true;
}

void URITrackPresentationSubsystem::BuildTrackSkin()
{
    for (int32 Index = 0; Index < RouteSegments; ++Index)
    {
        const float AngleA = 2.0f * PI * static_cast<float>(Index) / static_cast<float>(RouteSegments);
        const float AngleB = 2.0f * PI * static_cast<float>(Index + 1) / static_cast<float>(RouteSegments);
        const FVector A = RoutePoint(AngleA, 0.0f);
        const FVector B = RoutePoint(AngleB, 0.0f);

        FVector Direction = B - A;
        Direction.Z = 0.0f;
        const float Length = Direction.Size();
        if (Length < 1.0f) continue;

        const FVector Forward = Direction / Length;
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        const FVector Center = (A + B) * 0.5f;
        const FRotator Rotation = Forward.Rotation();

        // Dark asphalt overlay sits above the old checkerboard road visuals.
        SpawnVisual(
            CubeMesh,
            FVector(Center.X, Center.Y, 1.85f),
            Rotation,
            FVector((Length + 120.0f) / 100.0f, RoadWidth / 100.0f, 0.012f),
            AsphaltColor);

        // One dashed centre guide per route segment. This gives speed/curvature
        // information without pretending the prototype is a strict road-lane sim.
        SpawnVisual(
            CubeMesh,
            FVector(Center.X, Center.Y, 3.15f),
            Rotation,
            FVector((Length * 0.42f) / 100.0f, 0.075f, 0.012f),
            LaneWhite);

        const float EdgeOffset = RoadWidth * 0.5f - 48.0f;
        for (const float Side : {-1.0f, 1.0f})
        {
            const FVector EdgeCenter = Center + Right * (EdgeOffset * Side);
            SpawnVisual(
                CubeMesh,
                FVector(EdgeCenter.X, EdgeCenter.Y, 3.20f),
                Rotation,
                FVector((Length + 100.0f) / 100.0f, 0.055f, 0.012f),
                EdgeYellow);
        }

        // Cover the old black barrier blocks with a cleaner concrete shell and
        // a yellow top cap. Underlying collision remains the original proven wall.
        const float BarrierOffset = RoadWidth * 0.5f + 28.0f;
        for (const float Side : {-1.0f, 1.0f})
        {
            const FVector BarrierCenter = Center + Right * (BarrierOffset * Side);
            SpawnVisual(
                CubeMesh,
                FVector(BarrierCenter.X, BarrierCenter.Y, 60.0f),
                Rotation,
                FVector((Length + 242.0f) / 100.0f, 0.68f, 1.22f),
                ConcreteColor);

            SpawnVisual(
                CubeMesh,
                FVector(BarrierCenter.X, BarrierCenter.Y, 123.0f),
                Rotation,
                FVector((Length + 245.0f) / 100.0f, 0.76f, 0.045f),
                EdgeYellow);
        }
    }
}

void URITrackPresentationSubsystem::BuildStartFinish()
{
    const float Angle = 0.0f;
    const FVector Center = RoutePoint(Angle, 0.0f);
    const FVector Forward = RouteTangent(Angle).GetSafeNormal2D();
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
    const FRotator Rotation = Forward.Rotation();

    constexpr int32 TileCount = 10;
    const float TileWidth = RoadWidth / static_cast<float>(TileCount);
    for (int32 TileIndex = 0; TileIndex < TileCount; ++TileIndex)
    {
        const float Across = -RoadWidth * 0.5f + TileWidth * (static_cast<float>(TileIndex) + 0.5f);
        const FVector TileLocation = Center + Right * Across;
        const FLinearColor TileColor = (TileIndex % 2 == 0)
            ? FLinearColor(0.93f, 0.94f, 0.95f, 1.0f)
            : FLinearColor(0.055f, 0.060f, 0.065f, 1.0f);

        SpawnVisual(
            CubeMesh,
            FVector(TileLocation.X, TileLocation.Y, 3.40f),
            Rotation,
            FVector(0.95f, TileWidth / 100.0f, 0.014f),
            TileColor);
    }

    const float PostOffset = RoadWidth * 0.5f + 135.0f;
    for (const float Side : {-1.0f, 1.0f})
    {
        const FVector PostBase = Center + Right * (PostOffset * Side);
        SpawnVisual(
            CubeMesh,
            FVector(PostBase.X, PostBase.Y, 225.0f),
            Rotation,
            FVector(0.24f, 0.24f, 4.5f),
            EdgeYellow);
    }

    SpawnVisual(
        CubeMesh,
        Center + FVector::UpVector * 442.0f,
        Rotation,
        FVector(0.26f, (RoadWidth + 380.0f) / 100.0f, 0.28f),
        FLinearColor(0.08f, 0.09f, 0.10f, 1.0f));

    SpawnVisual(
        CubeMesh,
        Center + FVector::UpVector * 472.0f,
        Rotation,
        FVector(0.30f, 4.8f, 0.24f),
        EdgeYellow);
}

void URITrackPresentationSubsystem::BuildRoadsideScenery()
{
    // Sparse stylised scenery gives the player visual speed references without
    // adding collision or affecting the analytical racing line.
    for (int32 Index = 0; Index < RouteSegments; Index += 3)
    {
        const float Angle = 2.0f * PI * static_cast<float>(Index) / static_cast<float>(RouteSegments);
        const FVector RouteCenter = RoutePoint(Angle, 0.0f);
        const FVector Forward = RouteTangent(Angle).GetSafeNormal2D();
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        const float Side = (Index / 3) % 2 == 0 ? 1.0f : -1.0f;
        const float Distance = RoadWidth * 0.5f + 850.0f + 90.0f * static_cast<float>(Index % 5);
        const FVector TreeBase = RouteCenter + Right * (Distance * Side);
        const float HeightVariation = 0.90f + 0.08f * static_cast<float>(Index % 4);

        SpawnVisual(
            CubeMesh,
            FVector(TreeBase.X, TreeBase.Y, 135.0f),
            FRotator::ZeroRotator,
            FVector(0.28f, 0.28f, 2.7f * HeightVariation),
            TreeTrunk);

        SpawnVisual(
            SphereMesh,
            FVector(TreeBase.X, TreeBase.Y, 350.0f * HeightVariation),
            FRotator::ZeroRotator,
            FVector(1.25f, 1.15f, 1.55f) * HeightVariation,
            (Index % 2 == 0) ? TreeLeafA : TreeLeafB);
    }

    // Four simple roadside boards break up the empty horizon and act as corner
    // landmarks. They are presentation only and intentionally non-colliding.
    const FLinearColor SignColors[] =
    {
        FLinearColor(0.78f, 0.16f, 0.08f, 1.0f),
        FLinearColor(0.06f, 0.34f, 0.68f, 1.0f),
        FLinearColor(0.92f, 0.56f, 0.05f, 1.0f),
        FLinearColor(0.34f, 0.12f, 0.56f, 1.0f)
    };

    for (int32 SignIndex = 0; SignIndex < 4; ++SignIndex)
    {
        const int32 RouteIndex = 4 + SignIndex * 10;
        const float Angle = 2.0f * PI * static_cast<float>(RouteIndex) / static_cast<float>(RouteSegments);
        const FVector RouteCenter = RoutePoint(Angle, 0.0f);
        const FVector Forward = RouteTangent(Angle).GetSafeNormal2D();
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        const float Side = SignIndex % 2 == 0 ? -1.0f : 1.0f;
        const FVector Base = RouteCenter + Right * ((RoadWidth * 0.5f + 640.0f) * Side);
        const FRotator Rotation = Forward.Rotation();

        SpawnVisual(
            CubeMesh,
            FVector(Base.X, Base.Y, 120.0f),
            Rotation,
            FVector(0.14f, 0.14f, 2.4f),
            FLinearColor(0.16f, 0.17f, 0.18f, 1.0f));

        SpawnVisual(
            CubeMesh,
            FVector(Base.X, Base.Y, 295.0f),
            Rotation,
            FVector(0.12f, 1.45f, 0.72f),
            SignColors[SignIndex]);
    }
}

ARIBikePawn* URITrackPresentationSubsystem::FindHumanBike()
{
    if (CachedHumanBike.IsValid())
    {
        return CachedHumanBike.Get();
    }

    UWorld* World = GetWorld();
    if (!World) return nullptr;

    for (TActorIterator<ARIBikePawn> It(World); It; ++It)
    {
        ARIBikePawn* Bike = *It;
        const URIParticipantComponent* Participant = Bike ? Bike->GetParticipantComponent() : nullptr;
        if (Bike && Participant && Participant->IsHumanControlled())
        {
            CachedHumanBike = Bike;
            return Bike;
        }
    }

    return nullptr;
}

void URITrackPresentationSubsystem::UpdateCameraFeel(const float DeltaTime)
{
    ARIBikePawn* Bike = FindHumanBike();
    if (!Bike || !Bike->GetBikeMovement()) return;

    UCameraComponent* Camera = Bike->FindComponentByClass<UCameraComponent>();
    if (!Camera) return;

    const float SpeedKph = FMath::Abs(Bike->GetBikeMovement()->GetForwardSpeedKph());
    float SpeedAlpha = FMath::Clamp(SpeedKph / 100.0f, 0.0f, 1.0f);
    SpeedAlpha = SpeedAlpha * SpeedAlpha * (3.0f - 2.0f * SpeedAlpha);

    const float TargetFov = FMath::Lerp(92.0f, 101.0f, SpeedAlpha);
    const float SmoothedFov = FMath::FInterpTo(Camera->FieldOfView, TargetFov, DeltaTime, 3.2f);
    Camera->SetFieldOfView(SmoothedFov);
}

void URITrackPresentationSubsystem::Tick(const float DeltaTime)
{
    TryBuildPresentation();
    UpdateCameraFeel(DeltaTime);
}
