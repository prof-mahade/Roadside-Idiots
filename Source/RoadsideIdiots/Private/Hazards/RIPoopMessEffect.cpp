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

    StinkGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("StinkGlow"));
    StinkGlow->SetupAttachment(SceneRoot);
    StinkGlow->SetRelativeLocation(FVector(-35.0f, 0.0f, 105.0f));
    StinkGlow->SetAttenuationRadius(235.0f);
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
    // A rider can cross another pile before the previous mess expires. Refresh
    // the existing effect instead of stacking multiple splat/fume actors on the
    // same motorcycle, which quickly obscures the rider and camera view.
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
        ? FLinearColor(0.30f, 0.40f, 0.045f, 1.0f)
        : FLinearColor(0.36f, 0.46f, 0.07f, 1.0f);

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

        for (UStaticMeshComponent* Mesh : {FumeA.Get(), FumeB.Get(), FumeC.Get(), FumeD.Get()})
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
        SplatA->SetRelativeLocation(FVector(-68.0f, 0.0f, 50.0f));
        SplatA->SetRelativeScale3D(FVector(0.25f, 0.36f, 0.10f));
        SplatB->SetRelativeLocation(FVector(-25.0f, 28.0f, 92.0f));
        SplatB->SetRelativeScale3D(FVector(0.18f, 0.23f, 0.09f));
        SplatC->SetRelativeLocation(FVector(18.0f, -25.0f, 66.0f));
        SplatC->SetRelativeScale3D(FVector(0.16f, 0.23f, 0.08f));
        StinkGlow->SetIntensity(430.0f);
        StinkGlow->SetLightColor(FLinearColor(0.36f, 0.46f, 0.07f));
    }
    else
    {
        SplatA->SetRelativeLocation(FVector(-62.0f, -10.0f, 32.0f));
        SplatA->SetRelativeScale3D(FVector(0.16f, 0.22f, 0.08f));
        SplatB->SetRelativeLocation(FVector(-35.0f, 20.0f, 64.0f));
        SplatB->SetRelativeScale3D(FVector(0.12f, 0.16f, 0.07f));
        SplatC->SetRelativeLocation(FVector(4.0f, -18.0f, 45.0f));
        SplatC->SetRelativeScale3D(FVector(0.10f, 0.15f, 0.06f));
        StinkGlow->SetIntensity(260.0f);
        StinkGlow->SetLightColor(FLinearColor(0.40f, 0.50f, 0.08f));
    }

    UpdateFumes(0.0f);
}

void ARIPoopMessEffect::UpdateFumes(const float AgeSeconds)
{
    const float Rise = FMath::Fmod(AgeSeconds * (bCowMess ? 31.0f : 38.0f), 88.0f);

    FumeA->SetRelativeLocation(FVector(-70.0f, -30.0f + FMath::Sin(AgeSeconds * 2.2f) * 13.0f, 88.0f + Rise));
    FumeB->SetRelativeLocation(FVector(-32.0f, 34.0f + FMath::Cos(AgeSeconds * 1.8f) * 11.0f, 116.0f + FMath::Fmod(Rise + 30.0f, 88.0f)));
    FumeC->SetRelativeLocation(FVector(12.0f, -24.0f + FMath::Sin(AgeSeconds * 2.7f) * 9.0f, 80.0f + FMath::Fmod(Rise + 58.0f, 88.0f)));
    FumeD->SetRelativeLocation(FVector(-6.0f, 12.0f + FMath::Cos(AgeSeconds * 2.4f) * 14.0f, 138.0f + FMath::Fmod(Rise + 17.0f, 78.0f)));

    const float PulseA = 0.90f + FMath::Sin(AgeSeconds * 3.1f) * 0.10f;
    const float PulseB = 0.90f + FMath::Sin(AgeSeconds * 2.6f + 1.1f) * 0.10f;
    const float PulseC = 0.90f + FMath::Sin(AgeSeconds * 3.5f + 2.0f) * 0.10f;
    const float PulseD = 0.90f + FMath::Sin(AgeSeconds * 2.9f + 2.8f) * 0.10f;

    const float ScaleA = bCowMess ? 0.19f : 0.14f;
    const float ScaleB = bCowMess ? 0.16f : 0.12f;
    const float ScaleC = bCowMess ? 0.14f : 0.105f;
    const float ScaleD = bCowMess ? 0.12f : 0.09f;

    FumeA->SetRelativeScale3D(FVector(ScaleA * PulseA));
    FumeB->SetRelativeScale3D(FVector(ScaleB * PulseB));
    FumeC->SetRelativeScale3D(FVector(ScaleC * PulseC));
    FumeD->SetRelativeScale3D(FVector(ScaleD * PulseD));
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
