#include "Items/RIRottenEggProjectile.h"

#include "Items/RIRottenEggStinkEffect.h"
#include "Vehicle/RIBikePawn.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "AI/RIAIController.h"
#include "Visual/RIPrototypeVisuals.h"
#include "Audio/RIAudioEvents.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
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
    if (SphereMesh.Succeeded()) Visual->SetStaticMesh(SphereMesh.Object);

    Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
    Movement->UpdatedComponent = Collision;
    Movement->InitialSpeed = 2300.0f;
    Movement->MaxSpeed = 2600.0f;
    Movement->ProjectileGravityScale = 0.55f;
    Movement->bRotationFollowsVelocity = true;
    Movement->bShouldBounce = false;
    Movement->HomingAccelerationMagnitude = 7200.0f;

    InitialLifeSpan = 4.0f;
}

void ARIRottenEggProjectile::ConfigureSource(ARIBikePawn* InSourceBike)
{
    SourceBike = InSourceBike;
}

void ARIRottenEggProjectile::ConfigureTarget(ARIBikePawn* InTargetBike)
{
    AssistedTargetBike = InTargetBike;
}

void ARIRottenEggProjectile::BeginPlay()
{
    Super::BeginPlay();

    Collision->OnComponentBeginOverlap.AddDynamic(this, &ARIRottenEggProjectile::HandleOverlap);
    Collision->OnComponentHit.AddDynamic(this, &ARIRottenEggProjectile::HandleWorldHit);

    if (const UWorld* World = GetWorld()) SourceImmunityEndsAt = World->GetTimeSeconds() + 0.35;

    if (ARIBikePawn* Target = AssistedTargetBike.Get())
    {
        if (UStaticMeshComponent* TargetChassis = Target->GetChassis())
        {
            Movement->HomingTargetComponent = TargetChassis;
            Movement->bIsHomingProjectile = true;
            Movement->ProjectileGravityScale = 0.12f;
        }
    }

    Movement->Velocity = GetActorForwardVector() * Movement->InitialSpeed;
    RIAudioEvents::Play(this, TEXT("EggThrow"), GetActorLocation(), 0.80f, FMath::FRandRange(0.96f, 1.04f));

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
            if (World->GetTimeSeconds() < SourceImmunityEndsAt) return;
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

    if (URIHealthComponent* Health = Victim->GetHealthComponent()) Health->ApplyImpact(1.0f);

    RIAudioEvents::Play(this, TEXT("EggSplat"), Victim->GetActorLocation(), 1.0f, FMath::FRandRange(0.94f, 1.06f));
    Victim->TriggerComicImpact(SideSign, TEXT("SPLAT!"), 0.95f);
    RIPrototypeVisuals::PlayReaction(Victim, SideSign);

    if (Source)
    {
        if (ARIAIController* VictimAI = Cast<ARIAIController>(Victim->GetController())) VictimAI->NotifyProvokedBy(Source);
    }

    if (UWorld* World = GetWorld())
    {
        bool bRefreshedExistingStink = false;
        for (TActorIterator<ARIRottenEggStinkEffect> It(World); It; ++It)
        {
            ARIRottenEggStinkEffect* Existing = *It;
            if (Existing && Cast<ARIBikePawn>(Existing->GetOwner()) == Victim)
            {
                Existing->SetLifeSpan(6.0f);
                bRefreshedExistingStink = true;
                break;
            }
        }

        if (!bRefreshedExistingStink)
        {
            if (ARIRottenEggStinkEffect* Stink = World->SpawnActor<ARIRottenEggStinkEffect>(
                ARIRottenEggStinkEffect::StaticClass(), Victim->GetActorLocation(), Victim->GetActorRotation()))
            {
                Stink->SetOwner(Victim);
                Stink->AttachToActor(Victim, FAttachmentTransformRules::KeepWorldTransform);
            }
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
    if (Cast<ARIBikePawn>(OtherActor)) return;

    bResolved = true;
    RIAudioEvents::Play(this, TEXT("EggMiss"), GetActorLocation(), 0.55f, FMath::FRandRange(0.95f, 1.05f));
    Destroy();
}
