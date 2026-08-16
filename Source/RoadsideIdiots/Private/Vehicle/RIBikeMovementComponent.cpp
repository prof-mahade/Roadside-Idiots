#include "Vehicle/RIBikeMovementComponent.h"
#include "Audio/RIAudioEvents.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

URIBikeMovementComponent::URIBikeMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void URIBikeMovementComponent::BeginPlay()
{
    Super::BeginPlay();
}

UPrimitiveComponent* URIBikeMovementComponent::GetPhysicsBody() const
{
    return GetOwner() ? Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent()) : nullptr;
}

float URIBikeMovementComponent::GetForwardSpeedKph() const
{
    const UPrimitiveComponent* Body = GetPhysicsBody();
    if (!Body || !GetOwner()) return 0.0f;
    const float ForwardSpeedCms = FVector::DotProduct(Body->GetPhysicsLinearVelocity(), GetOwner()->GetActorForwardVector());
    return ForwardSpeedCms * 0.036f;
}

void URIBikeMovementComponent::UpdateGroundedState()
{
    bGrounded = false;
    if (!GetWorld() || !GetOwner()) return;
    const FVector Start = GetOwner()->GetActorLocation();
    const FVector End = Start - FVector::UpVector * GroundTraceLength;
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(RIBikeGroundTrace), false, GetOwner());
    bGrounded = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params) && Hit.bBlockingHit;
}

void URIBikeMovementComponent::ApplyDrive(float DeltaTime, UPrimitiveComponent* Body)
{
    if (!Body || !GetOwner()) return;

    const FVector Forward = GetOwner()->GetActorForwardVector().GetSafeNormal2D();
    const FVector Velocity = Body->GetPhysicsLinearVelocity();
    const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.0f);
    const float MaxSpeedCms = MaxSpeedKph / 0.036f;
    const float MaxReverseCms = MaxReverseSpeedKph / 0.036f;
    const float ForwardSpeed = FVector::DotProduct(HorizontalVelocity, Forward);

    if (bGrounded && ThrottleInput > KINDA_SMALL_NUMBER && ForwardSpeed < MaxSpeedCms)
    {
        Body->AddForce(Forward * (ThrottleInput * DriveAcceleration), NAME_None, true);
    }
    else if (bGrounded && ThrottleInput < -KINDA_SMALL_NUMBER && ForwardSpeed > -MaxReverseCms)
    {
        Body->AddForce(Forward * (ThrottleInput * ReverseAcceleration), NAME_None, true);
    }

    if (bGrounded && BrakeInput > KINDA_SMALL_NUMBER && !HorizontalVelocity.IsNearlyZero())
    {
        Body->AddForce(-HorizontalVelocity * (BrakeStrength * BrakeInput), NAME_None, true);
    }

    if (bGrounded && !HorizontalVelocity.IsNearlyZero())
    {
        Body->AddForce(-HorizontalVelocity * RollingDrag, NAME_None, true);
    }

    const float HorizontalSpeed = HorizontalVelocity.Size();
    if (HorizontalSpeed > MaxSpeedCms && ForwardSpeed > 0.0f)
    {
        const FVector LimitedHorizontal = HorizontalVelocity.GetSafeNormal() * MaxSpeedCms;
        Body->SetPhysicsLinearVelocity(FVector(LimitedHorizontal.X, LimitedHorizontal.Y, Velocity.Z));
    }
}

void URIBikeMovementComponent::ApplySteeringAndBalance(float DeltaTime, UPrimitiveComponent* Body)
{
    if (!Body || !GetOwner()) return;

    const FVector Forward = GetOwner()->GetActorForwardVector().GetSafeNormal();
    const FVector Right = GetOwner()->GetActorRightVector().GetSafeNormal2D();
    const FVector CurrentUp = GetOwner()->GetActorUpVector().GetSafeNormal();
    const float SpeedKph = FMath::Abs(GetForwardSpeedKph());
    const float SpeedAlpha = FMath::Clamp(SpeedKph / 130.0f, 0.0f, 1.0f);

    const float TargetLean = -SteeringInput * MaxLeanDegrees * SpeedAlpha;
    const FQuat LeanRotation(Forward, FMath::DegreesToRadians(TargetLean));
    const FVector DesiredUp = LeanRotation.RotateVector(FVector::UpVector).GetSafeNormal();

    const FVector AngularVelocity = Body->GetPhysicsAngularVelocityInRadians();
    const FVector TiltAngularVelocity = AngularVelocity - CurrentUp * FVector::DotProduct(AngularVelocity, CurrentUp);
    const FVector BalanceError = FVector::CrossProduct(CurrentUp, DesiredUp);
    Body->AddTorqueInRadians(BalanceError * BalanceStrength - TiltAngularVelocity * BalanceDamping, NAME_None, true);

    if (bGrounded)
    {
        const FVector Velocity = Body->GetPhysicsLinearVelocity();
        const float LateralSpeed = FVector::DotProduct(Velocity, Right);
        Body->AddForce(-Right * (LateralSpeed * LateralGrip), NAME_None, true);
    }

    if (bGrounded)
    {
        // VPR-24D: close the steering loop around yaw rate. The old model added
        // yaw torque directly from SteeringInput every physics tick; therefore a
        // constant command kept increasing yaw velocity until the AI reversed
        // the command, which produced repeated left/right overshoot.
        //
        // SteeringInput now means "fraction of desired yaw rate". At high speed
        // the allowed yaw rate is lower, making the same input less twitchy.
        const float DirectionSign = GetForwardSpeedKph() < -2.0f ? -1.0f : 1.0f;
        const float MaxYawRate = FMath::Lerp(MaxYawRateLowSpeed, MaxYawRateHighSpeed, SpeedAlpha);
        const float DesiredYawRate = SteeringInput * MaxYawRate * DirectionSign;
        const float CurrentYawRate = FVector::DotProduct(AngularVelocity, FVector::UpVector);
        const float YawRateError = DesiredYawRate - CurrentYawRate;
        const float YawAcceleration = FMath::Clamp(
            YawRateError * YawRateResponse,
            -MaxYawAcceleration,
            MaxYawAcceleration);

        // Apply yaw about world-up. Using the leaned DesiredUp as the yaw axis
        // coupled steering torque back into roll and made correction less clean.
        Body->AddTorqueInRadians(FVector::UpVector * YawAcceleration, NAME_None, true);
    }
}

