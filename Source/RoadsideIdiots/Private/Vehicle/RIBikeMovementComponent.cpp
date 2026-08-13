#include "Vehicle/RIBikeMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"

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
    if (!Body || !GetOwner())
    {
        return 0.0f;
    }

    const float ForwardSpeedCms = FVector::DotProduct(Body->GetPhysicsLinearVelocity(), GetOwner()->GetActorForwardVector());
    return ForwardSpeedCms * 0.036f;
}

void URIBikeMovementComponent::UpdateGroundedState()
{
    bGrounded = false;
    if (!GetWorld() || !GetOwner())
    {
        return;
    }

    const FVector Start = GetOwner()->GetActorLocation();
    const FVector End = Start - FVector::UpVector * GroundTraceLength;
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(RIBikeGroundTrace), false, GetOwner());
    bGrounded = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params) && Hit.bBlockingHit;
}

void URIBikeMovementComponent::ApplyDrive(float DeltaTime, UPrimitiveComponent* Body)
{
    if (!Body || !GetOwner())
    {
        return;
    }

    const FVector Forward = GetOwner()->GetActorForwardVector().GetSafeNormal2D();
    const FVector Velocity = Body->GetPhysicsLinearVelocity();
    const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.0f);
    const float MaxSpeedCms = MaxSpeedKph / 0.036f;
    const float ForwardSpeed = FVector::DotProduct(HorizontalVelocity, Forward);

    if (bGrounded && FMath::Abs(ThrottleInput) > KINDA_SMALL_NUMBER)
    {
        const bool bBelowLimit = FMath::Abs(ForwardSpeed) < MaxSpeedCms;
        const bool bTryingToSlow = ForwardSpeed * ThrottleInput < 0.0f;
        if (bBelowLimit || bTryingToSlow)
        {
            Body->AddForce(Forward * (ThrottleInput * DriveAcceleration), NAME_None, true);
        }
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
    if (HorizontalSpeed > MaxSpeedCms)
    {
        const FVector LimitedHorizontal = HorizontalVelocity.GetSafeNormal() * MaxSpeedCms;
        Body->SetPhysicsLinearVelocity(FVector(LimitedHorizontal.X, LimitedHorizontal.Y, Velocity.Z));
    }
}

void URIBikeMovementComponent::ApplySteeringAndBalance(float DeltaTime, UPrimitiveComponent* Body)
{
    if (!Body || !GetOwner())
    {
        return;
    }

    const FVector Forward = GetOwner()->GetActorForwardVector().GetSafeNormal();
    const FVector CurrentUp = GetOwner()->GetActorUpVector().GetSafeNormal();
    const float SpeedAlpha = FMath::Clamp(FMath::Abs(GetForwardSpeedKph()) / 70.0f, 0.0f, 1.0f);
    const float TargetLean = -SteeringInput * MaxLeanDegrees * SpeedAlpha;
    const FQuat LeanRotation(Forward, FMath::DegreesToRadians(TargetLean));
    const FVector DesiredUp = LeanRotation.RotateVector(FVector::UpVector).GetSafeNormal();

    FVector AngularVelocity = Body->GetPhysicsAngularVelocityInRadians();
    const FVector TiltAngularVelocity = AngularVelocity - CurrentUp * FVector::DotProduct(AngularVelocity, CurrentUp);
    const FVector BalanceError = FVector::CrossProduct(CurrentUp, DesiredUp);
    const FVector BalanceAcceleration = BalanceError * BalanceStrength - TiltAngularVelocity * BalanceDamping;
    Body->AddTorqueInRadians(BalanceAcceleration, NAME_None, true);

    if (bGrounded && FMath::Abs(SteeringInput) > KINDA_SMALL_NUMBER)
    {
        const float SteeringAuthority = FMath::Lerp(0.22f, 1.0f, SpeedAlpha);
        Body->AddTorqueInRadians(DesiredUp * (SteeringInput * SteeringAcceleration * SteeringAuthority), NAME_None, true);
    }
}

void URIBikeMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UPrimitiveComponent* Body = GetPhysicsBody();
    if (!Body || !Body->IsSimulatingPhysics())
    {
        return;
    }

    UpdateGroundedState();
    ApplyDrive(DeltaTime, Body);
    ApplySteeringAndBalance(DeltaTime, Body);
}
