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

private:
    UPrimitiveComponent* GetPhysicsBody() const;
    void UpdateGroundedState();
    void ApplyDrive(float DeltaTime, UPrimitiveComponent* Body);
    void ApplySteeringAndBalance(float DeltaTime, UPrimitiveComponent* Body);

    UPROPERTY(EditAnywhere, Category="Tuning|Speed")
    float MaxSpeedKph = 155.0f;

    UPROPERTY(EditAnywhere, Category="Tuning|Speed")
    float DriveAcceleration = 1250.0f;

    UPROPERTY(EditAnywhere, Category="Tuning|Speed")
    float BrakeStrength = 2.2f;

    UPROPERTY(EditAnywhere, Category="Tuning|Speed")
    float RollingDrag = 0.08f;

    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float SteeringAcceleration = 2.8f;

    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float MaxLeanDegrees = 31.0f;

    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float BalanceStrength = 18.0f;

    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float BalanceDamping = 6.0f;

    UPROPERTY(EditAnywhere, Category="Tuning|Handling")
    float GroundTraceLength = 145.0f;

    float ThrottleInput = 0.0f;
    float SteeringInput = 0.0f;
    float BrakeInput = 0.0f;
    bool bGrounded = false;
};
