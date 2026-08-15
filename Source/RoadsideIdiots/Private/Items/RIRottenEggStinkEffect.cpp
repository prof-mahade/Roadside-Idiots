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
    PuffA->SetRelativeLocation(FVector(-10.0f, -28.0f, 120.0f));
    PuffA->SetRelativeScale3D(FVector(0.19f));

    PuffB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuffB"));
    PuffB->SetupAttachment(Root);
    PuffB->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PuffB->SetRelativeLocation(FVector(-28.0f, 18.0f, 155.0f));
    PuffB->SetRelativeScale3D(FVector(0.15f));

    PuffC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuffC"));
    PuffC->SetupAttachment(Root);
    PuffC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PuffC->SetRelativeLocation(FVector(12.0f, 35.0f, 135.0f));
    PuffC->SetRelativeScale3D(FVector(0.13f));

    if (SphereMesh.Succeeded())
    {
        PuffA->SetStaticMesh(SphereMesh.Object);
        PuffB->SetStaticMesh(SphereMesh.Object);
        PuffC->SetStaticMesh(SphereMesh.Object);
    }

    Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("StinkGlow"));
    Glow->SetupAttachment(Root);
    Glow->SetLightColor(FLinearColor(0.28f, 0.65f, 0.04f));
    Glow->SetIntensity(1200.0f);
    Glow->SetAttenuationRadius(250.0f);
    Glow->SetRelativeLocation(FVector(0.0f, 0.0f, 125.0f));

    InitialLifeSpan = 4.5f;
}

void ARIRottenEggStinkEffect::BeginPlay()
{
    Super::BeginPlay();

    if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
    {
        if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, this))
        {
            Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.22f, 0.48f, 0.025f, 1.0f));
            PuffA->SetMaterial(0, Material);
            PuffB->SetMaterial(0, Material);
            PuffC->SetMaterial(0, Material);
        }
    }
}
