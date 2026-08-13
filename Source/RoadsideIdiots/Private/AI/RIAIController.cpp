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
    const int32 PrevIndex = (TargetIndex - 1 + Count) % Count;
    const int32 NextIndex = (TargetIndex + 1) % Count;
    const FVector Tangent = (RoutePoints[NextIndex] - RoutePoints[PrevIndex]).GetSafeNormal2D();
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();
    const FVector TargetPoint = RoutePoints[TargetIndex] + Right * LaneOffset;
    FVector ToTarget = TargetPoint - Bike->GetActorLocation();
    ToTarget.Z = 0.0f;

    if (ToTarget.SizeSquared() < FMath::Square(WaypointReachDistance))
    {
        TargetIndex = (TargetIndex + 1) % Count;
        return;
    }

    const FVector Desired = ToTarget.GetSafeNormal();
    const FVector Forward = Bike->GetActorForwardVector().GetSafeNormal2D();
    const float Dot = FMath::Clamp(FVector::DotProduct(Forward, Desired), -1.0f, 1.0f);
    const float CrossZ = FVector::CrossProduct(Forward, Desired).Z;
    const float Angle = FMath::Atan2(CrossZ, Dot);
    const float Steering = FMath::Clamp(Angle / 0.75f, -1.0f, 1.0f);

    const float SpeedKph = FMath::Abs(Bike->GetBikeMovement()->GetForwardSpeedKph());
    float Brake = SpeedKph > TargetSpeedKph + 8.0f ? 0.45f : 0.0f;
    float Throttle = Brake > 0.0f ? 0.35f : 1.0f;
    if (FMath::Abs(Angle) > 1.05f)
    {
        Brake = FMath::Max(Brake, 0.55f);
        Throttle = 0.30f;
    }
    Bike->SetControlInputs(Throttle, Steering, Brake);
}
