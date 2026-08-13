#include "Core/RIParticipantComponent.h"
#include "Net/UnrealNetwork.h"

URIParticipantComponent::URIParticipantComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void URIParticipantComponent::AssignParticipant(FName NewParticipantId, bool bNewHumanControlled)
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        ParticipantId = NewParticipantId;
        bHumanControlled = bNewHumanControlled;
    }
}

void URIParticipantComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(URIParticipantComponent, ParticipantId);
    DOREPLIFETIME(URIParticipantComponent, bHumanControlled);
}
