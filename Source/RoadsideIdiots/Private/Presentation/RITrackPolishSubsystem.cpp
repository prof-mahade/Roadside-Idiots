#include "Presentation/RITrackPolishSubsystem.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float RITP2RouteRadiusX = 9000.0f;
    constexpr float RITP2RouteRadiusY = 5000.0f;
    constexpr float RITP2RoadWidth = 1200.0f;
    constexpr int32 RITP2RouteSegments = 40;

    const FLinearColor RITP2AsphaltColor(0.070f, 0.075f, 0.085f, 1.0f);
    const FLinearColor RITP2BarrierInsetColor(0.16f, 0.19f, 0.23f, 1.0f);
    const FLinearColor RITP2ChevronYellow(0.86f, 0.50f, 0.045f, 1.0f);
    const FLinearColor RITP2ChevronDark(0.045f, 0.055f, 0.065f, 1.0f);
}

bool URITrackPolishSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !bBuilt && !IsTemplate() && World && World->IsGameWorld();
}

TStatId URITrackPolishSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URITrackPolishSubsystem, STATGROUP_Tickables);
}

FVector URITrackPolishSubsystem::RoutePoint(const float AngleRadians, const float Height) const
{
    return FVector(
        FMath::Cos(AngleRadians) * RITP2RouteRadiusX,
        FMath::Sin(AngleRadians) * RITP2RouteRadiusY,
        Height);
}

FVector URITrackPolishSubsystem::RouteTangent(const float AngleRadians) const
{
    return FVector(
        -FMath::Sin(AngleRadians) * RITP2RouteRadiusX,
        FMath::Cos(AngleRadians) * RITP2RouteRadiusY,
        0.0f);
}

UInstancedStaticMeshComponent* URITrackPolishSubsystem::CreateGroup(UStaticMesh* Mesh, const FLinearColor& Color)
{
    if (!PolishRoot || !Mesh || !BasicMaterial)
    {
        return nullptr;
    }

    UInstancedStaticMeshComponent* Group = NewObject<UInstancedStaticMeshComponent>(PolishRoot);
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

    if (USceneComponent* RootComponent = PolishRoot->GetRootComponent())
    {
        Group->SetupAttachment(RootComponent);
    }

    if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BasicMaterial, Group))
    {
        Material->SetVectorParameterValue(TEXT("Color"), Color);
        Group->SetMaterial(0, Material);
    }

    PolishRoot->AddInstanceComponent(Group);
    Group->RegisterComponent();
    return Group;
}

void URITrackPolishSubsystem::AddInstance(
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

void URITrackPolishSubsystem::TryBuild()
{
    UWorld* World = GetWorld();
    if (bBuilt || !World)
    {
        return;
    }

    CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    BasicMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!CubeMesh || !BasicMaterial)
    {
        return;
    }

    PolishRoot = World->SpawnActor<AActor>();
    if (!PolishRoot)
    {
        return;
    }
    PolishRoot->SetActorEnableCollision(false);

    USceneComponent* RootComponent = NewObject<USceneComponent>(PolishRoot, TEXT("TrackPolishRoot"));
    if (!RootComponent)
    {
        PolishRoot->Destroy();
        PolishRoot = nullptr;
        return;
    }

    PolishRoot->SetRootComponent(RootComponent);
    PolishRoot->AddInstanceComponent(RootComponent);
    RootComponent->RegisterComponent();

    AsphaltJoinInstances = CreateGroup(CubeMesh, RITP2AsphaltColor);
    BarrierInsetInstances = CreateGroup(CubeMesh, RITP2BarrierInsetColor);
    ChevronYellowInstances = CreateGroup(CubeMesh, RITP2ChevronYellow);
    ChevronDarkInstances = CreateGroup(CubeMesh, RITP2ChevronDark);

    if (!AsphaltJoinInstances || !BarrierInsetInstances || !ChevronYellowInstances || !ChevronDarkInstances)
    {
        PolishRoot->Destroy();
        PolishRoot = nullptr;
        return;
    }

    // Bridge the chord joins between the 40 straight visual road segments. The
    // underlying road collision is untouched; these patches only hide the small
    // green triangular gaps that were visible on tighter bends.
    for (int32 Index = 0; Index < RITP2RouteSegments; ++Index)
    {
        const float Angle = 2.0f * PI * static_cast<float>(Index) / static_cast<float>(RITP2RouteSegments);
        const FVector JoinCenter = RoutePoint(Angle, 2.75f);
        const FRotator JoinRotation = RouteTangent(Angle).GetSafeNormal2D().Rotation();
        AddInstance(
            AsphaltJoinInstances,
            JoinCenter,
            JoinRotation,
            FVector(3.25f, RITP2RoadWidth / 100.0f, 0.014f));
    }

    // Dark inset strips leave only a thin yellow rim on the barrier caps. This
    // keeps the existing safety-color language while removing the glowing-slab look.
    for (int32 Index = 0; Index < RITP2RouteSegments; ++Index)
    {
        const float AngleA = 2.0f * PI * static_cast<float>(Index) / static_cast<float>(RITP2RouteSegments);
        const float AngleB = 2.0f * PI * static_cast<float>(Index + 1) / static_cast<float>(RITP2RouteSegments);
        const FVector A = RoutePoint(AngleA);
        const FVector B = RoutePoint(AngleB);

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
        const float BarrierOffset = RITP2RoadWidth * 0.5f + 28.0f;

        for (const float Side : {-1.0f, 1.0f})
        {
            const FVector BarrierCenter = Center + Right * (BarrierOffset * Side);
            AddInstance(
                BarrierInsetInstances,
                FVector(BarrierCenter.X, BarrierCenter.Y, 125.2f),
                Rotation,
                FVector((Length + 232.0f) / 100.0f, 0.50f, 0.026f));
        }
    }

    // A few simple non-colliding chevron boards make long curves easier to read
    // at speed without filling the course with more actors.
    for (int32 Marker = 0; Marker < 10; ++Marker)
    {
        const int32 RouteIndex = 1 + Marker * 4;
        const float Angle = 2.0f * PI * static_cast<float>(RouteIndex) / static_cast<float>(RITP2RouteSegments);
        const FVector Center = RoutePoint(Angle);
        const FVector Forward = RouteTangent(Angle).GetSafeNormal2D();
        const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        const float Side = (Marker % 2 == 0) ? 1.0f : -1.0f;
        const FVector Base = Center + Right * ((RITP2RoadWidth * 0.5f + 420.0f) * Side);
        const FRotator Rotation = Forward.Rotation();

        AddInstance(
            ChevronDarkInstances,
            FVector(Base.X, Base.Y, 82.0f),
            Rotation,
            FVector(0.12f, 0.12f, 1.64f));

        AddInstance(
            ChevronYellowInstances,
            FVector(Base.X, Base.Y, 172.0f),
            Rotation,
            FVector(0.14f, 0.72f, 0.32f));
    }

    bBuilt = true;
}

void URITrackPolishSubsystem::Tick(const float DeltaTime)
{
    TryBuild();
}
