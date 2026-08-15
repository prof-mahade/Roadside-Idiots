#include "Presentation/RITrackPresentationSubsystem.h"

#include "Camera/CameraComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Vehicle/RIBikePawn.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float TPRouteRadiusX = 9000.0f;
    constexpr float TPRouteRadiusY = 5000.0f;
    constexpr float TPRoadWidth = 1200.0f;
    constexpr int32 TPRouteSegments = 40;

    const FLinearColor TPGrassColor(0.13f, 0.29f, 0.085f, 1.0f);
    const FLinearColor TPGrassAccentColor(0.085f, 0.20f, 0.055f, 1.0f);
    const FLinearColor TPAsphaltColor(0.070f, 0.075f, 0.085f, 1.0f);
    const FLinearColor TPConcreteColor(0.21f, 0.25f, 0.30f, 1.0f);
    const FLinearColor TPEdgeYellow(0.96f, 0.62f, 0.055f, 1.0f);
    const FLinearColor TPLaneWhite(0.91f, 0.93f, 0.95f, 1.0f);
    const FLinearColor TPDarkColor(0.055f, 0.065f, 0.075f, 1.0f);
    const FLinearColor TPTreeTrunk(0.24f, 0.115f, 0.04f, 1.0f);
    const FLinearColor TPTreeLeafA(0.065f, 0.30f, 0.075f, 1.0f);
    const FLinearColor TPTreeLeafB(0.10f, 0.38f, 0.095f, 1.0f);
    const FLinearColor TPRedColor(0.78f, 0.12f, 0.055f, 1.0f);
    const FLinearColor TPBlueColor(0.055f, 0.30f, 0.72f, 1.0f);
    const FLinearColor TPOrangeColor(0.92f, 0.45f, 0.035f, 1.0f);
    const FLinearColor TPPurpleColor(0.38f, 0.10f, 0.58f, 1.0f);
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
        FMath::Cos(AngleRadians) * TPRouteRadiusX,
        FMath::Sin(AngleRadians) * TPRouteRadiusY,
        Height);
}

FVector URITrackPresentationSubsystem::RouteTangent(const float AngleRadians) const
{
    return FVector(
        -FMath::Sin(AngleRadians) * TPRouteRadiusX,
        FMath::Cos(AngleRadians) * TPRouteRadiusY,
        0.0f);
}

UInstancedStaticMeshComponent* URITrackPresentationSubsystem::CreateInstanceGroup(
    UStaticMesh* MeshAsset,
    const FLinearColor& Color)
{
    if (!PresentationRoot || !MeshAsset || !BasicMaterial)
    {
        return nullptr;
    }

    UInstancedStaticMeshComponent* Group = NewObject<UInstancedStaticMeshComponent>(PresentationRoot);
    if (!Group)
    {
        return nullptr;
    }

    Group->SetMobility(EComponentMobility::Movable);
    Group->SetStaticMesh(MeshAsset);
    Group->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Group->SetCollisionProfileName(TEXT("NoCollision"));
    Group->SetGenerateOverlapEvents(false);
    Group->SetCastShadow(true);

    if (USceneComponent* RootComponent = PresentationRoot->GetRootComponent())
    {
        Group->SetupAttachment(RootComponent);
    }

    if (UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BasicMaterial, Group))
    {
        DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
        Group->SetMaterial(0, DynamicMaterial);
    }

    Group->RegisterComponent();
    return Group;
}

void URITrackPresentationSubsystem::AddVisualInstance(
    UInstancedStaticMeshComponent* Group,
    const FVector& Location,
    const FRotator& Rotation,
    const FVector& Scale)
{
    if (!Group)
    {
        return;
    }

    Group->AddInstance(FTransform(Rotation, Location, Scale), false);
}

