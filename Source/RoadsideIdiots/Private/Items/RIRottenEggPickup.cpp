#include "Items/RIRottenEggPickup.h"

#include "Audio/RIAudioEvents.h"
#include "Vehicle/RIBikePawn.h"
#include "Core/RIParticipantComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ARIRottenEggPickup::ARIRottenEggPickup()
{
    PrimaryActorTick.bCanEverTick = false;

    Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
    SetRootComponent(Trigger);
    Trigger->SetSphereRadius(90.0f);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionObjectType(ECC_WorldDynamic);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    Trigger->SetGenerateOverlapEvents(true);

    Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
    Visual->SetupAttachment(Trigger);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Visual->SetRelativeScale3D(FVector(0.34f, 0.26f, 0.42f));
    Visual->SetRelativeRotation(FRotator(7.0f, 0.0f, -8.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        Visual->SetStaticMesh(SphereMesh.Object);
    }

    Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
    Glow->SetupAttachment(Trigger);
    Glow->SetLightColor(FLinearColor(0.42f, 0.58f, 0.05f));
    Glow->SetIntensity(900.0f);
    Glow->SetAttenuationRadius(190.0f);
    Glow->SetRelativeLocation(FVector(0.0f, 0.0f, 20.0f));
}

void ARIRottenEggPickup::BeginPlay()
{
    Super::BeginPlay();
    Trigger->OnComponentBeginOverlap.AddDynamic(this, &ARIRottenEggPickup::HandleOverlap);

    if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
    {
        if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, this))
        {
            Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.30f, 0.38f, 0.025f, 1.0f));
            Visual->SetMaterial(0, Material);
        }
    }
}

void ARIRottenEggPickup::HandleOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;

    ARIBikePawn* Bike = Cast<ARIBikePawn>(OtherActor);
    if (!Bike) return;

    const URIParticipantComponent* Participant = Bike->GetParticipantComponent();
    if (!Participant) return;

    Bike->AddRottenEgg(1);

    if (Participant->IsHumanControlled())
    {
        RIAudioEvents::Play(this, FName(TEXT("PickupEgg")), GetActorLocation(), 0.68f, 0.88f);
    }

    Destroy();
}
