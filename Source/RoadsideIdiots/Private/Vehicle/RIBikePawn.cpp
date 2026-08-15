#include "Vehicle/RIBikePawn.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Interaction/RIInteractionComponent.h"
#include "Items/RIBananaPeelHazard.h"
#include "Visual/RIPrototypeVisuals.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ARIBikePawn::ARIBikePawn()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    Chassis = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Chassis"));
    SetRootComponent(Chassis);
    Chassis->SetCollisionProfileName(TEXT("PhysicsActor"));
    Chassis->SetSimulatePhysics(true);
    Chassis->SetEnableGravity(true);
    Chassis->SetLinearDamping(0.15f);
    Chassis->SetAngularDamping(0.35f);
    Chassis->SetNotifyRigidBodyCollision(true);
    Chassis->SetGenerateOverlapEvents(true);
    Chassis->BodyInstance.bUseCCD = true;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

    if (CubeMesh.Succeeded())
    {
        Chassis->SetStaticMesh(CubeMesh.Object);
        Chassis->SetWorldScale3D(FVector(2.1f, 0.46f, 0.30f));
    }

    FrontWheelVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontWheelVisual"));
    FrontWheelVisual->SetupAttachment(Chassis);
    FrontWheelVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FrontWheelVisual->SetRelativeLocation(FVector(82.0f, 0.0f, -34.0f));
    FrontWheelVisual->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
    FrontWheelVisual->SetRelativeScale3D(FVector(0.70f, 0.70f, 0.16f));

    RearWheelVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearWheelVisual"));
    RearWheelVisual->SetupAttachment(Chassis);
    RearWheelVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RearWheelVisual->SetRelativeLocation(FVector(-82.0f, 0.0f, -34.0f));
    RearWheelVisual->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
    RearWheelVisual->SetRelativeScale3D(FVector(0.70f, 0.70f, 0.16f));

    if (CylinderMesh.Succeeded())
    {
        FrontWheelVisual->SetStaticMesh(CylinderMesh.Object);
        RearWheelVisual->SetStaticMesh(CylinderMesh.Object);
    }

    RiderBodyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RiderBodyVisual"));
    RiderBodyVisual->SetupAttachment(Chassis);
    RiderBodyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RiderBodyVisual->SetRelativeLocation(FVector(-12.0f, 0.0f, 82.0f));
    RiderBodyVisual->SetRelativeScale3D(FVector(0.46f, 0.30f, 0.72f));
    if (CubeMesh.Succeeded())
    {
        RiderBodyVisual->SetStaticMesh(CubeMesh.Object);
    }

    RiderHeadVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RiderHeadVisual"));
    RiderHeadVisual->SetupAttachment(Chassis);
    RiderHeadVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RiderHeadVisual->SetRelativeLocation(FVector(-5.0f, 0.0f, 157.0f));
    RiderHeadVisual->SetRelativeScale3D(FVector(0.28f));
    if (SphereMesh.Succeeded())
    {
        RiderHeadVisual->SetStaticMesh(SphereMesh.Object);
    }

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(Chassis);
    CameraBoom->TargetArmLength = 550.0f;
    CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 185.0f));
    CameraBoom->SetRelativeRotation(FRotator(-12.5f, 0.0f, 0.0f));
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 20.0f);
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 8.5f;
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraRotationLagSpeed = 10.0f;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritRoll = false;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->FieldOfView = 95.0f;

    BikeMovement = CreateDefaultSubobject<URIBikeMovementComponent>(TEXT("BikeMovement"));
    Health = CreateDefaultSubobject<URIHealthComponent>(TEXT("Health"));
    Participant = CreateDefaultSubobject<URIParticipantComponent>(TEXT("Participant"));
    Interaction = CreateDefaultSubobject<URIInteractionComponent>(TEXT("Interaction"));
}

void ARIBikePawn::BeginPlay()
{
    Super::BeginPlay();

    // Apply custom mass after engine/physics initialization. Calling this in the
    // native constructor causes UE to query physical-material data while the CDO
    // is being built, before GEngine exists.
    Chassis->SetMassOverrideInKg(NAME_None, 180.0f, true);

    if (GetWorld())
    {
        DamageEnabledAfterTime = GetWorld()->GetTimeSeconds() + 1.25;
    }

    Chassis->OnComponentHit.AddDynamic(this, &ARIBikePawn::HandleChassisHit);
    if (!bHasRecoveryTransform)
    {
        SetRecoveryTransform(GetActorTransform());
    }
    RIPrototypeVisuals::Setup(this);
}

void ARIBikePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("Throttle"), this, &ARIBikePawn::InputThrottle);
    PlayerInputComponent->BindAxis(TEXT("Steer"), this, &ARIBikePawn::InputSteering);
    PlayerInputComponent->BindAxis(TEXT("Brake"), this, &ARIBikePawn::InputBrake);
    PlayerInputComponent->BindAction(TEXT("InteractLeft"), IE_Pressed, this, &ARIBikePawn::InteractLeft);
    PlayerInputComponent->BindAction(TEXT("InteractRight"), IE_Pressed, this, &ARIBikePawn::InteractRight);
    PlayerInputComponent->BindAction(TEXT("UseItem"), IE_Pressed, this, &ARIBikePawn::UseItem);
    PlayerInputComponent->BindAction(TEXT("Recover"), IE_Pressed, this, &ARIBikePawn::RecoverBike);
    PlayerInputComponent->BindAction(TEXT("RestartRace"), IE_Pressed, this, &ARIBikePawn::RestartRace);
}

void ARIBikePawn::InputThrottle(float Value)
{
    PlayerThrottleInput = FMath::Clamp(Value, 0.0f, 1.0f);
    UpdatePlayerDriveInputs();
}

void ARIBikePawn::InputSteering(float Value)
{
    BikeMovement->SetSteeringInput(Value);
}

void ARIBikePawn::InputBrake(float Value)
{
    PlayerBrakeInput = FMath::Clamp(Value, 0.0f, 1.0f);
    UpdatePlayerDriveInputs();
}

void ARIBikePawn::UpdatePlayerDriveInputs()
{
    if (!BikeMovement) return;

    if (PlayerBrakeInput > KINDA_SMALL_NUMBER)
    {
        if (BikeMovement->GetForwardSpeedKph() > 4.0f)
        {
            BikeMovement->SetThrottleInput(0.0f);
            BikeMovement->SetBrakeInput(PlayerBrakeInput);
        }
        else
        {
            BikeMovement->SetBrakeInput(0.0f);
            BikeMovement->SetThrottleInput(-PlayerBrakeInput);
        }
    }
    else
    {
        BikeMovement->SetBrakeInput(0.0f);
        BikeMovement->SetThrottleInput(PlayerThrottleInput);
    }
}

void ARIBikePawn::SetControlInputs(float Throttle, float Steering, float Brake)
{
    BikeMovement->SetThrottleInput(Throttle);
    BikeMovement->SetSteeringInput(Steering);
    BikeMovement->SetBrakeInput(Brake);
}

void ARIBikePawn::InteractLeft()
{
    Interaction->TrySideInteraction(-1.0f);
}

void ARIBikePawn::InteractRight()
{
    Interaction->TrySideInteraction(1.0f);
}

void ARIBikePawn::AddBananaPeel(int32 Amount)
{
    if (!HasAuthority() || Amount <= 0) return;
    BananaPeelCount = FMath::Clamp(BananaPeelCount + Amount, 0, MaxBananaPeels);
}

void ARIBikePawn::UseItem()
{
    if (!HasAuthority() || !GetWorld()) return;

    if (BananaPeelCount <= 0)
    {
        if (GEngine && Participant && Participant->IsHumanControlled())
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Silver, TEXT("No banana peel. Find a glowing banana first."));
        }
        return;
    }

    // Start the peel near the rider and above the ground so gravity visibly
    // drops it behind the bike. Deferred spawning is critical: ConfigureSource
    // runs before BeginPlay/overlap events, preventing the player from slipping
    // on their own peel during the spawn frame.
    const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
    const FVector SpawnLocation = GetActorLocation() - Forward * 135.0f + FVector::UpVector * 95.0f;
    const FRotator SpawnRotation(0.0f, GetActorRotation().Yaw, 0.0f);
    const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

    AActor* DeferredActor = UGameplayStatics::BeginDeferredActorSpawnFromClass(
        this,
        ARIBananaPeelHazard::StaticClass(),
        SpawnTransform,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
        this);

    if (ARIBananaPeelHazard* Peel = Cast<ARIBananaPeelHazard>(DeferredActor))
    {
        Peel->ConfigureSource(this);
        UGameplayStatics::FinishSpawningActor(Peel, SpawnTransform);
        BananaPeelCount = FMath::Max(0, BananaPeelCount - 1);

        if (GEngine && Participant && Participant->IsHumanControlled())
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.2f, FColor::Yellow, TEXT("Peel dropped behind you. Bad citizenship achieved."));
        }
    }
}