void URIBikeMovementComponent::UpdateDriveAudio(float DeltaTime, UPrimitiveComponent* Body)
{
    if (!Body || !GetOwner()) return;

    const APawn* PawnOwner = Cast<APawn>(GetOwner());
    if (!PawnOwner || !PawnOwner->IsPlayerControlled()) return;

    EngineAudioAccumulator += FMath::Max(0.0f, DeltaTime);
    SkidAudioCooldownRemaining = FMath::Max(0.0f, SkidAudioCooldownRemaining - DeltaTime);

    const float SpeedKph = FMath::Abs(GetForwardSpeedKph());
    const float SpeedAlpha = FMath::Clamp(SpeedKph / FMath::Max(1.0f, MaxSpeedKph), 0.0f, 1.0f);
    const float EngineLoad = FMath::Clamp(FMath::Abs(ThrottleInput), 0.0f, 1.0f);
    const float EngineActivity = FMath::Clamp(EngineLoad * 0.68f + SpeedAlpha * 0.32f, 0.0f, 1.0f);

    if (bGrounded && (EngineLoad > 0.04f || SpeedKph > 7.0f))
    {
        const float PulseInterval = FMath::Lerp(0.205f, 0.105f, EngineActivity);
        if (EngineAudioAccumulator >= PulseInterval)
        {
            EngineAudioAccumulator = FMath::Fmod(EngineAudioAccumulator, PulseInterval);
            const float Volume = 0.12f + EngineLoad * 0.16f + SpeedAlpha * 0.08f;
            const float Pitch = 0.78f + SpeedAlpha * 0.58f + EngineLoad * 0.12f;
            RIAudioEvents::Play(
                GetOwner(),
                FName(TEXT("EnginePulse")),
                GetOwner()->GetActorLocation(),
                Volume,
                Pitch);
        }
    }
    else
    {
        EngineAudioAccumulator = FMath::Min(EngineAudioAccumulator, 0.20f);
    }

    if (!bGrounded || SkidAudioCooldownRemaining > 0.0f || SpeedKph < 28.0f)
    {
        return;
    }

    const FVector Right = GetOwner()->GetActorRightVector().GetSafeNormal2D();
    const FVector HorizontalVelocity = FVector(Body->GetPhysicsLinearVelocity().X, Body->GetPhysicsLinearVelocity().Y, 0.0f);
    const float LateralSpeedKph = FMath::Abs(FVector::DotProduct(HorizontalVelocity, Right)) * 0.036f;
    const bool bHardBraking = BrakeInput > 0.58f && SpeedKph > 38.0f;
    const bool bSliding = LateralSpeedKph > FMath::Lerp(14.0f, 24.0f, SpeedAlpha);

    if (bHardBraking || bSliding)
    {
        const float BrakeIntensity = FMath::Clamp((BrakeInput - 0.55f) / 0.45f, 0.0f, 1.0f);
        const float SlideIntensity = FMath::Clamp(LateralSpeedKph / 34.0f, 0.0f, 1.0f);
        const float SkidIntensity = FMath::Max(BrakeIntensity, SlideIntensity);
        const float Volume = FMath::Lerp(0.08f, 0.24f, SkidIntensity);
        const float Pitch = FMath::Lerp(0.90f, 1.18f, SpeedAlpha);

        RIAudioEvents::Play(
            GetOwner(),
            FName(TEXT("TireSkid")),
            GetOwner()->GetActorLocation(),
            Volume,
            Pitch);

        SkidAudioCooldownRemaining = FMath::Lerp(0.22f, 0.14f, SkidIntensity);
    }
}

void URIBikeMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UPrimitiveComponent* Body = GetPhysicsBody();
    if (!Body || !Body->IsSimulatingPhysics()) return;
    UpdateGroundedState();
    ApplyDrive(DeltaTime, Body);
    ApplySteeringAndBalance(DeltaTime, Body);
    UpdateDriveAudio(DeltaTime, Body);
}
