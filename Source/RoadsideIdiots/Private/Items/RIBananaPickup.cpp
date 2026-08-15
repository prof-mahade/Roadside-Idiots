#include "Items/RIBananaPickup.h"

#include "Audio/RIAudioEvents.h"
#include "Vehicle/RIBikePawn.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ARIBananaPickup::ARIBananaPickup()
{
    PrimaryActorTick.bCanEverTick = true;

    PickupTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("PickupTrigger"));
    SetRootComponent(PickupTrigger);
    PickupTrigger->SetSphereRadius(90.0f);
    PickupTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupTrigger->SetCollisionObjectType(ECC_WorldDynamic);
    PickupTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupTrigger->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    PickupTrigger->SetGenerateOverlapEvents(true);

    Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
    Visual->SetupAttachment(PickupTrigger);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Visual->SetGenerateOverlapEvents(false);
    Visual->SetRelativeScale3D(FVector(0.38f, 0.18f, 0.15f));
    Visual->SetRelativeLocation(FVector(-13.0f, 0.0f, 0.0f));
    Visual->SetRelativeRotation(FRotator(0.0f, 0.0f, 28.0f));

    VisualTip = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualTip"));
    VisualTip->SetupAttachment(PickupTrigger);
    VisualTip->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualTip->SetGenerateOverlapEvents(false);
    VisualTip->SetRelativeScale3D(FVector(0.31f, 0.16f, 0.14f));
    VisualTip->SetRelativeLocation(FVector(20.0f, 0.0f, 9.0f));
    VisualTip->SetRelativeRotation(FRotator(0.0f, 0.0f, -28.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        Visual->SetStaticMesh(SphereMesh.Object);
        VisualTip->SetStaticMesh(SphereMesh.Object);
    }

    Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
    Glow->SetupAttachment(PickupTrigger);
    Glow->SetLightColor(FLinearColor(1.0f, 0.72f, 0.04f));
    Glow->SetIntensity(1450.0f);
    Glow->SetAttenuationRadius(240.0f);
    Glow->SetRelativeLocation(FVector(0.0f, 0.0f, 28.0f));
}

void ARIBananaPickup::BeginPlay()
{
    Super::BeginPlay();
    PickupTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARIBananaPickup::HandlePickupOverlap);

    if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
    {
        if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, this))
        {
            Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.68f, 0.02f, 1.0f));
            Visual->SetMaterial(0, Material);
            VisualTip->SetMaterial(0, Material);
        }
    }
}

void ARIBananaPickup::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    AddActorLocalRotation(FRotator(0.0f, 100.0f * DeltaSeconds, 0.0f));
}

void ARIBananaPickup::HandlePickupOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (bConsumed || !HasAuthority()) return;

    ARIBikePawn* Bike = Cast<ARIBikePawn>(OtherActor);
    if (!Bike) return;

    const URIParticipantComponent* Participant = Bike->GetParticipantComponent();
    if (!Participant) return;

    bConsumed = true;

    Bike->GetHealthComponent()->Heal(HealAmount);
    Bike->AddBananaPeel(1);

    if (Participant->IsHumanControlled())
    {
        RIAudioEvents::Play(this, FName(TEXT("PickupBanana")), GetActorLocation(), 0.72f, 1.08f);
    }

    Destroy();
}
