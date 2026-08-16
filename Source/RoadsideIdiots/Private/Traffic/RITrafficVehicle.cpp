#include "Traffic/RITrafficVehicle.h"

#include "Vehicle/RIBikePawn.h"
#include "Core/RIHealthComponent.h"
#include "Visual/RIPrototypeVisuals.h"
#include "Audio/RIAudioEvents.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    constexpr float RouteRadiusX = 9000.0f;
    constexpr float RouteRadiusY = 5000.0f;
    constexpr float RouteHeight = 62.0f;

    FVector RouteCenterAt(const float AngleRadians)
    {
        return FVector(
            FMath::Cos(AngleRadians) * RouteRadiusX,
            FMath::Sin(AngleRadians) * RouteRadiusY,
            RouteHeight);
    }

    FVector RouteTangentAt(const float AngleRadians)
    {
        return FVector(
            -FMath::Sin(AngleRadians) * RouteRadiusX,
            FMath::Cos(AngleRadians) * RouteRadiusY,
            0.0f);
    }
}

ARITrafficVehicle::ARITrafficVehicle()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = false;

    ImpactVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ImpactVolume"));
    SetRootComponent(ImpactVolume);
    ImpactVolume->SetBoxExtent(FVector(170.0f, 82.0f, 60.0f));
    ImpactVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ImpactVolume->SetCollisionObjectType(ECC_WorldDynamic);
    ImpactVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    ImpactVolume->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    ImpactVolume->SetGenerateOverlapEvents(true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

    BodyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyVisual"));
    BodyVisual->SetupAttachment(ImpactVolume);
    BodyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BodyVisual->SetRelativeLocation(FVector(0.0f, 0.0f, -12.0f));
    BodyVisual->SetRelativeScale3D(FVector(3.15f, 1.50f, 0.48f));

    CabinVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CabinVisual"));
    CabinVisual->SetupAttachment(ImpactVolume);
    CabinVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CabinVisual->SetRelativeLocation(FVector(-24.0f, 0.0f, 38.0f));
    CabinVisual->SetRelativeScale3D(FVector(1.50f, 1.24f, 0.52f));

    FrontMarkerLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontMarkerLeft"));
    FrontMarkerLeft->SetupAttachment(ImpactVolume);
    FrontMarkerLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FrontMarkerLeft->SetRelativeLocation(FVector(154.0f, -52.0f, -8.0f));
    FrontMarkerLeft->SetRelativeScale3D(FVector(0.10f));

    FrontMarkerRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontMarkerRight"));
    FrontMarkerRight->SetupAttachment(ImpactVolume);
    FrontMarkerRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FrontMarkerRight->SetRelativeLocation(FVector(154.0f, 52.0f, -8.0f));
    FrontMarkerRight->SetRelativeScale3D(FVector(0.10f));

    RearMarkerLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearMarkerLeft"));
    RearMarkerLeft->SetupAttachment(ImpactVolume);
    RearMarkerLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RearMarkerLeft->SetRelativeLocation(FVector(-154.0f, -52.0f, -8.0f));
    RearMarkerLeft->SetRelativeScale3D(FVector(0.09f));

    RearMarkerRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearMarkerRight"));
    RearMarkerRight->SetupAttachment(ImpactVolume);
    RearMarkerRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RearMarkerRight->SetRelativeLocation(FVector(-154.0f, 52.0f, -8.0f));
    RearMarkerRight->SetRelativeScale3D(FVector(0.09f));

    auto SetupWheel = [this](UStaticMeshComponent* Wheel, const FVector& Location)
    {
        Wheel->SetupAttachment(ImpactVolume);
        Wheel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Wheel->SetRelativeLocation(Location);
        Wheel->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
        Wheel->SetRelativeScale3D(FVector(0.54f, 0.54f, 0.18f));
    };

    FrontWheelLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontWheelLeft"));
    SetupWheel(FrontWheelLeft, FVector(104.0f, -78.0f, -43.0f));
    FrontWheelRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontWheelRight"));
    SetupWheel(FrontWheelRight, FVector(104.0f, 78.0f, -43.0f));
    RearWheelLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearWheelLeft"));
    SetupWheel(RearWheelLeft, FVector(-104.0f, -78.0f, -43.0f));
    RearWheelRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearWheelRight"));
    SetupWheel(RearWheelRight, FVector(-104.0f, 78.0f, -43.0f));

    if (CubeMesh.Succeeded())
    {
        BodyVisual->SetStaticMesh(CubeMesh.Object);
        CabinVisual->SetStaticMesh(CubeMesh.Object);
    }
    if (SphereMesh.Succeeded())
    {
        FrontMarkerLeft->SetStaticMesh(SphereMesh.Object);
        FrontMarkerRight->SetStaticMesh(SphereMesh.Object);
        RearMarkerLeft->SetStaticMesh(SphereMesh.Object);
        RearMarkerRight->SetStaticMesh(SphereMesh.Object);
    }
    if (CylinderMesh.Succeeded())
    {
        FrontWheelLeft->SetStaticMesh(CylinderMesh.Object);
        FrontWheelRight->SetStaticMesh(CylinderMesh.Object);
        RearWheelLeft->SetStaticMesh(CylinderMesh.Object);
        RearWheelRight->SetStaticMesh(CylinderMesh.Object);
    }
}

