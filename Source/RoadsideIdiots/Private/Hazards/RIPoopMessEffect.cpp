#include "Hazards/RIPoopMessEffect.h"

#include "Vehicle/RIBikePawn.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ARIPoopMessEffect::ARIPoopMessEffect()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

    auto SetupSphere = [&](UStaticMeshComponent* Mesh)
    {
        Mesh->SetupAttachment(SceneRoot);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        if (SphereMesh.Succeeded())
        {
            Mesh->SetStaticMesh(SphereMesh.Object);
        }
    };

    SplatA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SplatA"));
    SplatB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SplatB"));
    SplatC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SplatC"));
    FumeA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FumeA"));
    FumeB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FumeB"));
    FumeC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FumeC"));
    FumeD = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FumeD"));

    SetupSphere(SplatA);
    SetupSphere(SplatB);
    SetupSphere(SplatC);
    SetupSphere(FumeA);
    SetupSphere(FumeB);
    SetupSphere(FumeC);
    SetupSphere(FumeD);
    FumeD->SetVisibility(false, true);

    StinkGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("StinkGlow"));
    StinkGlow->SetupAttachment(SceneRoot);
    StinkGlow->SetRelativeLocation(FVector(-35.0f, 0.0f, 105.0f));
    StinkGlow->SetAttenuationRadius(190.0f);
    StinkGlow->SetCastShadows(false);
}

void ARIPoopMessEffect::Configure(ARIBikePawn* InBike, const bool bInCowMess, const float InLifetimeSeconds)
{
    AffectedBike = InBike;
    bCowMess = bInCowMess;
    LifetimeSeconds = FMath::Max(0.5f, InLifetimeSeconds);
    SetOwner(InBike);
}

void ARIPoopMessEffect::Refresh(const bool bInCowMess, const float InLifetimeSeconds)
{
    bCowMess = bCowMess || bInCowMess;
    LifetimeSeconds = FMath::Max(LifetimeSeconds, FMath::Max(0.5f, InLifetimeSeconds));

    if (GetWorld())
    {
        const double Now = GetWorld()->GetTimeSeconds();
        ExpiresAt = FMath::Max(ExpiresAt, Now + InLifetimeSeconds);
    }

    ApplyPresentation();
}

void ARIPoopMessEffect::BeginPlay()
{
    Super::BeginPlay();

    if (GetWorld())
    {
        SpawnedAt = GetWorld()->GetTimeSeconds();
        ExpiresAt = SpawnedAt + LifetimeSeconds;
    }

    ApplyPresentation();
}

void ARIPoopMessEffect::ApplyPresentation()
{
    const FLinearColor MessColor = bCowMess
        ? FLinearColor(0.24f, 0.13f, 0.035f, 1.0f)
        : FLinearColor(0.17f, 0.075f, 0.025f, 1.0f);

    const FLinearColor FumeColor = bCowMess
        ? FLinearColor(0.27f, 0.36f, 0.04f, 1.0f)
        : FLinearColor(0.32f, 0.41f, 0.06f, 1.0f);

    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (BaseMaterial)
    {
        for (UStaticMeshComponent* Mesh : {SplatA.Get(), SplatB.Get(), SplatC.Get()})
        {
            if (!Mesh) continue;
            if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Mesh))
            {
                Material->SetVectorParameterValue(TEXT("Color"), MessColor);
                Mesh->SetMaterial(0, Material);
            }
        }

        for (UStaticMeshComponent* Mesh : {FumeA.Get(), FumeB.Get(), FumeC.Get()})
        {
            if (!Mesh) continue;
            if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Mesh))
            {
                Material->SetVectorParameterValue(TEXT("Color"), FumeColor);
                Mesh->SetMaterial(0, Material);
            }
        }
    }

    if (bCowMess)
    {
        SplatA->SetRelativeLocation(FVector(-66.0f, 0.0f, 48.0f));
        SplatA->SetRelativeScale3D(FVector(0.22f, 0.31f, 0.08f));
        SplatB->SetRelativeLocation(FVector(-24.0f, 25.0f, 87.0f));
        SplatB->SetRelativeScale3D(FVector(0.15f, 0.20f, 0.07f));
        SplatC->SetRelativeLocation(FVector(15.0f, -22.0f, 62.0f));
        SplatC->SetRelativeScale3D(FVector(0.13f, 0.20f, 0.065f));
        StinkGlow->SetIntensity(250.0f);
        StinkGlow->SetLightColor(FLinearColor(0.34f, 0.43f, 0.06f));
    }
    else
    {
        SplatA->SetRelativeLocation(FVector(-60.0f, -9.0f, 31.0f));
        SplatA->SetRelativeScale3D(FVector(0.14f, 0.19f, 0.065f));
        SplatB->SetRelativeLocation(FVector(-34.0f, 18.0f, 61.0f));
        SplatB->SetRelativeScale3D(FVector(0.10f, 0.14f, 0.055f));
        SplatC->SetRelativeLocation(FVector(4.0f, -16.0f, 43.0f));
        SplatC->SetRelativeScale3D(FVector(0.085f, 0.13f, 0.05f));
        StinkGlow->SetIntensity(150.0f);
        StinkGlow->SetLightColor(FLinearColor(0.38f, 0.47f, 0.07f));
    }

    UpdateFumes(0.0f);
}

