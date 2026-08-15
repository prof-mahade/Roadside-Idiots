#include "Items/RIBananaPickup.h"

#include "Vehicle/RIBikePawn.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/Engine.h"
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
    Visual->SetRelativeScale3D(FVector(0.42f, 0.22f, 0.18f));
    Visual->SetRelativeRotation(FRotator(0.0f, 0.0f, 22.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        Visual->SetStaticMesh(SphereMesh.Object);
    }

    Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
    Glow->SetupAttachment(PickupTrigger);
    Glow->SetLightColor(FLinearColor(1.0f, 0.72f, 0.04f));
    Glow->SetIntensity(1800.0f);
    Glow->SetAttenuationRadius(260.0f);
    Glow->SetRelativeLocation(FVector(0.0f, 0.0f, 30.0f));
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
        }
    }
}

void ARIBananaPickup::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    Visual->AddLocalRotation(FRotator(0.0f, 100.0f * DeltaSeconds, 0.0f));
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

    const float Before = Bike->GetHealthComponent()->GetCurrentHealth();
    const float After = Bike->GetHealthComponent()->Heal(HealAmount);
    Bike->AddBananaPeel(1);

    if (GEngine && Participant->IsHumanControlled())
    {
        const float Recovered = FMath::Max(0.0f, After - Before);
        GEngine->AddOnScreenDebugMessage(
            -1,
            2.0f,
            FColor::Yellow,
            FString::Printf(TEXT("NOM! Banana +%.0f Condition | Peel ready [F]"), Recovered));
    }

    Destroy();
}