FTransform ARITrafficVehicle::MakeRouteTransform(const float AngleRadians, const float LaneOffset)
{
    const FVector Center = RouteCenterAt(AngleRadians);
    const FVector Tangent = RouteTangentAt(AngleRadians).GetSafeNormal2D();
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();
    const FVector Location = Center + Right * LaneOffset;
    return FTransform(Tangent.Rotation(), Location);
}

void ARITrafficVehicle::Configure(
    const float InStartAngleRadians,
    const float InSpeedKph,
    const float InLaneOffset,
    const FLinearColor& InBodyColor,
    const bool bInWanders,
    const float InWanderPhase,
    const FString& InTrafficLabel)
{
    RouteAngleRadians = InStartAngleRadians;
    SpeedCms = FMath::Max(100.0f, InSpeedKph / 0.036f);
    BaseLaneOffset = InLaneOffset;
    BodyColor = InBodyColor;
    bWanders = bInWanders;
    WanderPhase = InWanderPhase;
    TrafficLabel = InTrafficLabel;

    // Reset the generic sedan silhouette before applying a per-vehicle style.
    ImpactVolume->SetBoxExtent(FVector(170.0f, 82.0f, 60.0f));
    BodyVisual->SetRelativeLocation(FVector(0.0f, 0.0f, -12.0f));
    BodyVisual->SetRelativeScale3D(FVector(3.15f, 1.50f, 0.48f));
    CabinVisual->SetRelativeLocation(FVector(-24.0f, 0.0f, 38.0f));
    CabinVisual->SetRelativeScale3D(FVector(1.50f, 1.24f, 0.52f));

    FrontWheelLeft->SetVisibility(true, true);
    FrontWheelRight->SetVisibility(true, true);
    RearWheelLeft->SetVisibility(true, true);
    RearWheelRight->SetVisibility(true, true);

    FrontWheelLeft->SetRelativeLocation(FVector(104.0f, -78.0f, -43.0f));
    FrontWheelRight->SetRelativeLocation(FVector(104.0f, 78.0f, -43.0f));
    RearWheelLeft->SetRelativeLocation(FVector(-104.0f, -78.0f, -43.0f));
    RearWheelRight->SetRelativeLocation(FVector(-104.0f, 78.0f, -43.0f));
    FrontWheelLeft->SetRelativeScale3D(FVector(0.54f, 0.54f, 0.18f));
    FrontWheelRight->SetRelativeScale3D(FVector(0.54f, 0.54f, 0.18f));
    RearWheelLeft->SetRelativeScale3D(FVector(0.54f, 0.54f, 0.18f));
    RearWheelRight->SetRelativeScale3D(FVector(0.54f, 0.54f, 0.18f));

    if (TrafficLabel.Equals(TEXT("CNG AUTO"), ESearchCase::IgnoreCase))
    {
        // Three-wheeler silhouette made only from existing Engine basic shapes:
        // one centered front wheel, two rear wheels and a tall compact cabin.
        ImpactVolume->SetBoxExtent(FVector(118.0f, 64.0f, 76.0f));
        BodyVisual->SetRelativeLocation(FVector(-4.0f, 0.0f, -8.0f));
        BodyVisual->SetRelativeScale3D(FVector(1.72f, 0.96f, 0.50f));
        CabinVisual->SetRelativeLocation(FVector(-20.0f, 0.0f, 42.0f));
        CabinVisual->SetRelativeScale3D(FVector(1.20f, 0.88f, 0.82f));

        FrontWheelLeft->SetRelativeLocation(FVector(82.0f, 0.0f, -38.0f));
        FrontWheelLeft->SetRelativeScale3D(FVector(0.42f, 0.42f, 0.16f));
        FrontWheelRight->SetVisibility(false, true);
        RearWheelLeft->SetRelativeLocation(FVector(-64.0f, -54.0f, -38.0f));
        RearWheelRight->SetRelativeLocation(FVector(-64.0f, 54.0f, -38.0f));
        RearWheelLeft->SetRelativeScale3D(FVector(0.44f, 0.44f, 0.16f));
        RearWheelRight->SetRelativeScale3D(FVector(0.44f, 0.44f, 0.16f));

        FrontMarkerLeft->SetRelativeLocation(FVector(90.0f, -24.0f, -4.0f));
        FrontMarkerRight->SetRelativeLocation(FVector(90.0f, 24.0f, -4.0f));
        RearMarkerLeft->SetRelativeLocation(FVector(-88.0f, -30.0f, -4.0f));
        RearMarkerRight->SetRelativeLocation(FVector(-88.0f, 30.0f, -4.0f));
    }
    else if (TrafficLabel.Equals(TEXT("DELIVERY VAN"), ESearchCase::IgnoreCase))
    {
        ImpactVolume->SetBoxExtent(FVector(188.0f, 82.0f, 78.0f));
        BodyVisual->SetRelativeLocation(FVector(-4.0f, 0.0f, -4.0f));
        BodyVisual->SetRelativeScale3D(FVector(3.45f, 1.52f, 0.64f));
        CabinVisual->SetRelativeLocation(FVector(-42.0f, 0.0f, 50.0f));
        CabinVisual->SetRelativeScale3D(FVector(2.10f, 1.30f, 0.72f));
    }
    else if (TrafficLabel.Equals(TEXT("MICROBUS"), ESearchCase::IgnoreCase))
    {
        ImpactVolume->SetBoxExtent(FVector(190.0f, 84.0f, 82.0f));
        BodyVisual->SetRelativeLocation(FVector(-2.0f, 0.0f, -2.0f));
        BodyVisual->SetRelativeScale3D(FVector(3.55f, 1.56f, 0.68f));
        CabinVisual->SetRelativeLocation(FVector(-18.0f, 0.0f, 52.0f));
        CabinVisual->SetRelativeScale3D(FVector(2.45f, 1.34f, 0.78f));
    }
    else if (TrafficLabel.Equals(TEXT("SUNDAY DRIVER"), ESearchCase::IgnoreCase))
    {
        ImpactVolume->SetBoxExtent(FVector(150.0f, 76.0f, 58.0f));
        BodyVisual->SetRelativeScale3D(FVector(2.72f, 1.36f, 0.44f));
        CabinVisual->SetRelativeLocation(FVector(-18.0f, 0.0f, 35.0f));
        CabinVisual->SetRelativeScale3D(FVector(1.30f, 1.12f, 0.48f));
        FrontWheelLeft->SetRelativeLocation(FVector(88.0f, -68.0f, -40.0f));
        FrontWheelRight->SetRelativeLocation(FVector(88.0f, 68.0f, -40.0f));
        RearWheelLeft->SetRelativeLocation(FVector(-88.0f, -68.0f, -40.0f));
        RearWheelRight->SetRelativeLocation(FVector(-88.0f, 68.0f, -40.0f));
    }
    else if (TrafficLabel.Equals(TEXT("LOST DRIVER"), ESearchCase::IgnoreCase))
    {
        ImpactVolume->SetBoxExtent(FVector(156.0f, 78.0f, 62.0f));
        BodyVisual->SetRelativeScale3D(FVector(2.85f, 1.40f, 0.46f));
        CabinVisual->SetRelativeLocation(FVector(-36.0f, 0.0f, 38.0f));
        CabinVisual->SetRelativeScale3D(FVector(1.42f, 1.18f, 0.56f));
    }
}