void URITrackPresentationSubsystem::InitializeInstanceGroups()
{
    UWorld* World = GetWorld();
    if (!World || PresentationRoot)
    {
        return;
    }

    PresentationRoot = World->SpawnActor<AActor>();
    if (!PresentationRoot)
    {
        return;
    }

    PresentationRoot->SetActorEnableCollision(false);

    USceneComponent* RootComponent = NewObject<USceneComponent>(PresentationRoot, TEXT("TrackPresentationRoot"));
    if (!RootComponent)
    {
        PresentationRoot->Destroy();
        PresentationRoot = nullptr;
        return;
    }

    PresentationRoot->SetRootComponent(RootComponent);
    RootComponent->RegisterComponent();

    GrassInstances = CreateInstanceGroup(CubeMesh, TPGrassColor);
    GrassAccentInstances = CreateInstanceGroup(CubeMesh, TPGrassAccentColor);
    AsphaltInstances = CreateInstanceGroup(CubeMesh, TPAsphaltColor);
    ConcreteInstances = CreateInstanceGroup(CubeMesh, TPConcreteColor);
    YellowInstances = CreateInstanceGroup(CubeMesh, TPEdgeYellow);
    WhiteInstances = CreateInstanceGroup(CubeMesh, TPLaneWhite);
    DarkInstances = CreateInstanceGroup(CubeMesh, TPDarkColor);
    TrunkInstances = CreateInstanceGroup(CubeMesh, TPTreeTrunk);
    LeafAInstances = CreateInstanceGroup(SphereMesh, TPTreeLeafA);
    LeafBInstances = CreateInstanceGroup(SphereMesh, TPTreeLeafB);
    RedInstances = CreateInstanceGroup(CubeMesh, TPRedColor);
    BlueInstances = CreateInstanceGroup(CubeMesh, TPBlueColor);
    OrangeInstances = CreateInstanceGroup(CubeMesh, TPOrangeColor);
    PurpleInstances = CreateInstanceGroup(CubeMesh, TPPurpleColor);
}

void URITrackPresentationSubsystem::TryBuildPresentation()
{
    if (bBuiltPresentation || !GetWorld())
    {
        return;
    }

    CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    BasicMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    if (!CubeMesh || !SphereMesh || !BasicMaterial)
    {
        return;
    }

    InitializeInstanceGroups();
    if (!PresentationRoot || !GrassInstances || !AsphaltInstances)
    {
        return;
    }

    // Visual-only ground. It sits just above the seamless authoritative collider,
    // but every presentation component has collision disabled.
    AddVisualInstance(
        GrassInstances,
        FVector(0.0f, 0.0f, 0.55f),
        FRotator::ZeroRotator,
        FVector(300.0f, 300.0f, 0.008f));

    BuildTrackSkin();
    BuildStartFinish();
    BuildRoadsideScenery();
    bBuiltPresentation = true;
}

