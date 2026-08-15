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
    StinkGlow->SetRelativeLocation(FVector(-35.0f, 0.0f, 115.0f));
    StinkGlow->SetAttenuationRadius(320.0f);
    StinkGlow->SetCastShadows(false);
}

void ARIPoopMessEffect::Configure(ARIBikePawn* InBike, const bool bInCowMess, const float InLifetimeSeconds)
{
    AffectedBike = InBike;
    bCowMess = bInCowMess;
    LifetimeSeconds = FMath::Max(0.5f, InLifetimeSeconds);
    SetOwner(InBike);
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
        SplatA->SetRelativeLocation(FVector(-78.0f, 0.0f, 56.0f));
        SplatA->SetRelativeScale3D(FVector(0.42f, 0.60f, 0.20f));
        SplatB->SetRelativeLocation(FVector(-26.0f, 33.0f, 108.0f));
        SplatB->SetRelativeScale3D(FVector(0.30f, 0.38f, 0.16f));
        SplatC->SetRelativeLocation(FVector(22.0f, -30.0f, 72.0f));
        SplatC->SetRelativeScale3D(FVector(0.28f, 0.42f, 0.15f));

        FumeA->SetRelativeScale3D(FVector(0.34f));
        FumeB->SetRelativeScale3D(FVector(0.28f));
        FumeC->SetRelativeScale3D(FVector(0.25f));
        FumeD->SetRelativeScale3D(FVector(0.22f));
        StinkGlow->SetIntensity(820.0f);
        StinkGlow->SetLightColor(FLinearColor(0.36f, 0.46f, 0.07f));
    }
    else
    {
        SplatA->SetRelativeLocation(FVector(-72.0f, -12.0f, 34.0f));
        SplatA->SetRelativeScale3D(FVector(0.24f, 0.34f, 0.12f));
        SplatB->SetRelativeLocation(FVector(-40.0f, 24.0f, 70.0f));
        SplatB->SetRelativeScale3D(FVector(0.18f, 0.24f, 0.10f));
        SplatC->SetRelativeLocation(FVector(6.0f, -22.0f, 48.0f));
        SplatC->SetRelativeScale3D(FVector(0.15f, 0.22f, 0.09f));

        FumeA->SetRelativeScale3D(FVector(0.24f));
        FumeB->SetRelativeScale3D(FVector(0.20f));
        FumeC->SetRelativeScale3D(FVector(0.18f));
        FumeD->SetRelativeScale3D(FVector(0.16f));
        StinkGlow->SetIntensity(460.0f);
        StinkGlow->SetLightColor(FLinearColor(0.40f, 0.50f, 0.08f));
    }

    UpdateFumes(0.0f);
}

void ARIPoopMessEffect::UpdateFumes(const float AgeSeconds)
{
    const float SizeBoost = bCowMess ? 1.25f : 1.0f;
    const float Rise = FMath::Fmod(AgeSeconds * (bCowMess ? 38.0f : 46.0f), 105.0f);

    FumeA->SetRelativeLocation(FVector(-80.0f, -36.0f + FMath::Sin(AgeSeconds * 2.2f) * 18.0f, 92.0f + Rise));
    FumeB->SetRelativeLocation(FVector(-35.0f, 42.0f + FMath::Cos(AgeSeconds * 1.8f) * 15.0f, 126.0f + FMath::Fmod(Rise + 34.0f, 105.0f)));
    FumeC->SetRelativeLocation(FVector(18.0f, -28.0f + FMath::Sin(AgeSeconds * 2.7f) * 12.0f, 82.0f + FMath::Fmod(Rise + 67.0f, 105.0f)));
    FumeD->SetRelativeLocation(FVector(-8.0f, 15.0f + FMath::Cos(AgeSeconds * 2.4f) * 20.0f, 155.0f + FMath::Fmod(Rise + 18.0f, 90.0f)));

    const float Pulse = 0.88f + FMath::Sin(AgeSeconds * 3.1f) * 0.12f;
    FumeA->SetRelativeScale3D(FumeA->GetRelativeScale3D().GetSafeNormal() * (0.30f * SizeBoost * Pulse));
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