void ARITrafficVehicle::BeginPlay()
{
    Super::BeginPlay();
    ImpactVolume->OnComponentBeginOverlap.AddDynamic(this, &ARITrafficVehicle::HandleImpactOverlap);
    ApplyVisualMaterials();
}

void ARITrafficVehicle::ApplyVisualMaterials()
{
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!BaseMaterial) return;

    if (UMaterialInstanceDynamic* BodyMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, BodyVisual))
    {
        BodyMaterial->SetVectorParameterValue(TEXT("Color"), BodyColor);
        BodyVisual->SetMaterial(0, BodyMaterial);
    }

    if (UMaterialInstanceDynamic* CabinMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, CabinVisual))
    {
        CabinMaterial->SetVectorParameterValue(
            TEXT("Color"),
            FLinearColor(BodyColor.R * 0.48f, BodyColor.G * 0.48f, BodyColor.B * 0.48f, 1.0f));
        CabinVisual->SetMaterial(0, CabinMaterial);
    }

    if (UMaterialInstanceDynamic* LampMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, FrontMarkerLeft))
    {
        LampMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.90f, 0.36f, 1.0f));
        FrontMarkerLeft->SetMaterial(0, LampMaterial);
        FrontMarkerRight->SetMaterial(0, LampMaterial);
    }

    if (UMaterialInstanceDynamic* RearLampMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, RearMarkerLeft))
    {
        RearLampMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.85f, 0.025f, 0.015f, 1.0f));
        RearMarkerLeft->SetMaterial(0, RearLampMaterial);
        RearMarkerRight->SetMaterial(0, RearLampMaterial);
    }

    if (UMaterialInstanceDynamic* TireMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, FrontWheelLeft))
    {
        TireMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.025f, 0.028f, 0.032f, 1.0f));
        FrontWheelLeft->SetMaterial(0, TireMaterial);
        FrontWheelRight->SetMaterial(0, TireMaterial);
        RearWheelLeft->SetMaterial(0, TireMaterial);
        RearWheelRight->SetMaterial(0, TireMaterial);
    }
}

