#include "Core/RIHealthComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

URIHealthComponent::URIHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void URIHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        CurrentHealth = MaxHealth;
        LastAppliedImpactTime = -100.0;
        LastImpactSource = NAME_None;
        LastImpactAmount = 0.0f;
        ImpactSerial = 0;
    }
}

float URIHealthComponent::ApplyImpact(float Amount)
{
    return ApplyImpactFromSource(Amount, FName(TEXT("Unknown")));
}

float URIHealthComponent::ApplyImpactFromSource(float Amount, FName SourceTag)
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || Amount <= 0.0f)
    {
        return CurrentHealth;
    }

    if (const UWorld* World = GetWorld())
    {
        const double Now = World->GetTimeSeconds();
        if (Now - LastAppliedImpactTime < ImpactImmunitySeconds)
        {
            return CurrentHealth;
        }
        LastAppliedImpactTime = Now;
    }

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, MaxHealth);
    const float AppliedAmount = FMath::Max(0.0f, PreviousHealth - CurrentHealth);

    if (AppliedAmount > KINDA_SMALL_NUMBER)
    {
        LastImpactSource = SourceTag.IsNone() ? FName(TEXT("Unknown")) : SourceTag;
        LastImpactAmount = AppliedAmount;
        ++ImpactSerial;
    }

    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    return CurrentHealth;
}

float URIHealthComponent::Heal(float Amount)
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || Amount <= 0.0f)
    {
        return CurrentHealth;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    return CurrentHealth;
}

void URIHealthComponent::ResetHealth()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    CurrentHealth = MaxHealth;
    LastAppliedImpactTime = -100.0;
    LastImpactSource = NAME_None;
    LastImpactAmount = 0.0f;
    ImpactSerial = 0;
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void URIHealthComponent::OnRep_CurrentHealth()
{
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void URIHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(URIHealthComponent, CurrentHealth);
}
