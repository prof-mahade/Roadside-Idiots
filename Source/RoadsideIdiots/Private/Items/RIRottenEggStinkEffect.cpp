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
    PuffA->SetRelativeLocation(FVector(-22.0f, -44.0f, 118.0f));
    PuffA->SetRelativeScale3D(FVector(0.42f));

    PuffB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuffB"));
    PuffB->SetupAttachment(Root);
    PuffB->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PuffB->SetRelativeLocation(FVector(-42.0f, 28.0f, 164.0f));
    PuffB->SetRelativeScale3D(FVector(0.34f));

    PuffC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuffC"));
    PuffC->SetupAttachment(Root);
    PuffC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PuffC->SetRelativeLocation(FVector(24.0f, 52.0f, 138.0f));
    PuffC->SetRelativeScale3D(FVector(0.30f));

    PuffD = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuffD"));
    PuffD->SetupAttachment(Root);
    PuffD->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PuffD->SetRelativeLocation(FVector(34.0f, -30.0f, 182.0f));
    PuffD->SetRelativeScale3D(FVector(0.26f));

    PuffE = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuffE"));
    PuffE->SetupAttachment(Root);
    PuffE->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PuffE->SetRelativeLocation(FVector(-10.0f, 8.0f, 215.0f));
    PuffE->SetRelativeScale3D(FVector(0.22f));

    Splatter = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Splatter"));
    Splatter->SetupAttachment(Root);
    Splatter->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Splatter->SetRelativeLocation(FVector(-18.0f, 0.0f, 112.0f));
    Splatter->SetRelativeRotation(FRotator(0.0f, 0.0f, 18.0f));
    Splatter->SetRelativeScale3D(FVector(0.20f, 0.34f, 0.055f));

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
    Glow->SetLightColor(FLinearColor(0.30f, 0.78f, 0.025f));
    Glow->SetIntensity(2300.0f);
    Glow->SetAttenuationRadius(360.0f);
    Glow->SetRelativeLocation(FVector(0.0f, 0.0f, 145.0f));

    InitialLifeSpan = 6.0f;
}

void ARIRottenEggStinkEffect::BeginPlay()
{
    Super::BeginPlay();

    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!BaseMaterial) return;

    if (UMaterialInstanceDynamic* StinkMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this))
    {
        StinkMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.20f, 0.47f, 0.018f, 1.0f));
        PuffA->SetMaterial(0, StinkMaterial);
        PuffB->SetMaterial(0, StinkMaterial);
        PuffC->SetMaterial(0, StinkMaterial);
        PuffD->SetMaterial(0, StinkMaterial);
        PuffE->SetMaterial(0, StinkMaterial);
    }

    if (UMaterialInstanceDynamic* SplatterMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this))
    {
        SplatterMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.72f, 0.62f, 0.02f, 1.0f));
        Splatter->SetMaterial(0, SplatterMaterial);
    }
}
