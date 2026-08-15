#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RIRaceSettingsSubsystem.generated.h"

UCLASS()
class ROADSIDEIDIOTS_API URIRaceSettingsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Race Setup")
    int32 GetOpponentCount() const { return OpponentCount; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Race Setup")
    int32 GetLapCount() const { return LapCount; }

    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Race Setup")
    int32 GetTrafficCount() const { return TrafficCount; }

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Race Setup")
    void SetOpponentCount(int32 Value) { OpponentCount = FMath::Clamp(Value, 2, 6); }

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Race Setup")
    void SetLapCount(int32 Value) { LapCount = FMath::Clamp(Value, 1, 5); }

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Race Setup")
    void SetTrafficCount(int32 Value) { TrafficCount = FMath::Clamp(Value, 0, 6); }

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Race Setup")
    void ResetToDemoDefaults()
    {
        OpponentCount = 3;
        LapCount = 3;
        TrafficCount = 3;
    }

    static constexpr int32 MinOpponents = 2;
    static constexpr int32 MaxOpponents = 6;
    static constexpr int32 MinLaps = 1;
    static constexpr int32 MaxLaps = 5;
    static constexpr int32 MinTraffic = 0;
    static constexpr int32 MaxTraffic = 6;

private:
    UPROPERTY()
    int32 OpponentCount = 3;

    UPROPERTY()
    int32 LapCount = 3;

    UPROPERTY()
    int32 TrafficCount = 3;
};
