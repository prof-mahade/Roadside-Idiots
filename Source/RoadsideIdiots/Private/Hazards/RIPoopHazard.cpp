#include "Hazards/RIPoopHazard.h"

#include "Hazards/RIPoopMessEffect.h"
#include "Vehicle/RIBikePawn.h"
#include "Visual/RIPrototypeVisuals.h"
#include "Audio/RIAudioEvents.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ARIPoopHazard::ARIPoopHazard()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    SetRootComponent(TriggerVolume);
    TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
    TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerVolume->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    TriggerVolume->SetGenerateOverlapEvents(true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

    BlobA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlobA"));
    BlobA->SetupAttachment(TriggerVolume);
    BlobA->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    BlobB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlobB"));
    BlobB->SetupAttachment(TriggerVolume);
    BlobB->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    BlobC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlobC"));
    BlobC->SetupAttachment(TriggerVolume);
    BlobC->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (SphereMesh.Succeeded())
    {
        BlobA->SetStaticMesh(SphereMesh.Object);
        BlobB->SetStaticMesh(SphereMesh.Object);
        BlobC->SetStaticMesh(SphereMesh.Object);
    }

    StinkGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("StinkGlow"));
    StinkGlow->SetupAttachment(TriggerVolume);
    StinkGlow->SetLightColor(FLinearColor(0.54f, 0.72f, 0.08f));
    StinkGlow->SetIntensity(210.0f);
    StinkGlow->SetAttenuationRadius(145.0f);
    StinkGlow->SetRelativeLocation(FVector(0.0f, 0.0f, 24.0f));
    StinkGlow->SetCastShadows(false);
}

void ARIPoopHazard::Configure(const ERIPoopHazardType InType)
{
    HazardType = InType;
}

void ARIPoopHazard::BeginPlay()
{
    Super::BeginPlay();
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ARIPoopHazard::HandleOverlap);
    ApplyPresentation();
}

void ARIPoopHazard::ApplyPresentation()
{
    const bool bCow = HazardType == ERIPoopHazardType::Cow;
    const FLinearColor Color = bCow
        ? FLinearColor(0.31f, 0.17f, 0.050f, 1.0f)
        : FLinearColor(0.145f, 0.055f, 0.012f, 1.0f);

    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (BaseMaterial)
    {
        for (UStaticMeshComponent* Mesh : {BlobA.Get(), BlobB.Get(), BlobC.Get()})
        {
            if (!Mesh) continue;
            if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Mesh))
            {
                Material->SetVectorParameterValue(TEXT("Color"), Color);
                Mesh->SetMaterial(0, Material);
            }
        }
    }

    if (StinkGlow)
    {
        // Cow patties are larger and deserve a slightly broader cue. Both remain
        // far dimmer than banana/egg pickup lights so visual language stays clear.
        StinkGlow->SetIntensity(bCow ? 270.0f : 190.0f);
        StinkGlow->SetAttenuationRadius(bCow ? 190.0f : 135.0f);
        StinkGlow->SetRelativeLocation(FVector(0.0f, 0.0f, bCow ? 28.0f : 31.0f));
    }

    if (bCow)
    {
        // Cow patty: broad and unmistakably flat. Its trigger/effect are unchanged.
        TriggerVolume->SetBoxExtent(FVector(108.0f, 88.0f, 36.0f));
        BlobA->SetRelativeLocation(FVector(-16.0f, 0.0f, -20.0f));
        BlobA->SetRelativeScale3D(FVector(0.78f, 0.61f, 0.12f));
        BlobB->SetRelativeLocation(FVector(46.0f, 24.0f, -18.0f));
        BlobB->SetRelativeScale3D(FVector(0.44f, 0.35f, 0.10f));
        BlobC->SetRelativeLocation(FVector(24.0f, -38.0f, -17.0f));
        BlobC->SetRelativeScale3D(FVector(0.38f, 0.31f, 0.09f));
    }
    else
    {
        // Dog poop: a smaller stacked pile, visually distinct from the cow patty.
        TriggerVolume->SetBoxExtent(FVector(58.0f, 48.0f, 28.0f));
        BlobA->SetRelativeLocation(FVector(-8.0f, 0.0f, -11.0f));
        BlobA->SetRelativeScale3D(FVector(0.24f, 0.21f, 0.24f));
        BlobB->SetRelativeLocation(FVector(9.0f, 5.0f, 8.0f));
        BlobB->SetRelativeScale3D(FVector(0.19f, 0.17f, 0.20f));
        BlobC->SetRelativeLocation(FVector(13.0f, -2.0f, 23.0f));
        BlobC->SetRelativeScale3D(FVector(0.13f, 0.12f, 0.15f));
    }
}