void URITrackPresentationSubsystem::BuildTrackSkin()
{
    for (int32 Index = 0; Index < TPRouteSegments; ++Index)
    {
        const float AngleA = 2.0f * PI * static_cast<float>(Index) / static_cast<float>(TPRouteSegments);
        const float AngleB = 2.0f * PI * static_cast<float>(Index + 1) / static_cast<float>(TPRouteSegments);
        const FVector A = RoutePoint(AngleA, 0.0f);
        const FVector B = RoutePoint(AngleB, 0.0f);

        FVector Direction = B - A;
        Direction.Z = 0.0f;
        const float Length = Direction.Size();
        if (Length < 1.0f)
        {
            continue;
        }

        const FVector Forward = Direction / Length;
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        const FVector Center = (A + B) * 0.5f;
        const FRotator Rotation = Forward.Rotation();

        AddVisualInstance(
            AsphaltInstances,
            FVector(Center.X, Center.Y, 1.85f),
            Rotation,
            FVector((Length + 120.0f) / 100.0f, TPRoadWidth / 100.0f, 0.012f));

        // Two dashed separators create three broad lanes while preserving plenty
        // of room for chaotic overtakes and side interactions.
        for (const float LaneOffset : {-200.0f, 200.0f})
        {
            const FVector LaneCenter = Center + Right * LaneOffset;
            AddVisualInstance(
                WhiteInstances,
                FVector(LaneCenter.X, LaneCenter.Y, 3.15f),
                Rotation,
                FVector((Length * 0.40f) / 100.0f, 0.050f, 0.012f));
        }

        const float EdgeOffset = TPRoadWidth * 0.5f - 48.0f;
        for (const float Side : {-1.0f, 1.0f})
        {
            const FVector EdgeCenter = Center + Right * (EdgeOffset * Side);
            AddVisualInstance(
                YellowInstances,
                FVector(EdgeCenter.X, EdgeCenter.Y, 3.20f),
                Rotation,
                FVector((Length + 100.0f) / 100.0f, 0.045f, 0.012f));
        }

        const float BarrierOffset = TPRoadWidth * 0.5f + 28.0f;
        for (const float Side : {-1.0f, 1.0f})
        {
            const FVector BarrierCenter = Center + Right * (BarrierOffset * Side);
            AddVisualInstance(
                ConcreteInstances,
                FVector(BarrierCenter.X, BarrierCenter.Y, 60.0f),
                Rotation,
                FVector((Length + 242.0f) / 100.0f, 0.68f, 1.22f));

            AddVisualInstance(
                YellowInstances,
                FVector(BarrierCenter.X, BarrierCenter.Y, 123.0f),
                Rotation,
                FVector((Length + 245.0f) / 100.0f, 0.76f, 0.040f));

            // Brighter verge strip just beyond the wall keeps the outside world
            // readable and prevents the roadside from collapsing into a black void.
            const FVector VergeCenter = Center + Right * ((TPRoadWidth * 0.5f + 520.0f) * Side);
            AddVisualInstance(
                GrassAccentInstances,
                FVector(VergeCenter.X, VergeCenter.Y, 1.15f),
                Rotation,
                FVector((Length + 280.0f) / 100.0f, 7.5f, 0.010f));
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

    constexpr int32 TileCount = 12;
    const float TileWidth = TPRoadWidth / static_cast<float>(TileCount);
    for (int32 TileIndex = 0; TileIndex < TileCount; ++TileIndex)
    {
        const float Across = -TPRoadWidth * 0.5f + TileWidth * (static_cast<float>(TileIndex) + 0.5f);
        const FVector TileLocation = Center + Right * Across;
        UInstancedStaticMeshComponent* TileGroup = (TileIndex % 2 == 0) ? WhiteInstances : DarkInstances;

        AddVisualInstance(
            TileGroup,
            FVector(TileLocation.X, TileLocation.Y, 3.40f),
            Rotation,
            FVector(0.95f, TileWidth / 100.0f, 0.014f));
    }

    const float PostOffset = TPRoadWidth * 0.5f + 135.0f;
    for (const float Side : {-1.0f, 1.0f})
    {
        const FVector PostBase = Center + Right * (PostOffset * Side);
        AddVisualInstance(
            DarkInstances,
            FVector(PostBase.X, PostBase.Y, 225.0f),
            Rotation,
            FVector(0.34f, 0.34f, 4.5f));

        AddVisualInstance(
            YellowInstances,
            FVector(PostBase.X, PostBase.Y, 225.0f),
            Rotation,
            FVector(0.39f, 0.12f, 4.15f));
    }

    AddVisualInstance(
        DarkInstances,
        Center + FVector::UpVector * 442.0f,
        Rotation,
        FVector(0.30f, (TPRoadWidth + 380.0f) / 100.0f, 0.34f));

    // Alternating overhead blocks make the gantry look like an intentional race
    // structure rather than a single primitive beam.
    constexpr int32 GantryBlocks = 8;
    const float GantrySpan = TPRoadWidth + 180.0f;
    const float GantryBlockWidth = GantrySpan / static_cast<float>(GantryBlocks);
    for (int32 Block = 0; Block < GantryBlocks; ++Block)
    {
        const float Across = -GantrySpan * 0.5f + GantryBlockWidth * (static_cast<float>(Block) + 0.5f);
        const FVector BlockLocation = Center + Right * Across + FVector::UpVector * 472.0f;
        AddVisualInstance(
            (Block % 2 == 0) ? YellowInstances : WhiteInstances,
            BlockLocation,
            Rotation,
            FVector(0.36f, GantryBlockWidth / 100.0f, 0.22f));
    }

    // Simple colored timing boards at both sides of the gantry.
    AddVisualInstance(RedInstances, Center - Right * (TPRoadWidth * 0.5f + 120.0f) + FVector::UpVector * 360.0f, Rotation, FVector(0.20f, 0.60f, 0.42f));
    AddVisualInstance(BlueInstances, Center + Right * (TPRoadWidth * 0.5f + 120.0f) + FVector::UpVector * 360.0f, Rotation, FVector(0.20f, 0.60f, 0.42f));
}

void URITrackPresentationSubsystem::BuildRoadsideScenery()
{
    // Keep the original sphere/cube trees only as a true fallback for machines
    // that do not have the approved free banana pack installed locally.
    const bool bUsePrimitiveTreeFallback =
        LoadObject<UStaticMesh>(nullptr, TEXT("/Game/PN_Banana/Meshes/plants/banana_01_07.banana_01_07")) == nullptr;

    if (bUsePrimitiveTreeFallback)
    {
        for (int32 Index = 0; Index < TPRouteSegments; Index += 2)
        {
            const float Angle = 2.0f * PI * static_cast<float>(Index) / static_cast<float>(TPRouteSegments);
            const FVector RouteCenter = RoutePoint(Angle, 0.0f);
            const FVector Forward = RouteTangent(Angle).GetSafeNormal2D();
            const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
            const float Side = (Index / 2) % 2 == 0 ? 1.0f : -1.0f;
            const float Distance = TPRoadWidth * 0.5f + 900.0f + 110.0f * static_cast<float>(Index % 5);
            const FVector TreeBase = RouteCenter + Right * (Distance * Side);
            const float HeightVariation = 0.88f + 0.07f * static_cast<float>(Index % 5);

            AddVisualInstance(
                TrunkInstances,
                FVector(TreeBase.X, TreeBase.Y, 135.0f),
                FRotator::ZeroRotator,
                FVector(0.30f, 0.30f, 2.7f * HeightVariation));

            AddVisualInstance(
                (Index % 4 == 0) ? LeafAInstances : LeafBInstances,
                FVector(TreeBase.X, TreeBase.Y, 350.0f * HeightVariation),
                FRotator::ZeroRotator,
                FVector(1.28f, 1.16f, 1.55f) * HeightVariation);
        }
    }

    UInstancedStaticMeshComponent* SignGroups[] =
    {
        RedInstances,
        BlueInstances,
        OrangeInstances,
        PurpleInstances
    };

    for (int32 SignIndex = 0; SignIndex < 8; ++SignIndex)
    {
        const int32 RouteIndex = 2 + SignIndex * 5;
        const float Angle = 2.0f * PI * static_cast<float>(RouteIndex) / static_cast<float>(TPRouteSegments);
        const FVector RouteCenter = RoutePoint(Angle, 0.0f);
        const FVector Forward = RouteTangent(Angle).GetSafeNormal2D();
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        const float Side = SignIndex % 2 == 0 ? -1.0f : 1.0f;
        const FVector Base = RouteCenter + Right * ((TPRoadWidth * 0.5f + 690.0f) * Side);
        const FRotator Rotation = Forward.Rotation();

        AddVisualInstance(
            DarkInstances,
            FVector(Base.X, Base.Y, 120.0f),
            Rotation,
            FVector(0.15f, 0.15f, 2.4f));

        AddVisualInstance(
            SignGroups[SignIndex % 4],
            FVector(Base.X, Base.Y, 300.0f),
            Rotation,
            FVector(0.13f, 1.65f, 0.76f));
    }

    // Small reflective-style roadside posts add speed references on long bends.
    for (int32 Index = 1; Index < TPRouteSegments; Index += 4)
    {
        const float Angle = 2.0f * PI * static_cast<float>(Index) / static_cast<float>(TPRouteSegments);
        const FVector RouteCenter = RoutePoint(Angle, 0.0f);
        const FVector Forward = RouteTangent(Angle).GetSafeNormal2D();
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();

        for (const float Side : {-1.0f, 1.0f})
        {
            const FVector Base = RouteCenter + Right * ((TPRoadWidth * 0.5f + 310.0f) * Side);
            AddVisualInstance(
                WhiteInstances,
                FVector(Base.X, Base.Y, 65.0f),
                Forward.Rotation(),
                FVector(0.10f, 0.10f, 1.30f));
            AddVisualInstance(
                YellowInstances,
                FVector(Base.X, Base.Y, 118.0f),
                Forward.Rotation(),
                FVector(0.12f, 0.12f, 0.18f));
        }
    }
}

ARIBikePawn* URITrackPresentationSubsystem::FindHumanBike()
{
    if (CachedHumanBike.IsValid())
    {
        return CachedHumanBike.Get();
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

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
    if (!Bike || !Bike->GetBikeMovement())
    {
        return;
    }

    UCameraComponent* Camera = Bike->FindComponentByClass<UCameraComponent>();
    if (!Camera)
    {
        return;
    }

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
