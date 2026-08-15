#include "Items/RIRottenEggStinkEffect.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ARIRottenEggStinkEffect::ARIRottenEggStinkEffect()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

    PuffA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuffA"));
    PuffA->SetupAttachment(Root);
    PuffA->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PuffA->SetRelativeLocation(FVector(-18.0f, -28.0f, 122.0f));
    PuffA->SetRelativeScale3D(FVector(0.075f, 0.075f, 0.20f));

    PuffB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuffB"));
    PuffB->SetupAttachment(Root);
    PuffB->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PuffB->SetRelativeLocation(FVector(-30.0f, 22.0f, 158.0f));
    PuffB->SetRelativeScale3D(FVector(0.065f, 0.065f, 0.17f));

    PuffC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuffC"));
    PuffC->SetupAttachment(Root);
    PuffC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PuffC->SetRelativeLocation(FVector(18.0f, 34.0f, 142.0f));
    PuffC->SetRelativeScale3D(FVector(0.055f, 0.055f, 0.15f));

    PuffD = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuffD"));
    PuffD->SetupAttachment(Root);
    PuffD->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PuffD->SetVisibility(false, true);

    PuffE = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuffE"));
    PuffE->SetupAttachment(Root);
    PuffE->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PuffE->SetVisibility(false, true);

    Splatter = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Splatter"));
    Splatter->SetupAttachment(Root);
    Splatter->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Splatter->SetRelativeLocation(FVector(-16.0f, 0.0f, 108.0f));
    Splatter->SetRelativeRotation(FRotator(0.0f, 0.0f, 18.0f));
    Splatter->SetRelativeScale3D(FVector(0.14f, 0.24f, 0.045f));

    if (SphereMesh.Succeeded())
    {
        PuffA->SetStaticMesh(SphereMesh.Object);
        PuffB->SetStaticMesh(SphereMesh.Object);
        PuffC->SetStaticMesh(SphereMesh.Object);
        PuffD->SetStaticMesh(SphereMesh.Object);
        PuffE->SetStaticMesh(SphereMesh.Object);
        Splatter->SetStaticMesh(SphereMesh.Object);
    }

    Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("StinkGlow"));
    Glow->SetupAttachment(Root);
    Glow->SetLightColor(FLinearColor(0.30f, 0.72f, 0.025f));
    Glow->SetIntensity(260.0f);
    Glow->SetAttenuationRadius(190.0f);
    Glow->SetRelativeLocation(FVector(0.0f, 0.0f, 135.0f));
    Glow->SetCastShadows(false);

    InitialLifeSpan = 6.0f;
}

void ARIRottenEggStinkEffect::BeginPlay()
{
    Super::BeginPlay();

    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!BaseMaterial) return;

    if (UMaterialInstanceDynamic* StinkMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this))
    {
        StinkMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.20f, 0.43f, 0.018f, 1.0f));
        PuffA->SetMaterial(0, StinkMaterial);
        PuffB->SetMaterial(0, StinkMaterial);
        PuffC->SetMaterial(0, StinkMaterial);
    }

    if (UMaterialInstanceDynamic* SplatterMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this))
    {
        SplatterMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.68f, 0.58f, 0.02f, 1.0f));
        Splatter->SetMaterial(0, SplatterMaterial);
    }
}