void ARITrafficVehicle::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    const FVector Derivative = RouteTangentAt(RouteAngleRadians);
    const float LocalRadius = FMath::Max(1.0f, Derivative.Size2D());
    RouteAngleRadians = FMath::Fmod(RouteAngleRadians + (SpeedCms / LocalRadius) * DeltaSeconds, 2.0f * PI);
    if (RouteAngleRadians < 0.0f)
    {
        RouteAngleRadians += 2.0f * PI;
    }

    float LaneOffset = BaseLaneOffset;
    if (bWanders && GetWorld())
    {
        // Slow, low-amplitude drift reads like an imperfect civilian driver.
        // The previous ~1.9 m side-to-side swing could feel like traffic was
        // deliberately swerving into the player rather than making a mistake.
        const bool bCng = TrafficLabel.Equals(TEXT("CNG AUTO"), ESearchCase::IgnoreCase);
        const float WanderAmplitude = bCng ? 58.0f : 42.0f;
        const float WanderRate = bCng ? 0.50f : 0.43f;
        LaneOffset += FMath::Sin(GetWorld()->GetTimeSeconds() * WanderRate + WanderPhase) * WanderAmplitude;
    }

    SetActorTransform(MakeRouteTransform(RouteAngleRadians, LaneOffset), false, nullptr, ETeleportType::TeleportPhysics);
}

void ARITrafficVehicle::HandleImpactOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    ARIBikePawn* Bike = Cast<ARIBikePawn>(OtherActor);
    if (!Bike || !GetWorld() || !Bike->AreRaceControlsEnabled()) return;

    const double Now = GetWorld()->GetTimeSeconds();
    const TWeakObjectPtr<ARIBikePawn> BikeKey(Bike);
    if (const double* LastTime = LastImpactTimes.Find(BikeKey))
    {
        if (Now - *LastTime < 1.25)
        {
            return;
        }
    }
    LastImpactTimes.Add(BikeKey, Now);

    FVector Away = Bike->GetActorLocation() - GetActorLocation();
    Away.Z = 0.0f;
    if (Away.IsNearlyZero())
    {
        Away = GetActorRightVector();
    }
    Away.Normalize();

    const float Side = FVector::DotProduct(Away, Bike->GetActorRightVector()) >= 0.0f ? 1.0f : -1.0f;

    float ImpactScale = 1.0f;
    if (TrafficLabel.Equals(TEXT("CNG AUTO"), ESearchCase::IgnoreCase)) ImpactScale = 0.78f;
    else if (TrafficLabel.Equals(TEXT("DELIVERY VAN"), ESearchCase::IgnoreCase)) ImpactScale = 1.08f;
    else if (TrafficLabel.Equals(TEXT("MICROBUS"), ESearchCase::IgnoreCase)) ImpactScale = 1.16f;
    else if (TrafficLabel.Equals(TEXT("SUNDAY DRIVER"), ESearchCase::IgnoreCase)) ImpactScale = 0.92f;

    if (UStaticMeshComponent* Chassis = Bike->GetChassis())
    {
        Chassis->AddImpulse(
            (Away * 260.0f + GetActorForwardVector() * 90.0f) * ImpactScale,
            NAME_None,
            true);
        Chassis->AddAngularImpulseInRadians(
            (Bike->GetActorForwardVector() * (Side * 2.2f) + FVector::UpVector * (Side * 0.75f)) * ImpactScale,
            NAME_None,
            true);
    }

    if (URIHealthComponent* Health = Bike->GetHealthComponent())
    {
        Health->ApplyImpact(6.0f * ImpactScale);
    }

    float HonkPitch = FMath::FRandRange(0.94f, 1.06f);
    if (TrafficLabel.Equals(TEXT("CNG AUTO"), ESearchCase::IgnoreCase)) HonkPitch *= 1.16f;
    else if (TrafficLabel.Equals(TEXT("MICROBUS"), ESearchCase::IgnoreCase)) HonkPitch *= 0.86f;

    RIAudioEvents::Play(this, TEXT("Honk"), GetActorLocation(), 1.0f, HonkPitch);
    RIAudioEvents::Play(this, TEXT("TrafficHit"), Bike->GetActorLocation(), 0.95f, FMath::FRandRange(0.94f, 1.04f));
    Bike->TriggerComicImpact(Side, TEXT("HONK!"), 0.85f);
    RIPrototypeVisuals::PlayReaction(Bike, Side);
}