void ARIPoopMessEffect::UpdateFumes(const float AgeSeconds)
{
    const float Rise = FMath::Fmod(AgeSeconds * (bCowMess ? 34.0f : 40.0f), 92.0f);

    FumeA->SetRelativeLocation(FVector(-63.0f, -24.0f + FMath::Sin(AgeSeconds * 2.1f) * 9.0f, 86.0f + Rise));
    FumeB->SetRelativeLocation(FVector(-28.0f, 27.0f + FMath::Cos(AgeSeconds * 1.7f) * 8.0f, 112.0f + FMath::Fmod(Rise + 32.0f, 92.0f)));
    FumeC->SetRelativeLocation(FVector(8.0f, -18.0f + FMath::Sin(AgeSeconds * 2.5f) * 7.0f, 78.0f + FMath::Fmod(Rise + 61.0f, 92.0f)));

    const float PulseA = 0.92f + FMath::Sin(AgeSeconds * 3.0f) * 0.08f;
    const float PulseB = 0.92f + FMath::Sin(AgeSeconds * 2.5f + 1.1f) * 0.08f;
    const float PulseC = 0.92f + FMath::Sin(AgeSeconds * 3.3f + 2.0f) * 0.08f;

    const float WidthA = bCowMess ? 0.082f : 0.060f;
    const float WidthB = bCowMess ? 0.070f : 0.052f;
    const float WidthC = bCowMess ? 0.060f : 0.045f;

    // Tall, narrow ellipsoids read more like drifting stink wisps than solid
    // green balls, while still using engine primitives and no Niagara assets.
    FumeA->SetRelativeScale3D(FVector(WidthA * PulseA, WidthA * PulseA, WidthA * 2.25f * PulseA));
    FumeB->SetRelativeScale3D(FVector(WidthB * PulseB, WidthB * PulseB, WidthB * 2.55f * PulseB));
    FumeC->SetRelativeScale3D(FVector(WidthC * PulseC, WidthC * PulseC, WidthC * 2.05f * PulseC));
}

void ARIPoopMessEffect::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    ARIBikePawn* Bike = AffectedBike.Get();
    if (!Bike || !GetWorld())
    {
        Destroy();
        return;
    }

    SetActorLocationAndRotation(Bike->GetActorLocation(), Bike->GetActorRotation(), false, nullptr, ETeleportType::TeleportPhysics);

    const double Now = GetWorld()->GetTimeSeconds();
    UpdateFumes(static_cast<float>(Now - SpawnedAt));

    if (Now >= ExpiresAt)
    {
        Destroy();
    }
}
