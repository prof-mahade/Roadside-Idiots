#include "Presentation/RIRoadsideArtSubsystem.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    // Keep all Unity-build symbols uniquely prefixed; Unreal may merge cpp files.
    constexpr float RIART_RouteRadiusX = 9000.0f;
    constexpr float RIART_RouteRadiusY = 5000.0f;
    constexpr float RIART_RoadWidth = 1200.0f;
    constexpr int32 RIART_RouteSegments = 40;

    const FLinearColor RIART_WoodColor(0.22f, 0.095f, 0.035f, 1.0f);
    const FLinearColor RIART_BambooColor(0.50f, 0.34f, 0.13f, 1.0f);
    const FLinearColor RIART_TrimColor(0.84f, 0.75f, 0.50f, 1.0f);
    const FLinearColor RIART_DoorColor(0.055f, 0.24f, 0.13f, 1.0f);
    const FLinearColor RIART_WindowColor(0.035f, 0.35f, 0.50f, 1.0f);
    const FLinearColor RIART_MetalColor(0.27f, 0.30f, 0.32f, 1.0f);
    const FLinearColor RIART_CrateColor(0.72f, 0.37f, 0.045f, 1.0f);
    const FLinearColor RIART_BarrelColor(0.035f, 0.30f, 0.52f, 1.0f);
}

bool URIRoadsideArtSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !bBuilt && !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIRoadsideArtSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIRoadsideArtSubsystem, STATGROUP_Tickables);
}

FVector URIRoadsideArtSubsystem::RoutePoint(const float AngleRadians, const float Height) const
{
    return FVector(
        FMath::Cos(AngleRadians) * RIART_RouteRadiusX,
        FMath::Sin(AngleRadians) * RIART_RouteRadiusY,
        Height);
}

FVector URIRoadsideArtSubsystem::RouteTangent(const float AngleRadians) const
{
    return FVector(
        -FMath::Sin(AngleRadians) * RIART_RouteRadiusX,
        FMath::Cos(AngleRadians) * RIART_RouteRadiusY,
        0.0f);
}

UInstancedStaticMeshComponent* URIRoadsideArtSubsystem::CreateColorGroup(
    UStaticMesh* Mesh,
    const FLinearColor& Color)
{
    if (!ArtRoot || !Mesh || !BasicMaterial)
    {
        return nullptr;
    }

    UInstancedStaticMeshComponent* Group = NewObject<UInstancedStaticMeshComponent>(ArtRoot);
    if (!Group)
    {
        return nullptr;
    }

    Group->SetMobility(EComponentMobility::Movable);
    Group->SetStaticMesh(Mesh);
    Group->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Group->SetCollisionProfileName(TEXT("NoCollision"));
    Group->SetGenerateOverlapEvents(false);
    Group->SetCastShadow(true);

    if (USceneComponent* Root = ArtRoot->GetRootComponent())
    {
        Group->SetupAttachment(Root);
    }

    if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BasicMaterial, Group))
    {
        Material->SetVectorParameterValue(TEXT("Color"), Color);
        Group->SetMaterial(0, Material);
    }

    ArtRoot->AddInstanceComponent(Group);
    Group->RegisterComponent();
    return Group;
}

UInstancedStaticMeshComponent* URIRoadsideArtSubsystem::CreateAssetGroup(UStaticMesh* Mesh)
{
    if (!ArtRoot || !Mesh)
    {
        return nullptr;
    }

    UInstancedStaticMeshComponent* Group = NewObject<UInstancedStaticMeshComponent>(ArtRoot);
    if (!Group)
    {
        return nullptr;
    }

    Group->SetMobility(EComponentMobility::Movable);
    Group->SetStaticMesh(Mesh);
    Group->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Group->SetCollisionProfileName(TEXT("NoCollision"));
    Group->SetGenerateOverlapEvents(false);
    Group->SetCastShadow(true);

    if (USceneComponent* Root = ArtRoot->GetRootComponent())
    {
        Group->SetupAttachment(Root);
    }

    // Preserve authored materials from the user's approved free PN_Banana pack.
    ArtRoot->AddInstanceComponent(Group);
    Group->RegisterComponent();
    return Group;
}

void URIRoadsideArtSubsystem::AddInstance(
    UInstancedStaticMeshComponent* Group,
    const FVector& Location,
    const FRotator& Rotation,
    const FVector& Scale)
{
    if (Group)
    {
        Group->AddInstance(FTransform(Rotation, Location, Scale), false);
    }
}

