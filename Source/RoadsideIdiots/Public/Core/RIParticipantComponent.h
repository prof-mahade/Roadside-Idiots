#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RIParticipantComponent.generated.h"

UCLASS(ClassGroup=(RoadsideIdiots), meta=(BlueprintSpawnableComponent))
class ROADSIDEIDIOTS_API URIParticipantComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URIParticipantComponent();

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Participant")
    void AssignParticipant(FName NewParticipantId, bool bNewHumanControlled);

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Participant")
    FName GetParticipantId() const { return ParticipantId; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Participant")
    bool IsHumanControlled() const { return bHumanControlled; }

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated, VisibleAnywhere, Category="Participant")
    FName ParticipantId = NAME_None;

    UPROPERTY(Replicated, VisibleAnywhere, Category="Participant")
    bool bHumanControlled = false;
};
