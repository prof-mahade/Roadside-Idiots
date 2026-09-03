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

    // 0 = CLEAN, 1 = BALANCED, 2 = MAYHEM. This changes how frequently the
    // chaos director creates deliberate rival incidents; it never changes the
    // accepted low-level racing-line follower or physical bike handling.
    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Race Setup")
    int32 GetChaosLevel() const { return ChaosLevel; }

    // Player-only analog response curve. Keyboard full-left/full-right remains
    // unchanged and AI bypasses this path entirely.
    // 0 = CALM, 1 = NORMAL, 2 = QUICK.
    UFUNCTION(BlueprintPure, Category="Roadside Idiots|Settings")
    int32 GetSteeringFeel() const { return SteeringFeel; }

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Race Setup")
    void SetOpponentCount(int32 Value) { OpponentCount = FMath::Clamp(Value, MinOpponents, MaxOpponents); }

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Race Setup")
    void SetLapCount(int32 Value) { LapCount = FMath::Clamp(Value, MinLaps, MaxLaps); }

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Race Setup")
    void SetTrafficCount(int32 Value) { TrafficCount = FMath::Clamp(Value, MinTraffic, MaxTraffic); }

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Race Setup")
    void SetChaosLevel(int32 Value) { ChaosLevel = FMath::Clamp(Value, MinChaosLevel, MaxChaosLevel); }

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Settings")
    void SetSteeringFeel(int32 Value) { SteeringFeel = FMath::Clamp(Value, MinSteeringFeel, MaxSteeringFeel); }

    // Shapes an analog steering value without changing the vehicle physics.
    // CALM gives finer center-stick control, NORMAL is linear, QUICK responds
    // earlier. At |input|=1 all three still produce full steering authority.
    float ShapePlayerSteeringInput(float RawValue) const
    {
        const float Clamped = FMath::Clamp(RawValue, -1.0f, 1.0f);
        const float Magnitude = FMath::Abs(Clamped);
        float Exponent = 1.0f;
        if (SteeringFeel <= 0) Exponent = 1.45f;
        else if (SteeringFeel >= 2) Exponent = 0.78f;
        return FMath::Sign(Clamped) * FMath::Pow(Magnitude, Exponent);
    }

    UFUNCTION(BlueprintCallable, Category="Roadside Idiots|Race Setup")
    void ResetToDemoDefaults()
    {
        OpponentCount = 3;
        LapCount = 3;
        TrafficCount = 3;
        ChaosLevel = 1;
        SteeringFeel = 1;
    }

    // A direct pause-menu restart should rebuild the same configured race instead
    // of forcing the player through setup again. GameInstance subsystems survive
    // OpenLevel, so this one-shot flag is the safest place to carry that intent.
    void RequestAutoStartAfterReload() { bAutoStartAfterReload = true; }
    bool ConsumeAutoStartAfterReload()
    {
        const bool bRequested = bAutoStartAfterReload;
        bAutoStartAfterReload = false;
        return bRequested;
    }

    static constexpr int32 MinOpponents = 2;
    static constexpr int32 MaxOpponents = 6;
    static constexpr int32 MinLaps = 1;
    static constexpr int32 MaxLaps = 5;
    static constexpr int32 MinTraffic = 0;
    static constexpr int32 MaxTraffic = 6;
    static constexpr int32 MinChaosLevel = 0;
    static constexpr int32 MaxChaosLevel = 2;
    static constexpr int32 MinSteeringFeel = 0;
    static constexpr int32 MaxSteeringFeel = 2;

private:
    UPROPERTY()
    int32 OpponentCount = 3;

    UPROPERTY()
    int32 LapCount = 3;

    UPROPERTY()
    int32 TrafficCount = 3;

    UPROPERTY()
    int32 ChaosLevel = 1;

    UPROPERTY()
    int32 SteeringFeel = 1;

    UPROPERTY()
    bool bAutoStartAfterReload = false;
};
