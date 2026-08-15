#include "Vehicle/RIBikePawn.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Interaction/RIInteractionComponent.h"
#include "Items/RIBananaPeelHazard.h"
#include "Items/RIRottenEggProjectile.h"
#include "Race/RIRaceManager.h"
#include "Visual/RIPrototypeVisuals.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
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

    Chassis->SetMassOverrideInKg(NAME_None, 180.0f, true);

    if (GetWorld())
    {
        for (TActorIterator<ARIRaceManager> It(GetWorld()); It; ++It)
        {
            CachedRaceManager = *It;
            break;
        }

        const double Now = GetWorld()->GetTimeSeconds();
        const float PreRaceGrace = CachedRaceManager ? CachedRaceManager->GetSecondsUntilStart() + 0.75f : 1.25f;
        DamageEnabledAfterTime = Now + FMath::Max(1.25f, PreRaceGrace);
    }

    Chassis->OnComponentHit.AddDynamic(this, &ARIBikePawn::HandleChassisHit);
    if (!bHasRecoveryTransform)
    {
        SetRecoveryTransform(GetActorTransform());
    }
    RIPrototypeVisuals::Setup(this);
}

bool ARIBikePawn::IsRaceInputEnabled() const
{
    return !CachedRaceManager || CachedRaceManager->IsRaceStarted();
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
    if (!BikeMovement) return;
    BikeMovement->SetSteeringInput(IsRaceInputEnabled() ? Value : 0.0f);
}

void ARIBikePawn::InputBrake(float Value)
{
    PlayerBrakeInput = FMath::Clamp(Value, 0.0f, 1.0f);
    UpdatePlayerDriveInputs();
}

void ARIBikePawn::UpdatePlayerDriveInputs()
{
    if (!BikeMovement) return;

    if (!IsRaceInputEnabled())
    {
        BikeMovement->SetThrottleInput(0.0f);
        BikeMovement->SetBrakeInput(0.0f);
        BikeMovement->SetSteeringInput(0.0f);
        return;
    }

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
    if (!BikeMovement) return;

    if (!IsRaceInputEnabled())
    {
        BikeMovement->SetThrottleInput(0.0f);
        BikeMovement->SetSteeringInput(0.0f);
        BikeMovement->SetBrakeInput(0.0f);
        return;
    }

    BikeMovement->SetThrottleInput(Throttle);
    BikeMovement->SetSteeringInput(Steering);
    BikeMovement->SetBrakeInput(Brake);
}

void ARIBikePawn::InteractLeft()
{
    if (IsRaceInputEnabled() && Interaction)
    {
        Interaction->TrySideInteraction(-1.0f);
    }
}

void ARIBikePawn::InteractRight()
{
    if (IsRaceInputEnabled() && Interaction)
    {
        Interaction->TrySideInteraction(1.0f);
    }
}

void ARIBikePawn::AddBananaPeel(int32 Amount)
{
    if (!HasAuthority() || Amount <= 0) return;
    BananaPeelCount = FMath::Clamp(BananaPeelCount + Amount, 0, MaxBananaPeels);
}

void ARIBikePawn::AddRottenEgg(int32 Amount)
{
    if (!HasAuthority() || Amount <= 0) return;
    RottenEggCount = FMath::Clamp(RottenEggCount + Amount, 0, MaxRottenEggs);
}

void ARIBikePawn::TriggerComicImpact(float Side, const FString& Text, float Duration)
{
    ComicImpactText = Text;
    ComicImpactDuration = FMath::Clamp(Duration, 0.25f, 1.50f);

    if (const UWorld* World = GetWorld())
    {
        ComicImpactStartedAt = World->GetTimeSeconds();
        ComicImpactExpiresAt = ComicImpactStartedAt + ComicImpactDuration;
    }

    if (Participant && Participant->IsHumanControlled())
    {
        const float SignedSide = Side < 0.0f ? -1.0f : 1.0f;
        CameraKickYaw = FMath::Clamp(CameraKickYaw + SignedSide * 4.0f, -6.0f, 6.0f);
        CameraKickRoll = FMath::Clamp(CameraKickRoll - SignedSide * 2.8f, -4.0f, 4.0f);
    }
}

