#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RIBikeMovementComponent.generated.h"

class UPrimitiveComponent;

UCLASS(ClassGroup=(RoadsideIdiots), meta=(BlueprintSpawnableComponent))
class ROADSIDEIDIOTS_API URIBikeMovementComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URIBikeMovementComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Bike")
    void SetThrottleInput(float Value) { ThrottleInput = FMath::Clamp(Value, -1.0f, 1.0f); }

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Bike")
    void SetSteeringInput(float Value) { SteeringInput = FMath::Clamp(Value, -1.0f, 1.0f); }

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Bike")
    void SetBrakeInput(float Value) { BrakeInput = FMath::Clamp(Value, 0.0f, 1.0f); }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    float GetForwardSpeedKph() const;

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    bool IsGrounded() const { return bGrounded; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    float GetThrottleInput() const { return ThrottleInput; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    float GetSteeringInput() const { return SteeringInput; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    float GetBrakeInput() const { return BrakeInput; }

private:
    UPrimitiveComponent* GetPhysicsBody() const;
    void UpdateGroundedState();
    void ApplyDrive(float DeltaTime, UPrimitiveComponent* Body);
    void ApplySteeringAndBalance(float DeltaTime, UPrimitiveComponent* Body);

    UPROPERTY(EditAnywhere, Category="Tuning|Speed")
    float MaxSpeedKph = 155.0f;

    UPROPERTY(EditAnywhere, Category="Tuning|Speed")
    float MaxReverseSpeedKph = 32.0f;

    UPROPERTY(EditAnywhere, Category="Tuning|Speed")
    float DriveAcceleration = 1550.0f;

    UPROPERTY(EditAnywhere, Category="Tuning|Speed")
    float ReverseAcceleration = 900.0f;

    UPROPERTY(EditAnywhere, Category="Tuning|Speed")
    float BrakeStrength = 2.6f;

    UPROPERTY(EditAnywhere, Category="Tuning|Speed")
    float RollingDrag = 0.07f;

    // VPR-24D: steering input is a requested yaw rate, not an open-loop torque.
    // This gives the vehicle an inner damping loop: when the requested yaw rate
    // is reached, steering torque naturally falls toward zero instead of
    // continuing to accelerate rotation and overshooting the road centerline.
    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float MaxYawRateLowSpeed = 1.55f;

    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float MaxYawRateHighSpeed = 0.82f;

    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float YawRateResponse = 6.5f;

    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float MaxYawAcceleration = 8.0f;

    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float MaxLeanDegrees = 34.0f;

    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float BalanceStrength = 20.0f;

    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float BalanceDamping = 7.0f;

    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float LateralGrip = 4.6f;

    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float GroundTraceLength = 145.0f;

    float ThrottleInput = 0.0f;
    float SteeringInput = 0.0f;
    float BrakeInput = 0.0f;
    bool bGrounded = false;
};
