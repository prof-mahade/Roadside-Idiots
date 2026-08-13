#include "AI/RIAIController.h"
#include "Vehicle/RIBikePawn.h"
#include "Vehicle/RIBikeMovementComponent.h"

ARIAIController::ARIAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f;
}

void ARIAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    Bike = Cast<ARIBikePawn>(InPawn);
}

void ARIAIController::SetRoute(const TArray<FVector>& InRoutePoints, int32 StartTargetIndex, float InLaneOffset)
{
    RoutePoints = InRoutePoints;
    TargetIndex = RoutePoints.Num() > 0 ? FMath::Abs(StartTargetIndex) % RoutePoints.Num() : 0;
    LaneOffset = InLaneOffset;
}

void ARIAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Bike || RoutePoints.Num() < 3) return;

    const int32 Count = RoutePoints.Num();
    const FVector BikeLocation = Bike->GetActorLocation();

    int32 NearestIndex = 0;
    float NearestDistanceSq = TNumericLimits<float>::Max();
    for (int32 Index = 0; Index < Count; ++Index)
    {
        FVector Delta = RoutePoints[Index] - BikeLocation;
        Delta.Z = 0.0f;
        const float DistanceSq = Delta.SizeSquared();
        if (DistanceSq < NearestDistanceSq)
        {
            NearestDistanceSq = DistanceSq;
            NearestIndex = Index;
        }
    }

    TargetIndex = (NearestIndex + 1) % Count;
    const int32 PrevIndex = (TargetIndex - 1 + Count) % Count;
    const int32 NextIndex = (TargetIndex + 1) % Count;
    const FVector Tangent = (RoutePoints[NextIndex] - RoutePoints[PrevIndex]).GetSafeNormal2D();
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();
    const FVector TargetPoint = RoutePoints[TargetIndex] + Right * LaneOffset;

    FVector ToTarget = TargetPoint - BikeLocation;
    ToTarget.Z = 0.0f;

    const float SpeedKph = FMath::Abs(Bike->GetBikeMovement()->GetForwardSpeedKph());

    static TMap<TWeakObjectPtr<ARIAIController>, float> LowMotionTimes;
    float& LowMotionTime = LowMotionTimes.FindOrAdd(this);
    if (SpeedKph < 7.0f && ToTarget.SizeSquared() > FMath::Square(WaypointReachDistance * 0.5f))
    {
        LowMotionTime += DeltaSeconds;
    }
    else
    {
        LowMotionTime = 0.0f;
    }

    if (LowMotionTime > 1.6f)
    {
        Bike->RecoverBike();
        LowMotionTime = 0.0f;
        return;
    }

    const FVector Desired = ToTarget.GetSafeNormal();
    const FVector Forward = Bike->GetActorForwardVector().GetSafeNormal2D();
    const float Dot = FMath::Clamp(FVector::DotProduct(Forward, Desired), -1.0f, 1.0f);
    const float CrossZ = FVector::CrossProduct(Forward, Desired).Z;
    const float Angle = FMath::Atan2(CrossZ, Dot);
    const float AbsAngle = FMath::Abs(Angle);
    const float Steering = FMath::Clamp(Angle / 0.55f, -1.0f, 1.0f);

    float DesiredSpeed = FMath::Min(TargetSpeedKph, 108.0f);
    if (AbsAngle > 0.70f)
    {
        DesiredSpeed = 58.0f;
    }
    else if (AbsAngle > 0.35f)
    {
        DesiredSpeed = 82.0f;
    }

    const float Brake = SpeedKph > DesiredSpeed + 6.0f ? 0.65f : 0.0f;
    float Throttle = Brake > 0.0f ? 0.10f : 1.0f;
    if (AbsAngle > 0.90f)
    {
        Throttle = FMath::Min(Throttle, 0.40f);
    }

    Bike->SetControlInputs(Throttle, Steering, Brake);
}
