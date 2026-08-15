#include "Items/RIRottenEggProjectile.h"

#include "Items/RIRottenEggStinkEffect.h"
#include "Vehicle/RIBikePawn.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "AI/RIAIController.h"
#include "Visual/RIPrototypeVisuals.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ARIRottenEggProjectile::ARIRottenEggProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->SetSphereRadius(18.0f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    Collision->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    Collision->SetGenerateOverlapEvents(true);
    Collision->SetNotifyRigidBodyCollision(true);

    Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
    Visual->SetupAttachment(Collision);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Visual->SetRelativeScale3D(FVector(0.28f, 0.22f, 0.34f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        Visual->SetStaticMesh(SphereMesh.Object);
    }

    Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
    Movement->UpdatedComponent = Collision;
    Movement->InitialSpeed = 2300.0f;
    Movement->MaxSpeed = 2300.0f;
    Movement->ProjectileGravityScale = 0.55f;
    Movement->bRotationFollowsVelocity = true;
    Movement->bShouldBounce = false;

    InitialLifeSpan = 4.0f;
}

void ARIRottenEggProjectile::ConfigureSource(ARIBikePawn* InSourceBike)
{
    SourceBike = InSourceBike;
}

void ARIRottenEggProjectile::BeginPlay()
{
    Super::BeginPlay();

    Collision->OnComponentBeginOverlap.AddDynamic(this, &ARIRottenEggProjectile::HandleOverlap);
    Collision->OnComponentHit.AddDynamic(this, &ARIRottenEggProjectile::HandleWorldHit);

    if (const UWorld* World = GetWorld())
    {
        SourceImmunityEndsAt = World->GetTimeSeconds() + 0.35;
    }

    Movement->Velocity = GetActorForwardVector() * Movement->InitialSpeed;

    if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
    {
        if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, this))
        {
            Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.30f, 0.34f, 0.025f, 1.0f));
            Visual->SetMaterial(0, Material);
        }
    }
}

void ARIRottenEggProjectile::HandleOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (bResolved || !HasAuthority()) return;

    ARIBikePawn* Victim = Cast<ARIBikePawn>(OtherActor);
    if (!Victim) return;

    ARIBikePawn* Source = SourceBike.Get();
    if (Victim == Source)
    {
        if (const UWorld* World = GetWorld())
        {
            if (World->GetTimeSeconds() < SourceImmunityEndsAt)
            {
                return;
            }
        }
    }

    SplatterBike(Victim);
}

void ARIRottenEggProjectile::SplatterBike(ARIBikePawn* Victim)
{
    if (!Victim || bResolved) return;
    bResolved = true;

    ARIBikePawn* Source = SourceBike.Get();
    const URIParticipantComponent* VictimParticipant = Victim->GetParticipantComponent();
    const URIParticipantComponent* SourceParticipant = Source ? Source->GetParticipantComponent() : nullptr;

    const uint32 StableHash = VictimParticipant ? GetTypeHash(VictimParticipant->GetParticipantId()) : GetTypeHash(Victim);
    const float SideSign = (StableHash & 1u) == 0u ? 1.0f : -1.0f;

    if (UStaticMeshComponent* Chassis = Victim->GetChassis())
    {
        Chassis->AddImpulse(Victim->GetActorRightVector() * (SideSign * 170.0f), NAME_None, true);
        Chassis->AddAngularImpulseInRadians(
            Victim->GetActorForwardVector() * (SideSign * 1.7f) + FVector::UpVector * (SideSign * 0.7f),
            NAME_None,
            true);
    }

    if (URIHealthComponent* Health = Victim->GetHealthComponent())
    {
        Health->ApplyImpact(1.0f);
    }

    Victim->TriggerComicImpact(SideSign, TEXT("SPLAT!"), 0.95f);
    RIPrototypeVisuals::PlayReaction(Victim, SideSign);

    if (Source)
    {
        if (ARIAIController* VictimAI = Cast<ARIAIController>(Victim->GetController()))
        {
            VictimAI->NotifyProvokedBy(Source);
        }
    }

    if (UWorld* World = GetWorld())
    {
        if (ARIRottenEggStinkEffect* Stink = World->SpawnActor<ARIRottenEggStinkEffect>(
            ARIRottenEggStinkEffect::StaticClass(),
            Victim->GetActorLocation(),
            Victim->GetActorRotation()))
        {
            Stink->SetOwner(Victim);
            Stink->AttachToActor(Victim, FAttachmentTransformRules::KeepWorldTransform);
        }
    }

    if (GEngine)
    {
        const bool bVictimHuman = VictimParticipant && VictimParticipant->IsHumanControlled();
        const bool bSourceHuman = SourceParticipant && SourceParticipant->IsHumanControlled();
        const FString VictimName = VictimParticipant ? VictimParticipant->GetParticipantId().ToString() : TEXT("RIVAL");

        if (bVictimHuman && Victim == Source)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor(130, 190, 35), TEXT("YOU EGGED YOURSELF. Magnificent."));
        }
        else if (bVictimHuman)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor(130, 190, 35), TEXT("SPLAT! You smell like a bad decision."));
        }
        else if (bSourceHuman)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                2.0f,
                FColor(130, 190, 35),
                FString::Printf(TEXT("SPLAT! %s now smells like regret."), *VictimName));
        }
    }

    Destroy();
}

void ARIRottenEggProjectile::HandleWorldHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (bResolved || !HasAuthority()) return;

    if (Cast<ARIBikePawn>(OtherActor))
    {
        return;
    }

    bResolved = true;

    ARIBikePawn* Source = SourceBike.Get();
    const URIParticipantComponent* SourceParticipant = Source ? Source->GetParticipantComponent() : nullptr;
    if (GEngine && SourceParticipant && SourceParticipant->IsHumanControlled())
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.2f, FColor(110, 150, 40), TEXT("Egg wasted. The road is unimpressed."));
    }

    Destroy();
}
