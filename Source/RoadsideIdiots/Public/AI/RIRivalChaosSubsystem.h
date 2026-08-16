#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RIRivalChaosSubsystem.generated.h"

class ARIAIController;
class ARIBikePawn;
enum class ERITacticalIntent : uint8;

UCLASS()
class ROADSIDEIDIOTS_API URIRivalChaosSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    void IssueDirectives();
    ARIBikePawn* FindTargetFor(ARIAIController* Controller, ARIBikePawn* ControlledBike, int32 BotIndex, const TSet<const ARIBikePawn*>& ReservedTargets) const;
    float GetDirectiveInterval(int32 BotIndex) const;
    float GetDirectiveChance(int32 BotIndex) const;
    ERITacticalIntent ChooseIntent(const ARIBikePawn* ControlledBike, int32 BotIndex) const;

    float DecisionRemaining = 2.0f;
    float ActiveRaceSeconds = 0.0f;
    TMap<TWeakObjectPtr<ARIAIController>, double> LastDirectiveTime;
};
