#include "Vehicle/RIBikePawn.h"
#include "Vehicle/RIBikeMovementComponent.h"
#include "Core/RIHealthComponent.h"
#include "Core/RIParticipantComponent.h"
#include "Interaction/RIInteractionComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
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
    Chassis->SetMassOverrideInKg(NAME_None, 180.0f, true);

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
    CameraBoom->TargetArmLength = 540.0f;
    CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 145.0f));
    CameraBoom->SetRelativeRotation(FRotator(-9.0f, 0.0f, 0.0f));
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 9.0f;
    CameraBoom->bInheritRoll = false;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

    BikeMovement = CreateDefaultSubobject<URIBikeMovementComponent>(TEXT("BikeMovement"));
    Health = CreateDefaultSubobject<URIHealthComponent>(TEXT("Health"));
    Participant = CreateDefaultSubobject<URIParticipantComponent>(TEXT("Participant"));
    Interaction = CreateDefaultSubobject<URIInteractionComponent>(TEXT("Interaction"));
}

void ARIBikePawn::BeginPlay()
{
    Super::BeginPlay();
    Chassis->OnComponentHit.AddDynamic(this, &ARIBikePawn::HandleChassisHit);
}

void ARIBikePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("Throttle"), this, &ARIBikePawn::InputThrottle);
    PlayerInputComponent->BindAxis(TEXT("Steer"), this, &ARIBikePawn::InputSteering);
    PlayerInputComponent->BindAxis(TEXT("Brake"), this, &ARIBikePawn::InputBrake);
    PlayerInputComponent->BindAction(TEXT("InteractLeft"), IE_Pressed, this, &ARIBikePawn::InteractLeft);
    PlayerInputComponent->BindAction(TEXT("InteractRight"), IE_Pressed, this, &ARIBikePawn::InteractRight);
    PlayerInputComponent->BindAction(TEXT("Recover"), IE_Pressed, this, &ARIBikePawn::RecoverBike);
}

void ARIBikePawn::InputThrottle(float Value)
{
    BikeMovement->SetThrottleInput(Value);
}

void ARIBikePawn::InputSteering(float Value)
{
    BikeMovement->SetSteeringInput(Value);
}

void ARIBikePawn::InputBrake(float Value)
{
    BikeMovement->SetBrakeInput(Value);
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

void ARIBikePawn::HandleChassisHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HasAuthority() || !GetWorld())
    {
        return;
    }

    const double Now = GetWorld()->GetTimeSeconds();
    if (Now - LastImpactTime < 0.45)
    {
        return;
    }

    const float ImpulseSize = NormalImpulse.Size();
    if (ImpulseSize > 18000.0f)
    {
        const float Cost = FMath::Clamp((ImpulseSize - 18000.0f) / 7000.0f, 1.0f, 16.0f);
        Health->ApplyImpact(Cost);
        LastImpactTime = Now;
    }
}

void ARIBikePawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    const bool bTipped = GetActorUpVector().Z < 0.38f;
    const float HorizontalSpeed = Chassis->GetPhysicsLinearVelocity().Size2D();

    if (bTipped && !bCrashLatched)
    {
        bCrashLatched = true;
        if (HasAuthority())
        {
            Health->ApplyImpact(4.0f);
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
            RecoverBike();
        }
    }
    else
    {
        TippedStillTime = 0.0f;
    }
}

void ARIBikePawn::RecoverBike()
{
    if (!Chassis)
    {
        return;
    }

    const FVector Location = GetActorLocation() + FVector::UpVector * 105.0f;
    const float Yaw = GetActorRotation().Yaw;
    Chassis->SetPhysicsLinearVelocity(FVector::ZeroVector);
    Chassis->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
    SetActorLocationAndRotation(Location, FRotator(0.0f, Yaw, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
    TippedStillTime = 0.0f;
    bCrashLatched = false;
}
