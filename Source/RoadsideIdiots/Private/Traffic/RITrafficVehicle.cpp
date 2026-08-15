#include "Traffic/RITrafficVehicle.h"

#include "Vehicle/RIBikePawn.h"
#include "Core/RIHealthComponent.h"
#include "Visual/RIPrototypeVisuals.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
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
    ImpactVolume->SetBoxExtent(FVector(190.0f, 95.0f, 72.0f));
    ImpactVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ImpactVolume->SetCollisionObjectType(ECC_WorldDynamic);
    ImpactVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    ImpactVolume->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    ImpactVolume->SetGenerateOverlapEvents(true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

    BodyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyVisual"));
    BodyVisual->SetupAttachment(ImpactVolume);
    BodyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BodyVisual->SetRelativeLocation(FVector(0.0f, 0.0f, -6.0f));
    BodyVisual->SetRelativeScale3D(FVector(3.65f, 1.75f, 0.72f));

    CabinVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CabinVisual"));
    CabinVisual->SetupAttachment(ImpactVolume);
    CabinVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CabinVisual->SetRelativeLocation(FVector(-28.0f, 0.0f, 68.0f));
    CabinVisual->SetRelativeScale3D(FVector(1.85f, 1.48f, 0.72f));

    FrontMarkerLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontMarkerLeft"));
    FrontMarkerLeft->SetupAttachment(ImpactVolume);
    FrontMarkerLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FrontMarkerLeft->SetRelativeLocation(FVector(182.0f, -48.0f, -5.0f));
    FrontMarkerLeft->SetRelativeScale3D(FVector(0.14f));

    FrontMarkerRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontMarkerRight"));
    FrontMarkerRight->SetupAttachment(ImpactVolume);
    FrontMarkerRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FrontMarkerRight->SetRelativeLocation(FVector(182.0f, 48.0f, -5.0f));
    FrontMarkerRight->SetRelativeScale3D(FVector(0.14f));

    if (CubeMesh.Succeeded())
    {
        BodyVisual->SetStaticMesh(CubeMesh.Object);
        CabinVisual->SetStaticMesh(CubeMesh.Object);
    }
    if (SphereMesh.Succeeded())
    {
        FrontMarkerLeft->SetStaticMesh(SphereMesh.Object);
        FrontMarkerRight->SetStaticMesh(SphereMesh.Object);
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
            FLinearColor(BodyColor.R * 0.55f, BodyColor.G * 0.55f, BodyColor.B * 0.55f, 1.0f));
        CabinVisual->SetMaterial(0, CabinMaterial);
    }

    if (UMaterialInstanceDynamic* LampMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, FrontMarkerLeft))
    {
        LampMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.88f, 0.30f, 1.0f));
        FrontMarkerLeft->SetMaterial(0, LampMaterial);
        FrontMarkerRight->SetMaterial(0, LampMaterial);
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
        LaneOffset += FMath::Sin(GetWorld()->GetTimeSeconds() * 0.72f + WanderPhase) * 95.0f;
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
    if (!Bike || !GetWorld()) return;

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

    if (UStaticMeshComponent* Chassis = Bike->GetChassis())
    {
        Chassis->AddImpulse(Away * 260.0f + GetActorForwardVector() * 90.0f, NAME_None, true);
        Chassis->AddAngularImpulseInRadians(
            Bike->GetActorForwardVector() * (Side * 2.2f) + FVector::UpVector * (Side * 0.75f),
            NAME_None,
            true);
    }

    if (URIHealthComponent* Health = Bike->GetHealthComponent())
    {
        Health->ApplyImpact(6.0f);
    }

    Bike->TriggerComicImpact(Side, TEXT("HONK!"), 0.85f);
    RIPrototypeVisuals::PlayReaction(Bike, Side);

    if (GEngine)
    {
        const FString Message = TrafficLabel.IsEmpty()
            ? TEXT("THUNK! Civilian traffic wins this argument.")
            : FString::Printf(TEXT("THUNK! %s says: USE YOUR EYES!"), *TrafficLabel);
        GEngine->AddOnScreenDebugMessage(-1, 1.6f, FColor(255, 185, 40), Message);
    }
}
