#include "Presentation/RIRoadsideThemeSubsystem.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float RITHRouteRadiusX = 9000.0f;
    constexpr float RITHRouteRadiusY = 5000.0f;
    constexpr float RITHRoadWidth = 1200.0f;
    constexpr int32 RITHRouteSegments = 40;

    const FLinearColor RITHDirtColor(0.34f, 0.24f, 0.13f, 1.0f);
    const FLinearColor RITHFieldColor(0.18f, 0.38f, 0.10f, 1.0f);
    const FLinearColor RITHWaterColor(0.055f, 0.24f, 0.34f, 1.0f);
    const FLinearColor RITHBrickColor(0.43f, 0.16f, 0.08f, 1.0f);
    const FLinearColor RITHPlasterColor(0.68f, 0.60f, 0.43f, 1.0f);
    const FLinearColor RITHTinBlueColor(0.08f, 0.32f, 0.48f, 1.0f);
    const FLinearColor RITHTinRedColor(0.62f, 0.12f, 0.06f, 1.0f);
    const FLinearColor RITHShopOrangeColor(0.87f, 0.38f, 0.035f, 1.0f);
    const FLinearColor RITHShopGreenColor(0.12f, 0.48f, 0.22f, 1.0f);
    const FLinearColor RITHDarkColor(0.055f, 0.065f, 0.070f, 1.0f);
    const FLinearColor RITHConcreteColor(0.30f, 0.31f, 0.29f, 1.0f);
    const FLinearColor RITHLeafColor(0.08f, 0.32f, 0.09f, 1.0f);
}

bool URIRoadsideThemeSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !bBuilt && !IsTemplate() && World && World->IsGameWorld();
}

TStatId URIRoadsideThemeSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URIRoadsideThemeSubsystem, STATGROUP_Tickables);
}

FVector URIRoadsideThemeSubsystem::RoutePoint(const float AngleRadians, const float Height) const
{
    return FVector(
        FMath::Cos(AngleRadians) * RITHRouteRadiusX,
        FMath::Sin(AngleRadians) * RITHRouteRadiusY,
        Height);
}

FVector URIRoadsideThemeSubsystem::RouteTangent(const float AngleRadians) const
{
    return FVector(
        -FMath::Sin(AngleRadians) * RITHRouteRadiusX,
        FMath::Cos(AngleRadians) * RITHRouteRadiusY,
        0.0f);
}

UInstancedStaticMeshComponent* URIRoadsideThemeSubsystem::CreateGroup(UStaticMesh* Mesh, const FLinearColor& Color)
{
    if (!ThemeRoot || !Mesh || !BasicMaterial)
    {
        return nullptr;
    }

    UInstancedStaticMeshComponent* Group = NewObject<UInstancedStaticMeshComponent>(ThemeRoot);
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

    if (USceneComponent* Root = ThemeRoot->GetRootComponent())
    {
        Group->SetupAttachment(Root);
    }

    if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BasicMaterial, Group))
    {
        Material->SetVectorParameterValue(TEXT("Color"), Color);
        Group->SetMaterial(0, Material);
    }

    ThemeRoot->AddInstanceComponent(Group);
    Group->RegisterComponent();
    return Group;
}

UInstancedStaticMeshComponent* URIRoadsideThemeSubsystem::CreateAssetGroup(UStaticMesh* Mesh)
{
    if (!ThemeRoot || !Mesh)
    {
        return nullptr;
    }

    UInstancedStaticMeshComponent* Group = NewObject<UInstancedStaticMeshComponent>(ThemeRoot);
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

    if (USceneComponent* Root = ThemeRoot->GetRootComponent())
    {
        Group->SetupAttachment(Root);
    }

    // Do not override materials here. Imported free-art meshes keep their authored
    // Fab materials/textures, unlike the colorized primitive fallback groups.
    ThemeRoot->AddInstanceComponent(Group);
    Group->RegisterComponent();
    return Group;
}