void URIRoadsideArtSubsystem::AddAssetFootprintInstance(
    UInstancedStaticMeshComponent* Group,
    const FVector& Location,
    const FRotator& Rotation,
    const float DesiredWidthCm,
    const float UniformScaleMultiplier)
{
    if (!Group || DesiredWidthCm <= 0.0f)
    {
        return;
    }

    UStaticMesh* Mesh = Group->GetStaticMesh();
    if (!Mesh)
    {
        return;
    }

    const FBoxSphereBounds Bounds = Mesh->GetBounds();
    const float NativeWidth = FMath::Max(Bounds.BoxExtent.X, Bounds.BoxExtent.Y) * 2.0f;
    if (NativeWidth <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const float ScaleValue = (DesiredWidthCm / NativeWidth) * FMath::Max(0.05f, UniformScaleMultiplier);
    const float LocalBottomZ = Bounds.Origin.Z - Bounds.BoxExtent.Z;
    const FVector GroundedLocation = Location - FVector::UpVector * (LocalBottomZ * ScaleValue);
    AddInstance(Group, GroundedLocation, Rotation, FVector(ScaleValue));
}

void URIRoadsideArtSubsystem::BuildClusterDetails()
{
    // Match the six VPR-19/20 cluster anchors and dress the existing graybox shells.
    // All details stay far outside the authoritative road/barrier and have NoCollision.
    const int32 RIART_ClusterIndices[] = {3, 9, 16, 24, 31, 37};

    for (int32 Cluster = 0; Cluster < UE_ARRAY_COUNT(RIART_ClusterIndices); ++Cluster)
    {
        const int32 RouteIndex = RIART_ClusterIndices[Cluster];
        const float Angle = 2.0f * PI * static_cast<float>(RouteIndex) / static_cast<float>(RIART_RouteSegments);
        const FVector RouteCenter = RoutePoint(Angle);
        const FVector Forward = RouteTangent(Angle).GetSafeNormal2D();
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        const float Side = (Cluster % 2 == 0) ? -1.0f : 1.0f;
        const FVector TowardRoad = -Right * Side;
        const FVector Base = RouteCenter + Right * ((RIART_RoadWidth * 0.5f + 1350.0f) * Side);
        const FRotator Rotation = Forward.Rotation();

        if (Cluster % 3 == 0)
        {
            // Roadside tea/snack stall: open counter, awning, posts, shelf and bench.
            const FVector Front = Base + TowardRoad * 185.0f;
            AddInstance(TrimInstances, Front + FVector::UpVector * 18.0f, Rotation, FVector(4.2f, 1.35f, 0.14f));
            AddInstance(WoodInstances, Front + FVector::UpVector * 82.0f, Rotation, FVector(2.85f, 0.24f, 0.70f));
            AddInstance(WoodInstances, Front - TowardRoad * 48.0f + FVector::UpVector * 137.0f, Rotation, FVector(2.55f, 0.20f, 0.12f));

            const FVector AwningCenter = Base + TowardRoad * 250.0f + FVector::UpVector * 238.0f;
            AddInstance(MetalInstances, AwningCenter, FRotator(0.0f, Rotation.Yaw, -7.0f * Side), FVector(4.4f, 1.55f, 0.10f));

            for (const float Along : {-205.0f, 205.0f})
            {
                const FVector Post = Base + Forward * Along + TowardRoad * 315.0f;
                AddInstance(BambooInstances, Post + FVector::UpVector * 112.0f, Rotation, FVector(0.10f, 0.10f, 2.24f));
            }

            const FVector Bench = Base + Forward * 305.0f + TowardRoad * 285.0f;
            AddInstance(WoodInstances, Bench + FVector::UpVector * 48.0f, Rotation, FVector(1.65f, 0.34f, 0.16f));
            AddInstance(WoodInstances, Bench + Forward * 115.0f + FVector::UpVector * 24.0f, Rotation, FVector(0.12f, 0.26f, 0.48f));
            AddInstance(WoodInstances, Bench - Forward * 115.0f + FVector::UpVector * 24.0f, Rotation, FVector(0.12f, 0.26f, 0.48f));

            const FVector Sign = Base - Forward * 315.0f + TowardRoad * 135.0f;
            AddInstance(BambooInstances, Sign + FVector::UpVector * 118.0f, Rotation, FVector(0.10f, 0.10f, 2.36f));
            AddInstance(CrateInstances, Sign + FVector::UpVector * 226.0f, Rotation, FVector(0.12f, 1.25f, 0.52f));

            AddAssetFootprintInstance(
                GroundBananaAInstances,
                Base + Forward * 455.0f - TowardRoad * 95.0f,
                FRotator(0.0f, Rotation.Yaw + 28.0f, 0.0f),
                150.0f,
                0.95f);
            AddAssetFootprintInstance(
                RottenLeavesInstances,
                Base - Forward * 420.0f - TowardRoad * 60.0f,
                FRotator(0.0f, Rotation.Yaw - 36.0f, 0.0f),
                185.0f,
                0.90f);
        }
        else if (Cluster % 3 == 1)
        {
            // Rural house frontage: real facade cues over the existing colored shell.
            const FVector Front = Base + TowardRoad * 190.0f;
            AddInstance(DoorInstances, Front + FVector::UpVector * 104.0f, Rotation, FVector(0.78f, 0.075f, 1.04f));
            AddInstance(TrimInstances, Front + FVector::UpVector * 218.0f, Rotation, FVector(0.92f, 0.10f, 0.10f));

            for (const float Along : {-170.0f, 170.0f})
            {
                const FVector Window = Front + Forward * Along + FVector::UpVector * 142.0f;
                AddInstance(WindowInstances, Window, Rotation, FVector(0.70f, 0.075f, 0.52f));
                AddInstance(TrimInstances, Window + FVector::UpVector * 62.0f, Rotation, FVector(0.84f, 0.09f, 0.07f));
            }

            const FVector Veranda = Base + TowardRoad * 255.0f;
            AddInstance(MetalInstances, Veranda + FVector::UpVector * 246.0f, FRotator(0.0f, Rotation.Yaw, -5.0f * Side), FVector(3.8f, 1.34f, 0.09f));
            AddInstance(TrimInstances, Veranda + FVector::UpVector * 17.0f, Rotation, FVector(3.6f, 1.22f, 0.13f));

            for (const float Along : {-255.0f, 255.0f})
            {
                const FVector Post = Veranda + Forward * Along;
                AddInstance(BambooInstances, Post + FVector::UpVector * 112.0f, Rotation, FVector(0.10f, 0.10f, 2.24f));
            }

            const FVector Steps = Veranda + TowardRoad * 115.0f;
            AddInstance(TrimInstances, Steps + FVector::UpVector * 9.0f, Rotation, FVector(1.25f, 0.70f, 0.09f));
            AddInstance(TrimInstances, Steps + TowardRoad * 55.0f + FVector::UpVector * 4.0f, Rotation, FVector(1.55f, 0.40f, 0.05f));

            const FVector Barrel = Base + Forward * 315.0f - TowardRoad * 110.0f;
            AddInstance(BarrelInstances, Barrel + FVector::UpVector * 58.0f, Rotation, FVector(0.58f, 0.58f, 1.15f));

            AddAssetFootprintInstance(
                GroundBananaBInstances,
                Base - Forward * 390.0f - TowardRoad * 110.0f,
                FRotator(0.0f, Rotation.Yaw + 74.0f, 0.0f),
                170.0f,
                0.95f);
        }
        else
        {
            // Open shelter / repair stop: bench, fascia, crate stack and hanging board.
            const FVector Front = Base + TowardRoad * 150.0f;
            AddInstance(WoodInstances, Front + FVector::UpVector * 52.0f, Rotation, FVector(2.5f, 0.34f, 0.18f));
            AddInstance(WoodInstances, Front + Forward * 185.0f + FVector::UpVector * 27.0f, Rotation, FVector(0.12f, 0.28f, 0.54f));
            AddInstance(WoodInstances, Front - Forward * 185.0f + FVector::UpVector * 27.0f, Rotation, FVector(0.12f, 0.28f, 0.54f));

            const FVector Fascia = Base + TowardRoad * 105.0f + FVector::UpVector * 228.0f;
            AddInstance(CrateInstances, Fascia, Rotation, FVector(0.14f, 2.55f, 0.42f));
            AddInstance(TrimInstances, Fascia + FVector::UpVector * 54.0f, Rotation, FVector(0.16f, 2.75f, 0.08f));

            const FVector CrateA = Base - Forward * 265.0f + TowardRoad * 75.0f;
            AddInstance(CrateInstances, CrateA + FVector::UpVector * 35.0f, Rotation, FVector(0.72f, 0.58f, 0.70f));
            AddInstance(CrateInstances, CrateA + Forward * 65.0f + FVector::UpVector * 91.0f, Rotation, FVector(0.58f, 0.48f, 0.54f));

            const FVector Barrel = Base + Forward * 285.0f + TowardRoad * 55.0f;
            AddInstance(BarrelInstances, Barrel + FVector::UpVector * 54.0f, Rotation, FVector(0.54f, 0.54f, 1.08f));

            const FVector SideScreen = Base - TowardRoad * 125.0f - Forward * 175.0f;
            AddInstance(BambooInstances, SideScreen + FVector::UpVector * 98.0f, Rotation, FVector(1.65f, 0.10f, 1.95f));

            AddAssetFootprintInstance(
                RottenLeavesInstances,
                Base + Forward * 390.0f - TowardRoad * 90.0f,
                FRotator(0.0f, Rotation.Yaw + 112.0f, 0.0f),
                175.0f,
                0.92f);
        }
    }
}

void URIRoadsideArtSubsystem::BuildFenceDetails()
{
    // Short broken fence runs suggest yards/plots without creating a continuous wall.
    const int32 RIART_FenceIndices[] = {5, 12, 19, 27, 34};

    for (int32 FenceIndex = 0; FenceIndex < UE_ARRAY_COUNT(RIART_FenceIndices); ++FenceIndex)
    {
        const int32 RouteIndex = RIART_FenceIndices[FenceIndex];
        const float Angle = 2.0f * PI * static_cast<float>(RouteIndex) / static_cast<float>(RIART_RouteSegments);
        const FVector RouteCenter = RoutePoint(Angle);
        const FVector Forward = RouteTangent(Angle).GetSafeNormal2D();
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        const float Side = (FenceIndex % 2 == 0) ? 1.0f : -1.0f;
        const FVector FenceCenter = RouteCenter + Right * ((RIART_RoadWidth * 0.5f + 1500.0f) * Side);
        const FRotator Rotation = Forward.Rotation();

        for (int32 PostIndex = -2; PostIndex <= 2; ++PostIndex)
        {
            const FVector Post = FenceCenter + Forward * (static_cast<float>(PostIndex) * 165.0f);
            AddInstance(BambooInstances, Post + FVector::UpVector * 72.0f, Rotation, FVector(0.075f, 0.075f, 1.44f));
        }

        AddInstance(BambooInstances, FenceCenter + FVector::UpVector * 55.0f, Rotation, FVector(6.8f, 0.065f, 0.065f));
        AddInstance(BambooInstances, FenceCenter + FVector::UpVector * 102.0f, Rotation, FVector(6.8f, 0.055f, 0.055f));
    }
}

void URIRoadsideArtSubsystem::TryBuild()
{
    UWorld* World = GetWorld();
    if (bBuilt || !World)
    {
        return;
    }

    CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    BasicMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!CubeMesh || !CylinderMesh || !BasicMaterial)
    {
        return;
    }

    ArtRoot = World->SpawnActor<AActor>();
    if (!ArtRoot)
    {
        return;
    }
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("RoadsideArtRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        ArtRoot = nullptr;
        return;
    }

    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    WoodInstances = CreateColorGroup(CubeMesh, RIART_WoodColor);
    BambooInstances = CreateColorGroup(CubeMesh, RIART_BambooColor);
    TrimInstances = CreateColorGroup(CubeMesh, RIART_TrimColor);
    DoorInstances = CreateColorGroup(CubeMesh, RIART_DoorColor);
    WindowInstances = CreateColorGroup(CubeMesh, RIART_WindowColor);
    MetalInstances = CreateColorGroup(CubeMesh, RIART_MetalColor);
    CrateInstances = CreateColorGroup(CubeMesh, RIART_CrateColor);
    BarrelInstances = CreateColorGroup(CylinderMesh, RIART_BarrelColor);

    if (!WoodInstances || !BambooInstances || !TrimInstances || !DoorInstances ||
        !WindowInstances || !MetalInstances || !CrateInstances || !BarrelInstances)
    {
        ArtRoot->Destroy();
        ArtRoot = nullptr;
        return;
    }

    // Optional decoration from the user's already-imported, approved free PN_Banana pack.
    UStaticMesh* GroundBananaAMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/PN_Banana/Meshes/props/groundBananas_01.groundBananas_01"));
    UStaticMesh* GroundBananaBMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/PN_Banana/Meshes/props/groundBananas_03.groundBananas_03"));
    UStaticMesh* RottenLeavesMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/PN_Banana/Meshes/props/rottenLeaves_01.rottenLeaves_01"));

    GroundBananaAInstances = CreateAssetGroup(GroundBananaAMesh);
    GroundBananaBInstances = CreateAssetGroup(GroundBananaBMesh);
    RottenLeavesInstances = CreateAssetGroup(RottenLeavesMesh);

    BuildClusterDetails();
    BuildFenceDetails();
    bBuilt = true;
}

void URIRoadsideArtSubsystem::Tick(const float DeltaTime)
{
    TryBuild();
}