void ARIBikePawn::RestartRace()
{
    if (!GetWorld()) return;

    const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
    if (!LevelName.IsEmpty())
    {
        UGameplayStatics::OpenLevel(this, FName(*LevelName), false);
    }
}

void ARIBikePawn::SetRecoveryTransform(const FTransform& InTransform)
{
    RecoveryTransform = InTransform;
    FRotator SafeRotation = RecoveryTransform.Rotator();
    SafeRotation.Pitch = 0.0f;
    SafeRotation.Roll = 0.0f;
    RecoveryTransform.SetRotation(SafeRotation.Quaternion());
    bHasRecoveryTransform = true;
}

void ARIBikePawn::HandleChassisHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HasAuthority() || !GetWorld()) return;

    const double Now = GetWorld()->GetTimeSeconds();
    if (Now < DamageEnabledAfterTime) return;
    if (Now - LastImpactTime < 0.85) return;

    // Ignore ordinary scraping and rider-to-rider rubbing. Only meaningful crashes
    // should move the condition bar through the physics collision path.
    const float ImpulseSize = NormalImpulse.Size();
    if (ImpulseSize > 30000.0f)
    {
        const float Cost = FMath::Clamp((ImpulseSize - 30000.0f) / 12000.0f, 1.0f, 7.0f);
        Health->ApplyImpact(Cost);
        LastImpactTime = Now;
    }
}

void ARIBikePawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    RIPrototypeVisuals::Update(this);

    const bool bTipped = GetActorUpVector().Z < 0.38f;
    const float HorizontalSpeed = Chassis->GetPhysicsLinearVelocity().Size2D();

    if (bTipped && !bCrashLatched)
    {
        bCrashLatched = true;
        if (HasAuthority() && GetWorld() && GetWorld()->GetTimeSeconds() >= DamageEnabledAfterTime)
        {
            Health->ApplyImpact(3.0f);
        }
    }
    else if (!bTipped)
    {
        bCrashLatched = false;
    }

    if (bTipped && HorizontalSpeed < 280.0f)
    {
        TippedStillTime += DeltaSeconds;
        if (TippedStillTime > 2.4f)
        {
            RecoverUprightHere();
        }
    }
    else
    {
        TippedStillTime = 0.0f;
    }
}

void ARIBikePawn::RecoverUprightHere()
{
    if (!Chassis) return;

    const FVector Location = GetActorLocation() + FVector::UpVector * 70.0f;
    const float Yaw = GetActorRotation().Yaw;
    Chassis->SetPhysicsLinearVelocity(FVector::ZeroVector);
    Chassis->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
    SetActorLocationAndRotation(Location, FRotator(0.0f, Yaw, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
    TippedStillTime = 0.0f;
    bCrashLatched = false;
}

void ARIBikePawn::RecoverBike()
{
    if (!Chassis) return;

    FTransform TargetTransform = bHasRecoveryTransform ? RecoveryTransform : GetActorTransform();
    FVector Location = TargetTransform.GetLocation();
    Location.Z = FMath::Max(Location.Z, 28.0f);
    const float Yaw = TargetTransform.Rotator().Yaw;

    Chassis->SetPhysicsLinearVelocity(FVector::ZeroVector);
    Chassis->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
    SetActorLocationAndRotation(Location, FRotator(0.0f, Yaw, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);

    PlayerThrottleInput = 0.0f;
    PlayerBrakeInput = 0.0f;
    BikeMovement->SetThrottleInput(0.0f);
    BikeMovement->SetBrakeInput(0.0f);
    BikeMovement->SetSteeringInput(0.0f);
    TippedStillTime = 0.0f;
    bCrashLatched = false;
}
