#include "Items/RIBananaPeelHazard.h"

#include "Vehicle/RIBikePawn.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "AI/RIAIController.h"
#include "Visual/RIPrototypeVisuals.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ARIBananaPeelHazard::ARIBananaPeelHazard()
{
    PrimaryActorTick.bCanEverTick = false;

    Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
    SetRootComponent(Trigger);
    Trigger->SetSphereRadius(78.0f);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionObjectType(ECC_WorldDynamic);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    Trigger->SetGenerateOverlapEvents(true);

    Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
    Visual->SetupAttachment(Trigger);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Visual->SetGenerateOverlapEvents(false);
    Visual->SetRelativeScale3D(FVector(0.58f, 0.34f, 0.07f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        Visual->SetStaticMesh(SphereMesh.Object);
    }

    Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
    Glow->SetupAttachment(Trigger);
    Glow->SetLightColor(FLinearColor(1.0f, 0.62f, 0.02f));
    Glow->SetIntensity(700.0f);
    Glow->SetAttenuationRadius(155.0f);
    Glow->SetRelativeLocation(FVector(0.0f, 0.0f, 22.0f));

    InitialLifeSpan = 25.0f;
}

void ARIBananaPeelHazard::BeginPlay()
{
    Super::BeginPlay();
    Trigger->OnComponentBeginOverlap.AddDynamic(this, &ARIBananaPeelHazard::HandleHazardOverlap);

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
    if (!OtherBike || OtherBike == SourceBike.Get()) return;

    bTriggered = true;

    const URIParticipantComponent* OtherParticipant = OtherBike->GetParticipantComponent();
    const uint32 StableHash = OtherParticipant ? GetTypeHash(OtherParticipant->GetParticipantId()) : GetTypeHash(OtherBike);
    const float SideSign = (StableHash & 1u) == 0u ? 1.0f : -1.0f;

    if (UStaticMeshComponent* Chassis = OtherBike->GetChassis())
    {
        const FVector SideImpulse = OtherBike->GetActorRightVector() * (SideSign * 360.0f);
        Chassis->AddImpulse(SideImpulse + FVector::UpVector * 55.0f, NAME_None, true);

        const FVector AngularImpulse =
            OtherBike->GetActorForwardVector() * (SideSign * 4.2f) +
            FVector::UpVector * (SideSign * 1.4f);
        Chassis->AddAngularImpulseInRadians(AngularImpulse, NAME_None, true);
    }

    OtherBike->GetHealthComponent()->ApplyImpact(1.0f);
    RIPrototypeVisuals::PlayReaction(OtherBike, SideSign);

    ARIBikePawn* ValidSource = SourceBike.Get();
    if (ValidSource)
    {
        if (ARIAIController* OtherController = Cast<ARIAIController>(OtherBike->GetController()))
        {
            OtherController->NotifyProvokedBy(ValidSource);
        }
    }

    if (GEngine)
    {
        const URIParticipantComponent* SourceParticipant = ValidSource ? ValidSource->GetParticipantComponent() : nullptr;
        const bool bOtherHuman = OtherParticipant && OtherParticipant->IsHumanControlled();
        const bool bSourceHuman = SourceParticipant && SourceParticipant->IsHumanControlled();
        const FString OtherName = OtherParticipant ? OtherParticipant->GetParticipantId().ToString() : TEXT("RIVAL");

        if (bOtherHuman)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.8f, FColor::Yellow, TEXT("BANANA BETRAYAL! You hit a peel!"));
        }
        else if (bSourceHuman)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                1.8f,
                FColor::Yellow,
                FString::Printf(TEXT("%s slipped on YOUR banana peel!"), *OtherName));
        }
    }

    Destroy();
}
