#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RIBikePawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class URIBikeMovementComponent;
class URIHealthComponent;
class URIParticipantComponent;
class URIInteractionComponent;

UCLASS()
class ROADSIDEIDIOTS_API ARIBikePawn : public APawn
{
    GENERATED_BODY()

public:
    ARIBikePawn();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Bike")
    void SetControlInputs(float Throttle, float Steering, float Brake);

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Bike")
    void RecoverBike();

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Bike")
    void SetRecoveryTransform(const FTransform& InTransform);

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    UStaticMeshComponent* GetChassis() const { return Chassis; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    URIBikeMovementComponent* GetBikeMovement() const { return BikeMovement; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    URIHealthComponent* GetHealthComponent() const { return Health; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    URIParticipantComponent* GetParticipantComponent() const { return Participant; }

private:
    void InputThrottle(float Value);
    void InputSteering(float Value);
    void InputBrake(float Value);
    void UpdatePlayerDriveInputs();
    void InteractLeft();
    void InteractRight();
    void RecoverUprightHere();

    UFUNCTION()
    void HandleChassisHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UStaticMeshComponent> Chassis;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UStaticMeshComponent> FrontWheelVisual;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UStaticMeshComponent> RearWheelVisual;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UStaticMeshComponent> RiderBodyVisual;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UStaticMeshComponent> RiderHeadVisual;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<URIBikeMovementComponent> BikeMovement;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<URIHealthComponent> Health;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<URIParticipantComponent> Participant;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<URIInteractionComponent> Interaction;

    FTransform RecoveryTransform = FTransform::Identity;
    bool bHasRecoveryTransform = false;
    float PlayerThrottleInput = 0.0f;
    float PlayerBrakeInput = 0.0f;
    float TippedStillTime = 0.0f;
    bool bCrashLatched = false;
    double LastImpactTime = -100.0;
};
