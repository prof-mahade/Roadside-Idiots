#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RIHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRIHealthChangedSignature, float, CurrentHealth, float, MaxHealth);

UCLASS(ClassGroup=(RoadsideIdiots), meta=(BlueprintSpawnableComponent))
class ROADSIDEIDIOTS_API URIHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URIHealthComponent();

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Health")
    float ApplyImpact(float Amount);

    // C++ gameplay callers can attach a lightweight source tag for passive
    // playtest telemetry. This uses the exact same immunity and health math as
    // ApplyImpact; the tag never changes gameplay behavior.
    float ApplyImpactFromSource(float Amount, FName SourceTag);

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Health")
    float Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Health")
    void ResetHealth();

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Health")
    float GetMaxHealth() const { return MaxHealth; }

    // Read-only telemetry metadata for the most recent accepted condition loss.
    FName GetLastImpactSource() const { return LastImpactSource; }
    float GetLastImpactAmount() const { return LastImpactAmount; }
    uint32 GetImpactSerial() const { return ImpactSerial; }

    UPROPERTY(BlueprintAssignable, Category="Roadside Idiots|Health")
    FRIHealthChangedSignature OnHealthChanged;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditDefaultsOnly, Category="Health", meta=(ClampMin="1.0"))
    float MaxHealth = 100.0f;

    // Prevent one comic hit from being counted again immediately as a physics collision.
    UPROPERTY(EditDefaultsOnly, Category="Health", meta=(ClampMin="0.0"))
    float ImpactImmunitySeconds = 0.65f;

    UPROPERTY(ReplicatedUsing=OnRep_CurrentHealth, VisibleAnywhere, Category="Health")
    float CurrentHealth = 100.0f;

    double LastAppliedImpactTime = -100.0;

    // Not replicated: these exist only so local/server-side playtest telemetry can
    // attribute condition loss without guessing from transient presentation text.
    FName LastImpactSource = NAME_None;
    float LastImpactAmount = 0.0f;
    uint32 ImpactSerial = 0;

    UFUNCTION()
    void OnRep_CurrentHealth();
};