void URIRoadsideThemeSubsystem::AddInstance(
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

void URIRoadsideThemeSubsystem::AddAssetInstance(
    UInstancedStaticMeshComponent* Group,
    const FVector& Location,
    const FRotator& Rotation,
    const float DesiredHeightCm,
    const float UniformScaleMultiplier)
{
    if (!Group || DesiredHeightCm <= 0.0f)
    {
        return;
    }

    UStaticMesh* Mesh = Group->GetStaticMesh();
    if (!Mesh)
    {
        return;
    }

    const FBoxSphereBounds Bounds = Mesh->GetBounds();
    const float NativeHeight = Bounds.BoxExtent.Z * 2.0f;
    if (NativeHeight <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const float ScaleValue = (DesiredHeightCm / NativeHeight) * FMath::Max(0.05f, UniformScaleMultiplier);
    const float LocalBottomZ = Bounds.Origin.Z - Bounds.BoxExtent.Z;
    const FVector GroundedLocation = Location - FVector::UpVector * (LocalBottomZ * ScaleValue);
    AddInstance(Group, GroundedLocation, Rotation, FVector(ScaleValue));
}

void URIRoadsideThemeSubsystem::BuildFields()
{
    const int32 FieldRouteIndices[] = {5, 13, 21, 29, 35};
    for (int32 FieldIndex = 0; FieldIndex < UE_ARRAY_COUNT(FieldRouteIndices); ++FieldIndex)
    {
        const int32 RouteIndex = FieldRouteIndices[FieldIndex];
        const float Angle = 2.0f * PI * static_cast<float>(RouteIndex) / static_cast<float>(RITHRouteSegments);
        const FVector Center = RoutePoint(Angle, 0.0f);
        const FVector Forward = RouteTangent(Angle).GetSafeNormal2D();
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        const float Side = (FieldIndex % 2 == 0) ? 1.0f : -1.0f;
        const FVector PatchCenter = Center + Right * ((RITHRoadWidth * 0.5f + 2300.0f) * Side);
        const FRotator Rotation = Forward.Rotation();

        AddInstance(FieldInstances, FVector(PatchCenter.X, PatchCenter.Y, 2.0f), Rotation, FVector(18.0f, 14.0f, 0.012f));

        if (FieldIndex == 1 || FieldIndex == 3)
        {
            const FVector WaterCenter = PatchCenter + Right * (260.0f * Side) + Forward * 180.0f;
            AddInstance(WaterInstances, FVector(WaterCenter.X, WaterCenter.Y, 3.0f), Rotation, FVector(8.5f, 5.4f, 0.010f));
        }
    }
}

void URIRoadsideThemeSubsystem::BuildUtilityLines()
{
    // One continuous visual-only utility run avoids unsupported wire endpoints.
    constexpr int32 Step = 4;
    constexpr float PoleDistance = RITHRoadWidth * 0.5f + 1050.0f;
    constexpr float Side = 1.0f;

    for (int32 RouteIndex = 0; RouteIndex < RITHRouteSegments; RouteIndex += Step)
    {
        const int32 NextIndex = (RouteIndex + Step) % RITHRouteSegments;
        const float AngleA = 2.0f * PI * static_cast<float>(RouteIndex) / static_cast<float>(RITHRouteSegments);
        const float AngleB = 2.0f * PI * static_cast<float>(NextIndex) / static_cast<float>(RITHRouteSegments);

        const FVector RouteA = RoutePoint(AngleA);
        const FVector RouteB = RoutePoint(AngleB);
        const FVector ForwardA = RouteTangent(AngleA).GetSafeNormal2D();
        const FVector ForwardB = RouteTangent(AngleB).GetSafeNormal2D();
        const FVector RightA = FVector::CrossProduct(FVector::UpVector, ForwardA).GetSafeNormal();
        const FVector RightB = FVector::CrossProduct(FVector::UpVector, ForwardB).GetSafeNormal();

        const FVector PoleA = RouteA + RightA * (PoleDistance * Side);
        const FVector PoleB = RouteB + RightB * (PoleDistance * Side);
        const FRotator PoleRotation = ForwardA.Rotation();

        AddInstance(ConcreteInstances, FVector(PoleA.X, PoleA.Y, 260.0f), PoleRotation, FVector(0.22f, 0.22f, 5.2f));
        AddInstance(DarkInstances, FVector(PoleA.X, PoleA.Y, 510.0f), PoleRotation, FVector(0.16f, 1.15f, 0.12f));

        FVector LineDirection = PoleB - PoleA;
        LineDirection.Z = 0.0f;
        const float LineLength = LineDirection.Size();
        if (LineLength > 1.0f)
        {
            const FVector LineCenter = (PoleA + PoleB) * 0.5f + FVector::UpVector * 505.0f;
            AddInstance(DarkInstances, LineCenter, LineDirection.Rotation(), FVector(LineLength / 100.0f, 0.025f, 0.025f));
        }
    }
}

void URIRoadsideThemeSubsystem::BuildRoadsideClusters()
{
    // Reversible South-Asian roadside-inspired graybox clusters. Real free vegetation
    // now replaces the old ball-tree rows when the local Fab packs are available.
    const int32 ClusterIndices[] = {3, 9, 16, 24, 31, 37};
    for (int32 Cluster = 0; Cluster < UE_ARRAY_COUNT(ClusterIndices); ++Cluster)
    {
        const int32 RouteIndex = ClusterIndices[Cluster];
        const float Angle = 2.0f * PI * static_cast<float>(RouteIndex) / static_cast<float>(RITHRouteSegments);
        const FVector RouteCenter = RoutePoint(Angle);
        const FVector Forward = RouteTangent(Angle).GetSafeNormal2D();
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        const float Side = (Cluster % 2 == 0) ? -1.0f : 1.0f;
        const float BaseDistance = RITHRoadWidth * 0.5f + 1350.0f;
        const FVector Base = RouteCenter + Right * (BaseDistance * Side);
        const FRotator Rotation = Forward.Rotation();

        AddInstance(DirtInstances, FVector(Base.X, Base.Y, 2.2f), Rotation, FVector(11.0f, 7.5f, 0.014f));

        if (Cluster % 3 == 0)
        {
            AddInstance(BrickInstances, Base + FVector::UpVector * 105.0f, Rotation, FVector(4.4f, 2.9f, 2.1f));
            AddInstance(TinBlueInstances, Base + FVector::UpVector * 225.0f, FRotator(0.0f, Rotation.Yaw, 5.0f * Side), FVector(4.9f, 3.4f, 0.16f));
            AddInstance(ShopOrangeInstances, Base - Forward * 205.0f + FVector::UpVector * 115.0f, Rotation, FVector(0.12f, 2.1f, 0.55f));
            AddInstance(DarkInstances, Base - Forward * 225.0f + Right * 95.0f + FVector::UpVector * 62.0f, Rotation, FVector(0.12f, 0.12f, 1.25f));
            AddInstance(DarkInstances, Base - Forward * 225.0f - Right * 95.0f + FVector::UpVector * 62.0f, Rotation, FVector(0.12f, 0.12f, 1.25f));
        }
        else if (Cluster % 3 == 1)
        {
            AddInstance(PlasterInstances, Base + FVector::UpVector * 120.0f, Rotation, FVector(5.2f, 3.6f, 2.4f));
            AddInstance(TinRedInstances, Base + FVector::UpVector * 260.0f, FRotator(0.0f, Rotation.Yaw, -6.0f * Side), FVector(5.8f, 4.1f, 0.17f));
            AddInstance(ShopGreenInstances, Base - Forward * 255.0f + FVector::UpVector * 125.0f, Rotation, FVector(0.10f, 1.15f, 1.25f));
        }
        else
        {
            AddInstance(ConcreteInstances, Base + Right * 150.0f + FVector::UpVector * 115.0f, Rotation, FVector(0.18f, 0.18f, 2.3f));
            AddInstance(ConcreteInstances, Base - Right * 150.0f + FVector::UpVector * 115.0f, Rotation, FVector(0.18f, 0.18f, 2.3f));
            AddInstance(TinBlueInstances, Base + FVector::UpVector * 235.0f, Rotation, FVector(2.2f, 3.8f, 0.16f));
            AddInstance(DarkInstances, Base + FVector::UpVector * 55.0f, Rotation, FVector(2.8f, 0.35f, 0.30f));
        }

        if (!BananaTallInstances && !BananaMediumInstances)
        {
            for (int32 Tree = 0; Tree < 3; ++Tree)
            {
                const float Across = (static_cast<float>(Tree) - 1.0f) * 230.0f;
                const FVector TreeBase = Base + Right * Across + Forward * (430.0f + Tree * 65.0f);
                AddInstance(DarkInstances, TreeBase + FVector::UpVector * 120.0f, FRotator::ZeroRotator, FVector(0.22f, 0.22f, 2.4f));
                AddInstance(LeafInstances, TreeBase + FVector::UpVector * 300.0f, FRotator::ZeroRotator, FVector(0.95f, 0.85f, 1.15f));
            }
        }
    }
}

void URIRoadsideThemeSubsystem::BuildRealVegetation()
{
    if (!BananaTallInstances && !BananaMediumInstances && !GroundPlantTallInstances && !GroundPlantLowInstances)
    {
        return;
    }

    // VPR-20.1: denser but still deterministic vegetation. Everything remains well
    // beyond the barrier and uses instancing, so this does not affect gameplay or actor count.
    const int32 VegetationIndices[] =
    {
        0, 2, 4, 6, 8, 10, 12, 14, 16, 18,
        20, 22, 24, 26, 28, 30, 32, 34, 36, 38
    };

    for (int32 Site = 0; Site < UE_ARRAY_COUNT(VegetationIndices); ++Site)
    {
        const int32 RouteIndex = VegetationIndices[Site];
        const float Angle = 2.0f * PI * static_cast<float>(RouteIndex) / static_cast<float>(RITHRouteSegments);
        const FVector RouteCenter = RoutePoint(Angle);
        const FVector Forward = RouteTangent(Angle).GetSafeNormal2D();
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        const float Side = (Site % 2 == 0) ? 1.0f : -1.0f;

        const float DepthVariation = static_cast<float>((Site * 137) % 430);
        const float Distance = RITHRoadWidth * 0.5f + 1120.0f + DepthVariation;
        const FVector Base = RouteCenter + Right * (Distance * Side);
        const float YawVariation = static_cast<float>((Site * 53) % 150) - 75.0f;
        const FRotator PlantRotation(0.0f, Forward.Rotation().Yaw + YawVariation, 0.0f);

        UInstancedStaticMeshComponent* PrimaryBanana = ((Site % 3 == 0) && BananaMediumInstances)
            ? BananaMediumInstances
            : BananaTallInstances;
        if (!PrimaryBanana)
        {
            PrimaryBanana = BananaMediumInstances;
        }

        const float PrimaryHeight = 330.0f + 24.0f * static_cast<float>(Site % 5);
        AddAssetInstance(
            PrimaryBanana,
            Base,
            PlantRotation,
            PrimaryHeight,
            0.94f + 0.03f * static_cast<float>(Site % 4));

        // Roughly half the sites get a second, smaller banana so the landscape reads
        // as clusters/gardens rather than identical single trees placed on a spline.
        if ((Site % 2 == 0) && (BananaMediumInstances || BananaTallInstances))
        {
            UInstancedStaticMeshComponent* SecondaryBanana = BananaMediumInstances ? BananaMediumInstances : BananaTallInstances;
            const FVector SecondaryBase = Base + Forward * (220.0f + 25.0f * static_cast<float>(Site % 3)) + Right * (190.0f * Side);
            AddAssetInstance(
                SecondaryBanana,
                SecondaryBase,
                FRotator(0.0f, PlantRotation.Yaw + 72.0f, 0.0f),
                255.0f + 18.0f * static_cast<float>(Site % 4),
                0.92f);
        }

        const FVector GroundA = Base + Forward * 135.0f + Right * (95.0f * Side);
        const FVector GroundB = Base - Forward * 155.0f - Right * (115.0f * Side);
        const FVector GroundC = Base + Forward * 305.0f - Right * (165.0f * Side);
        const FVector GroundD = Base - Forward * 315.0f + Right * (145.0f * Side);

        AddAssetInstance(
            GroundPlantTallInstances,
            GroundA,
            FRotator(0.0f, PlantRotation.Yaw + 63.0f, 0.0f),
            96.0f + 7.0f * static_cast<float>(Site % 4),
            0.90f);
        AddAssetInstance(
            GroundPlantLowInstances,
            GroundB,
            FRotator(0.0f, PlantRotation.Yaw - 44.0f, 0.0f),
            62.0f + 5.0f * static_cast<float>(Site % 3),
            0.92f);
        AddAssetInstance(
            GroundPlantLowInstances,
            GroundC,
            FRotator(0.0f, PlantRotation.Yaw + 121.0f, 0.0f),
            58.0f + 4.0f * static_cast<float>((Site + 1) % 4),
            0.88f);

        if (Site % 3 != 1)
        {
            AddAssetInstance(
                GroundPlantTallInstances,
                GroundD,
                FRotator(0.0f, PlantRotation.Yaw - 103.0f, 0.0f),
                82.0f + 6.0f * static_cast<float>(Site % 3),
                0.86f);
        }
    }
}

void URIRoadsideThemeSubsystem::TryBuild()
{
    UWorld* World = GetWorld();
    if (bBuilt || !World)
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

    ThemeRoot = World->SpawnActor<AActor>();
    if (!ThemeRoot)
    {
        return;
    }
    ThemeRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ThemeRoot, TEXT("RoadsideThemeRoot"));
    if (!Root)
    {
        ThemeRoot->Destroy();
        ThemeRoot = nullptr;
        return;
    }

    ThemeRoot->SetRootComponent(Root);
    ThemeRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    DirtInstances = CreateGroup(CubeMesh, RITHDirtColor);
    FieldInstances = CreateGroup(CubeMesh, RITHFieldColor);
    WaterInstances = CreateGroup(CubeMesh, RITHWaterColor);
    BrickInstances = CreateGroup(CubeMesh, RITHBrickColor);
    PlasterInstances = CreateGroup(CubeMesh, RITHPlasterColor);
    TinBlueInstances = CreateGroup(CubeMesh, RITHTinBlueColor);
    TinRedInstances = CreateGroup(CubeMesh, RITHTinRedColor);
    ShopOrangeInstances = CreateGroup(CubeMesh, RITHShopOrangeColor);
    ShopGreenInstances = CreateGroup(CubeMesh, RITHShopGreenColor);
    DarkInstances = CreateGroup(CubeMesh, RITHDarkColor);
    ConcreteInstances = CreateGroup(CubeMesh, RITHConcreteColor);
    LeafInstances = CreateGroup(SphereMesh, RITHLeafColor);

    if (!DirtInstances || !FieldInstances || !WaterInstances || !BrickInstances || !PlasterInstances ||
        !TinBlueInstances || !TinRedInstances || !ShopOrangeInstances || !ShopGreenInstances ||
        !DarkInstances || !ConcreteInstances || !LeafInstances)
    {
        ThemeRoot->Destroy();
        ThemeRoot = nullptr;
        return;
    }

    // These packs are local/free-only presentation dependencies. If any are missing,
    // the theme still builds using the primitive fallback groups above.
    UStaticMesh* BananaTallMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/PN_Banana/Meshes/plants/banana_01_07.banana_01_07"));
    UStaticMesh* BananaMediumMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/PN_Banana/Meshes/plants/banana_02_05.banana_02_05"));
    UStaticMesh* GroundPlantTallMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_01_04.tropicalPlant_01_04"));
    UStaticMesh* GroundPlantLowMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/PN_tropicalGroundPlants/Meshes/tropicalPlant_05_04.tropicalPlant_05_04"));

    BananaTallInstances = CreateAssetGroup(BananaTallMesh);
    BananaMediumInstances = CreateAssetGroup(BananaMediumMesh);
    GroundPlantTallInstances = CreateAssetGroup(GroundPlantTallMesh);
    GroundPlantLowInstances = CreateAssetGroup(GroundPlantLowMesh);

    BuildFields();
    BuildUtilityLines();
    BuildRoadsideClusters();
    BuildRealVegetation();
    bBuilt = true;
}

void URIRoadsideThemeSubsystem::Tick(const float DeltaTime)
{
    TryBuild();
}