void ARIPoopHazard::HandleOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    ARIBikePawn* Bike = Cast<ARIBikePawn>(OtherActor);
    if (!Bike || !GetWorld() || !Bike->AreRaceControlsEnabled()) return;

    const double Now = GetWorld()->GetTimeSeconds();
    const TWeakObjectPtr<ARIBikePawn> BikeKey(Bike);
    const double Cooldown = HazardType == ERIPoopHazardType::Cow ? 3.5 : 2.5;

    if (const double* LastTime = LastTriggerTimes.Find(BikeKey))
    {
        if (Now - *LastTime < Cooldown)
        {
            return;
        }
    }
    LastTriggerTimes.Add(BikeKey, Now);

    if (HazardType == ERIPoopHazardType::Cow)
    {
        ApplyCowPoop(Bike);
    }
    else
    {
        ApplyDogPoop(Bike);
    }
}

void ARIPoopHazard::ApplyDogPoop(ARIBikePawn* Bike)
{
    if (!Bike) return;

    const float Side = FMath::RandBool() ? 1.0f : -1.0f;
    if (UStaticMeshComponent* Chassis = Bike->GetChassis())
    {
        Chassis->AddImpulse(Bike->GetActorRightVector() * (Side * 220.0f), NAME_None, true);
        Chassis->AddAngularImpulseInRadians(
            Bike->GetActorForwardVector() * (Side * 2.8f) + FVector::UpVector * (Side * 0.65f),
            NAME_None,
            true);
    }

    Bike->TriggerComicImpact(Side, TEXT("SKID! DOG POOP!"), 1.0f);
    RIPrototypeVisuals::PlayReaction(Bike, Side);
    RIAudioEvents::Play(this, TEXT("DogPoop"), Bike->GetActorLocation(), 0.95f, FMath::FRandRange(0.95f, 1.05f));
    SpawnMessEffect(Bike, false, 4.0f);
}

void ARIPoopHazard::ApplyCowPoop(ARIBikePawn* Bike)
{
    if (!Bike) return;

    if (UStaticMeshComponent* Chassis = Bike->GetChassis())
    {
        const FVector Velocity = Chassis->GetPhysicsLinearVelocity();
        const FVector Horizontal(Velocity.X, Velocity.Y, 0.0f);
        const FVector Slowed = Horizontal * 0.42f;
        Chassis->SetPhysicsLinearVelocity(FVector(Slowed.X, Slowed.Y, Velocity.Z));

        const float Side = FMath::RandBool() ? 1.0f : -1.0f;
        Chassis->AddAngularImpulseInRadians(
            Bike->GetActorForwardVector() * (Side * 1.45f) + FVector::UpVector * (Side * 0.35f),
            NAME_None,
            true);
    }

    Bike->TriggerComicImpact(0.0f, TEXT("SPLORCH! COW PATTY!"), 1.2f);
    RIPrototypeVisuals::PlayReaction(Bike, 1.0f);
    RIAudioEvents::Play(this, TEXT("CowPoop"), Bike->GetActorLocation(), 1.05f, FMath::FRandRange(0.90f, 1.00f));
    SpawnMessEffect(Bike, true, 6.5f);
}

void ARIPoopHazard::SpawnMessEffect(ARIBikePawn* Bike, const bool bCowMess, const float LifetimeSeconds)
{
    if (!Bike || !GetWorld()) return;

    for (TActorIterator<ARIPoopMessEffect> It(GetWorld()); It; ++It)
    {
        ARIPoopMessEffect* Existing = *It;
        if (Existing && Existing->GetAffectedBike() == Bike)
        {
            Existing->Refresh(bCowMess, LifetimeSeconds);
            return;
        }
    }

    const FTransform SpawnTransform(Bike->GetActorRotation(), Bike->GetActorLocation());
    AActor* DeferredActor = UGameplayStatics::BeginDeferredActorSpawnFromClass(
        GetWorld(),
        ARIPoopMessEffect::StaticClass(),
        SpawnTransform,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
        Bike);

    if (ARIPoopMessEffect* Effect = Cast<ARIPoopMessEffect>(DeferredActor))
    {
        Effect->Configure(Bike, bCowMess, LifetimeSeconds);
        UGameplayStatics::FinishSpawningActor(Effect, SpawnTransform);
    }
}
