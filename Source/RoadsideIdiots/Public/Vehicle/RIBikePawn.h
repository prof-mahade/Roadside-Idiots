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

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Items")
    void AddBananaPeel(int32 Amount = 1);

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Items")
    int32 GetBananaPeelCount() const { return BananaPeelCount; }

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Items")
    void AddRottenEgg(int32 Amount = 1);

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Items")
    int32 GetRottenEggCount() const { return RottenEggCount; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Items")
    int32 GetMaxRottenEggs() const { return MaxRottenEggs; }

    // Shared item actions. Human input and AI both call these same functions so
    // bots do not get a separate fake item implementation.
    bool DropBananaPeel();
    bool ThrowRottenEggAt(ARIBikePawn* TargetBike = nullptr);

    // Short-lived prototype feedback used by the HUD and local camera.
    void TriggerComicImpact(float Side, const FString& Text, float Duration = 0.70f);
    bool GetActiveComicImpact(FString& OutText, float& OutAlpha) const;

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    UStaticMeshComponent* GetChassis() const { return Chassis; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    URIBikeMovementComponent* GetBikeMovement() const { return BikeMovement; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    URIHealthComponent* GetHealthComponent() const { return Health; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    URIParticipantComponent* GetParticipantComponent() const { return Participant; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Bike")
    URIInteractionComponent* GetInteractionComponent() const { return Interaction; }

private:
    void InputThrottle(float Value);
    void InputSteering(float Value);
    void InputBrake(float Value);
    void UpdatePlayerDriveInputs();
    void InteractLeft();
    void InteractRight();
    void UseItem();
    void RestartRace();
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

    UPROPERTY(VisibleAnywhere, Category="Items")
    int32 BananaPeelCount = 0;

    UPROPERTY(EditDefaultsOnly, Category="Items")
    int32 MaxBananaPeels = 3;

    UPROPERTY(VisibleAnywhere, Category="Items")
    int32 RottenEggCount = 0;

    UPROPERTY(EditDefaultsOnly, Category="Items")
    int32 MaxRottenEggs = 2;

    FTransform RecoveryTransform = FTransform::Identity;
    bool bHasRecoveryTransform = false;
    float PlayerThrottleInput = 0.0f;
    float PlayerBrakeInput = 0.0f;
    float TippedStillTime = 0.0f;
    bool bCrashLatched = false;
    double LastImpactTime = -100.0;
    double DamageEnabledAfterTime = 0.0;

    FString ComicImpactText;
    double ComicImpactStartedAt = -100.0;
    double ComicImpactExpiresAt = -100.0;
    float ComicImpactDuration = 0.70f;
    float CameraKickYaw = 0.0f;
    float CameraKickRoll = 0.0f;
};