bool ARIBikePawn::GetActiveComicImpact(FString& OutText, float& OutAlpha) const
{
    OutText.Reset();
    OutAlpha = 0.0f;

    const UWorld* World = GetWorld();
    if (!World || ComicImpactText.IsEmpty()) return false;

    const double Now = World->GetTimeSeconds();
    if (Now >= ComicImpactExpiresAt) return false;

    OutText = ComicImpactText;
    const double Remaining = ComicImpactExpiresAt - Now;
    OutAlpha = FMath::Clamp(static_cast<float>(Remaining / FMath::Max(0.01f, ComicImpactDuration)), 0.0f, 1.0f);
    return true;
}

bool ARIBikePawn::DropBananaPeel()
{
    if (!HasAuthority() || !GetWorld() || BananaPeelCount <= 0 || !IsRaceInputEnabled()) return false;

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
        return true;
    }

    return false;
}

bool ARIBikePawn::ThrowRottenEggAt(ARIBikePawn* TargetBike)
{
    if (!HasAuthority() || !GetWorld() || RottenEggCount <= 0 || !IsRaceInputEnabled()) return false;

    const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
    const FVector SpawnLocation = GetActorLocation() + Forward * 150.0f + FVector::UpVector * 130.0f;

    FVector AimDirection = Forward;
    if (TargetBike && TargetBike != this)
    {
        FVector PredictedTarget = TargetBike->GetActorLocation() + FVector::UpVector * 95.0f;
        if (UStaticMeshComponent* TargetChassis = TargetBike->GetChassis())
        {
            PredictedTarget += TargetChassis->GetPhysicsLinearVelocity() * 0.22f;
        }

        const FVector ToPredicted = PredictedTarget - SpawnLocation;
        if (!ToPredicted.IsNearlyZero())
        {
            AimDirection = ToPredicted.GetSafeNormal();
        }
    }

    const FTransform SpawnTransform(AimDirection.Rotation(), SpawnLocation);
    AActor* DeferredActor = UGameplayStatics::BeginDeferredActorSpawnFromClass(
        this,
        ARIRottenEggProjectile::StaticClass(),
        SpawnTransform,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
        this);

    if (ARIRottenEggProjectile* Projectile = Cast<ARIRottenEggProjectile>(DeferredActor))
    {
        Projectile->ConfigureSource(this);
        UGameplayStatics::FinishSpawningActor(Projectile, SpawnTransform);
        RottenEggCount = FMath::Max(0, RottenEggCount - 1);
        return true;
    }

    return false;
}

void ARIBikePawn::UseItem()
{
    if (!IsRaceInputEnabled()) return;
    DropBananaPeel();
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

    DizzyTimeRemaining = FMath::Max(0.0f, DizzyTimeRemaining - DeltaSeconds);
    CameraKickYaw = FMath::FInterpTo(CameraKickYaw, 0.0f, DeltaSeconds, 10.0f);
    CameraKickRoll = FMath::FInterpTo(CameraKickRoll, 0.0f, DeltaSeconds, 11.0f);

    float DizzyYaw = 0.0f;
    float DizzyRoll = 0.0f;
    if (DizzyTimeRemaining > 0.0f && GetWorld() && Participant && Participant->IsHumanControlled())
    {
        const float Now = GetWorld()->GetTimeSeconds();
        const float Strength = FMath::Clamp(DizzyTimeRemaining / 1.8f, 0.0f, 1.0f);
        DizzyYaw = FMath::Sin(Now * 9.0f) * 2.8f * Strength;
        DizzyRoll = FMath::Sin(Now * 12.0f + 0.8f) * 2.2f * Strength;
    }

    if (CameraBoom)
    {
        CameraBoom->SetRelativeRotation(FRotator(-12.5f, CameraKickYaw + DizzyYaw, CameraKickRoll + DizzyRoll));
    }

    const bool bTipped = GetActorUpVector().Z < 0.38f;
    const float HorizontalSpeed = Chassis->GetPhysicsLinearVelocity().Size2D();

    if (bTipped && !bCrashLatched)
    {
        bCrashLatched = true;
        DizzyTimeRemaining = 1.8f;
        TriggerComicImpact(1.0f, TEXT("DIZZY!"), 1.35f);
        RIPrototypeVisuals::PlayReaction(this, 1.0f);

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
    DizzyTimeRemaining = 0.0f;
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
    DizzyTimeRemaining = 0.0f;
    bCrashLatched = false;
}
