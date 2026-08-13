#include "Interaction/RIInteractionComponent.h"
#include "Vehicle/RIBikePawn.h"
#include "Core/RIHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

URIInteractionComponent::URIInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool URIInteractionComponent::TrySideInteraction(float Side)
{
    ARIBikePawn* OwnerBike = Cast<ARIBikePawn>(GetOwner());
    if (!OwnerBike || !GetWorld())
    {
        return false;
    }

    const double Now = GetWorld()->GetTimeSeconds();
    if (Now - LastUseTime < CooldownSeconds)
    {
        return false;
    }
    LastUseTime = Now;

    Side = Side < 0.0f ? -1.0f : 1.0f;
    const FVector SideDirection = OwnerBike->GetActorRightVector() * Side;
    const FVector Start = OwnerBike->GetActorLocation() + OwnerBike->GetActorForwardVector() * 35.0f;
    const FVector End = Start + SideDirection * Reach;

    FCollisionObjectQueryParams ObjectQuery;
    ObjectQuery.AddObjectTypesToQuery(ECC_PhysicsBody);
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RISideInteraction), false, OwnerBike);
    TArray<FHitResult> Hits;

    const bool bHitAnything = GetWorld()->SweepMultiByObjectType(
        Hits,
        Start,
        End,
        FQuat::Identity,
        ObjectQuery,
        FCollisionShape::MakeSphere(Radius),
        QueryParams);

    if (!bHitAnything)
    {
        return false;
    }

    for (const FHitResult& Hit : Hits)
    {
        ARIBikePawn* OtherBike = Cast<ARIBikePawn>(Hit.GetActor());
        if (!OtherBike || OtherBike == OwnerBike)
        {
            continue;
        }

        if (UStaticMeshComponent* OtherChassis = OtherBike->GetChassis())
        {
            OtherChassis->AddImpulse(SideDirection * SideVelocityChange + FVector::UpVector * 45.0f, NAME_None, true);
        }

        if (URIHealthComponent* OtherHealth = OtherBike->GetHealthComponent())
        {
            OtherHealth->ApplyImpact(ImpactCost);
        }
        return true;
    }

    return false;
}
