#include "Hazards/RIPoopMessEffect.h"

#include "Vehicle/RIBikePawn.h"
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

    SplatA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SplatA"));
    SplatA->SetupAttachment(SceneRoot);
    SplatA->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    SplatB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SplatB"));
    SplatB->SetupAttachment(SceneRoot);
    SplatB->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    SplatC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SplatC"));
    SplatC->SetupAttachment(SceneRoot);
    SplatC->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (SphereMesh.Succeeded())
    {
        SplatA->SetStaticMesh(SphereMesh.Object);
        SplatB->SetStaticMesh(SphereMesh.Object);
        SplatC->SetStaticMesh(SphereMesh.Object);
    }
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
        ExpiresAt = GetWorld()->GetTimeSeconds() + LifetimeSeconds;
    }

    ApplyPresentation();
}

void ARIPoopMessEffect::ApplyPresentation()
{
    const FLinearColor MessColor = bCowMess
        ? FLinearColor(0.24f, 0.13f, 0.035f, 1.0f)
        : FLinearColor(0.17f, 0.075f, 0.025f, 1.0f);

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
    }

    if (bCowMess)
    {
        SplatA->SetRelativeLocation(FVector(-78.0f, 0.0f, 56.0f));
        SplatA->SetRelativeScale3D(FVector(0.42f, 0.60f, 0.20f));
        SplatB->SetRelativeLocation(FVector(-26.0f, 33.0f, 108.0f));
        SplatB->SetRelativeScale3D(FVector(0.30f, 0.38f, 0.16f));
        SplatC->SetRelativeLocation(FVector(22.0f, -30.0f, 72.0f));
        SplatC->SetRelativeScale3D(FVector(0.28f, 0.42f, 0.15f));
    }
    else
    {
        SplatA->SetRelativeLocation(FVector(-72.0f, -12.0f, 34.0f));
        SplatA->SetRelativeScale3D(FVector(0.24f, 0.34f, 0.12f));
        SplatB->SetRelativeLocation(FVector(-40.0f, 24.0f, 70.0f));
        SplatB->SetRelativeScale3D(FVector(0.18f, 0.24f, 0.10f));
        SplatC->SetRelativeLocation(FVector(6.0f, -22.0f, 48.0f));
        SplatC->SetRelativeScale3D(FVector(0.15f, 0.22f, 0.09f));
    }
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

    if (GetWorld()->GetTimeSeconds() >= ExpiresAt)
    {
        Destroy();
    }
}
