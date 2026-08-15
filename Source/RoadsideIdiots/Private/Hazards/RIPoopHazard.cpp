#include "Hazards/RIPoopHazard.h"

#include "Hazards/RIPoopMessEffect.h"
#include "Vehicle/RIBikePawn.h"
#include "Visual/RIPrototypeVisuals.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
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
        ? FLinearColor(0.29f, 0.16f, 0.045f, 1.0f)
        : FLinearColor(0.18f, 0.075f, 0.02f, 1.0f);

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

    if (bCow)
    {
        TriggerVolume->SetBoxExtent(FVector(125.0f, 100.0f, 38.0f));
        BlobA->SetRelativeLocation(FVector(-18.0f, 0.0f, -15.0f));
        BlobA->SetRelativeScale3D(FVector(1.15f, 0.90f, 0.22f));
        BlobB->SetRelativeLocation(FVector(58.0f, 28.0f, -9.0f));
        BlobB->SetRelativeScale3D(FVector(0.70f, 0.58f, 0.20f));
        BlobC->SetRelativeLocation(FVector(36.0f, -45.0f, -7.0f));
        BlobC->SetRelativeScale3D(FVector(0.58f, 0.46f, 0.18f));
    }
    else
    {
        TriggerVolume->SetBoxExtent(FVector(62.0f, 52.0f, 30.0f));
        BlobA->SetRelativeLocation(FVector(-12.0f, 0.0f, -17.0f));
        BlobA->SetRelativeScale3D(FVector(0.42f, 0.34f, 0.18f));
        BlobB->SetRelativeLocation(FVector(22.0f, 15.0f, -12.0f));
        BlobB->SetRelativeScale3D(FVector(0.28f, 0.24f, 0.15f));
        BlobC->SetRelativeLocation(FVector(15.0f, -18.0f, -10.0f));
        BlobC->SetRelativeScale3D(FVector(0.23f, 0.20f, 0.13f));
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
    if (!Bike || !GetWorld()) return;

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
    SpawnMessEffect(Bike, false, 4.0f);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.7f, FColor(165, 92, 38), TEXT("DOG POOP! Your dignity has left the road."));
    }
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
    SpawnMessEffect(Bike, true, 6.5f);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor(120, 72, 30), TEXT("COW PATTY! Speed reduced. Smell increased."));
    }
}

void ARIPoopHazard::SpawnMessEffect(ARIBikePawn* Bike, const bool bCowMess, const float LifetimeSeconds)
{
    if (!Bike || !GetWorld()) return;

    FActorSpawnParameters Params;
    Params.Owner = Bike;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    if (ARIPoopMessEffect* Effect = GetWorld()->SpawnActor<ARIPoopMessEffect>(
        ARIPoopMessEffect::StaticClass(),
        Bike->GetActorLocation(),
        Bike->GetActorRotation(),
        Params))
    {
        Effect->Configure(Bike, bCowMess, LifetimeSeconds);
    }
}
