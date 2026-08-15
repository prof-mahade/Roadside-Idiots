#include "Items/RIBananaPeelHazard.h"

#include "Vehicle/RIBikePawn.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "AI/RIAIController.h"
#include "Visual/RIPrototypeVisuals.h"
#include "Audio/RIAudioEvents.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ARIBananaPeelHazard::ARIBananaPeelHazard()
{
    PrimaryActorTick.bCanEverTick = false;

    PhysicsBody = CreateDefaultSubobject<USphereComponent>(TEXT("PhysicsBody"));
    SetRootComponent(PhysicsBody);
    PhysicsBody->SetSphereRadius(12.0f);
    PhysicsBody->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PhysicsBody->SetCollisionObjectType(ECC_WorldDynamic);
    PhysicsBody->SetCollisionResponseToAllChannels(ECR_Ignore);
    PhysicsBody->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    PhysicsBody->SetGenerateOverlapEvents(false);
    PhysicsBody->SetSimulatePhysics(true);
    PhysicsBody->SetEnableGravity(true);
    PhysicsBody->SetLinearDamping(0.85f);
    PhysicsBody->SetAngularDamping(1.25f);
    PhysicsBody->BodyInstance.bUseCCD = true;

    Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
    Trigger->SetupAttachment(PhysicsBody);
    Trigger->SetSphereRadius(78.0f);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionObjectType(ECC_WorldDynamic);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    Trigger->SetGenerateOverlapEvents(true);

    Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
    Visual->SetupAttachment(PhysicsBody);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Visual->SetGenerateOverlapEvents(false);
    Visual->SetRelativeScale3D(FVector(0.58f, 0.34f, 0.07f));
    Visual->SetRelativeLocation(FVector(0.0f, 0.0f, -8.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        Visual->SetStaticMesh(SphereMesh.Object);
    }

    Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
    Glow->SetupAttachment(PhysicsBody);
    Glow->SetLightColor(FLinearColor(1.0f, 0.62f, 0.02f));
    Glow->SetIntensity(700.0f);
    Glow->SetAttenuationRadius(155.0f);
    Glow->SetRelativeLocation(FVector(0.0f, 0.0f, 18.0f));

    InitialLifeSpan = 25.0f;
}

void ARIBananaPeelHazard::BeginPlay()
{
    Super::BeginPlay();

    PhysicsBody->SetMassOverrideInKg(NAME_None, 0.20f, true);
    Trigger->OnComponentBeginOverlap.AddDynamic(this, &ARIBananaPeelHazard::HandleHazardOverlap);

    if (const UWorld* World = GetWorld())
    {
        SourceImmunityEndsAt = World->GetTimeSeconds() + SourceImmunitySeconds;
    }

    if (ARIBikePawn* Dropper = SourceBike.Get())
    {
        if (UStaticMeshComponent* DropperChassis = Dropper->GetChassis())
        {
            const FVector InheritedVelocity = DropperChassis->GetPhysicsLinearVelocity() * 0.30f;
            PhysicsBody->SetPhysicsLinearVelocity(InheritedVelocity);
        }
        PhysicsBody->AddAngularImpulseInRadians(FVector(0.8f, 1.1f, 1.6f), NAME_None, true);
    }

    if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
    {
        if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, this))
        {
            Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.60f, 0.01f, 1.0f));
            Visual->SetMaterial(0, Material);
        }
    }
}

void ARIBananaPeelHazard::ConfigureSource(ARIBikePawn* InSourceBike)
{
    SourceBike = InSourceBike;
}

void ARIBananaPeelHazard::HandleHazardOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (bTriggered || !HasAuthority()) return;

    ARIBikePawn* OtherBike = Cast<ARIBikePawn>(OtherActor);
    if (!OtherBike) return;

    ARIBikePawn* ValidSource = SourceBike.Get();
    if (OtherBike == ValidSource)
    {
        if (const UWorld* World = GetWorld())
        {
            if (World->GetTimeSeconds() < SourceImmunityEndsAt)
            {
                return;
            }
        }
    }

    bTriggered = true;

    const URIParticipantComponent* OtherParticipant = OtherBike->GetParticipantComponent();
    const uint32 StableHash = OtherParticipant ? GetTypeHash(OtherParticipant->GetParticipantId()) : GetTypeHash(OtherBike);
    const float SideSign = (StableHash & 1u) == 0u ? 1.0f : -1.0f;

    if (UStaticMeshComponent* Chassis = OtherBike->GetChassis())
    {
        const FVector SideImpulse = OtherBike->GetActorRightVector() * (SideSign * 520.0f);
        Chassis->AddImpulse(SideImpulse + FVector::UpVector * 85.0f, NAME_None, true);

        const FVector AngularImpulse =
            OtherBike->GetActorForwardVector() * (SideSign * 6.5f) +
            FVector::UpVector * (SideSign * 2.2f);
        Chassis->AddAngularImpulseInRadians(AngularImpulse, NAME_None, true);
    }

    OtherBike->GetHealthComponent()->ApplyImpact(2.0f);
    OtherBike->TriggerComicImpact(SideSign, ValidSource == OtherBike ? TEXT("OWN GOAL!" ) : TEXT("SLIP!"), 0.95f);
    RIPrototypeVisuals::PlayReaction(OtherBike, SideSign);
    RIAudioEvents::Play(this, TEXT("PeelSlip"), OtherBike->GetActorLocation(), 1.0f, FMath::FRandRange(0.94f, 1.08f));

    if (ValidSource)
    {
        if (ARIAIController* OtherController = Cast<ARIAIController>(OtherBike->GetController()))
        {
            OtherController->NotifyProvokedBy(ValidSource);
        }
    }

    Destroy();
}
