#include "Visual/RIPrototypeVisuals.h"

#include "Vehicle/RIBikePawn.h"
#include "Animation/AnimSequence.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Modules/ModuleManager.h"
#include "TimerManager.h"

namespace
{
    const FName MotorcycleComponentName(TEXT("PrototypeMotorcycleVisual"));
    const FName RiderComponentName(TEXT("PrototypeRiderVisual"));

    IAssetRegistry& GetRegistry()
    {
        FAssetRegistryModule& Module = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
        return Module.Get();
    }

    template <typename TAsset>
    TAsset* FindAssetByName(const FName RootPath, const FName AssetName)
    {
        TArray<FAssetData> Assets;
        GetRegistry().GetAssetsByPath(RootPath, Assets, true, false);
        for (const FAssetData& Asset : Assets)
        {
            if (Asset.AssetName == AssetName)
            {
                return Cast<TAsset>(Asset.GetAsset());
            }
        }
        return nullptr;
    }

    USkeletalMeshComponent* FindVisualComponent(ARIBikePawn* Bike, const FName ComponentName)
    {
        if (!Bike) return nullptr;
        TArray<USkeletalMeshComponent*> Components;
        Bike->GetComponents<USkeletalMeshComponent>(Components);
        for (USkeletalMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == ComponentName)
            {
                return Component;
            }
        }
        return nullptr;
    }

    UAnimSequence* FindBestAnimation(const FString& FolderMarker, const float Side, const bool bWantBikeAnimation)
    {
        TArray<FAssetData> Assets;
        GetRegistry().GetAssetsByPath(FName(TEXT("/Game/MotoInteractionAnims/Animations")), Assets, true, false);

        UAnimSequence* BestAnimation = nullptr;
        int32 BestScore = TNumericLimits<int32>::Lowest();
        for (const FAssetData& Asset : Assets)
        {
            const FString PackagePath = Asset.PackagePath.ToString();
            if (!PackagePath.Contains(FolderMarker, ESearchCase::IgnoreCase)) continue;

            const FString Name = Asset.AssetName.ToString();
            const bool bIsBikeAnimation = Name.Contains(TEXT("_Bike"), ESearchCase::IgnoreCase);
            if (bIsBikeAnimation != bWantBikeAnimation) continue;

            UAnimSequence* Sequence = Cast<UAnimSequence>(Asset.GetAsset());
            if (!Sequence) continue;

            int32 Score = 0;
            if (Name.Contains(TEXT("Loop"), ESearchCase::IgnoreCase)) Score += 12;
            if (Name.Contains(TEXT("Ride"), ESearchCase::IgnoreCase)) Score += 10;
            if (Name.Contains(TEXT("Punch"), ESearchCase::IgnoreCase)) Score += 10;
            if (Name.Contains(TEXT("Hit"), ESearchCase::IgnoreCase)) Score += 10;
            if (Side < -0.1f && Name.Contains(TEXT("Left"), ESearchCase::IgnoreCase)) Score += 25;
            if (Side > 0.1f && Name.Contains(TEXT("Right"), ESearchCase::IgnoreCase)) Score += 25;

            if (Score > BestScore)
            {
                BestScore = Score;
                BestAnimation = Sequence;
            }
        }
        return BestAnimation;
    }

    void ResumeRiderLoop(ARIBikePawn* Bike)
    {
        USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
        if (!Rider) return;
        if (UAnimSequence* Ride = FindBestAnimation(TEXT("Riding"), 0.0f, false))
        {
            Rider->PlayAnimation(Ride, true);
        }
    }

    void ResumeLater(ARIBikePawn* Bike, UAnimSequence* Sequence)
    {
        if (!Bike || !Bike->GetWorld()) return;

        const float Delay = Sequence ? FMath::Clamp(Sequence->GetPlayLength(), 0.35f, 1.25f) : 0.65f;
        const TWeakObjectPtr<ARIBikePawn> WeakBike(Bike);
        FTimerHandle TimerHandle;
        Bike->GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            FTimerDelegate::CreateLambda([WeakBike]()
            {
                if (ARIBikePawn* ValidBike = WeakBike.Get())
                {
                    ResumeRiderLoop(ValidBike);
                }
            }),
            Delay,
            false);
    }
}

void RIPrototypeVisuals::Setup(ARIBikePawn* Bike)
{
    if (!Bike || !Bike->GetRootComponent()) return;

    USkeletalMesh* MotorcycleMesh = FindAssetByName<USkeletalMesh>(FName(TEXT("/Game/MotoInteractionAnims")), FName(TEXT("SM_Bike")));
    USkeletalMesh* MannyMesh = FindAssetByName<USkeletalMesh>(FName(TEXT("/Game/Characters")), FName(TEXT("SKM_Manny_Simple")));
    if (!MotorcycleMesh || !MannyMesh) return;

    USkeletalMeshComponent* Motorcycle = FindVisualComponent(Bike, MotorcycleComponentName);
    if (!Motorcycle)
    {
        Motorcycle = NewObject<USkeletalMeshComponent>(Bike, MotorcycleComponentName);
        Bike->AddInstanceComponent(Motorcycle);
        Motorcycle->SetupAttachment(Bike->GetRootComponent());
        Motorcycle->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Motorcycle->SetGenerateOverlapEvents(false);
        Motorcycle->SetRelativeLocation(FVector(0.0f, 0.0f, -28.0f));
        Motorcycle->RegisterComponent();
    }
    Motorcycle->SetSkeletalMesh(MotorcycleMesh, true);

    USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
    if (!Rider)
    {
        Rider = NewObject<USkeletalMeshComponent>(Bike, RiderComponentName);
        Bike->AddInstanceComponent(Rider);
        Rider->SetupAttachment(Bike->GetRootComponent());
        Rider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Rider->SetGenerateOverlapEvents(false);
        Rider->SetRelativeLocation(FVector(0.0f, 0.0f, -28.0f));
        Rider->RegisterComponent();
    }
    Rider->SetSkeletalMesh(MannyMesh, true);

    TArray<UStaticMeshComponent*> GrayboxParts;
    Bike->GetComponents<UStaticMeshComponent>(GrayboxParts);
    for (UStaticMeshComponent* Part : GrayboxParts)
    {
        // Hide only the primitive placeholder itself. Do not propagate visibility
        // to children because the real motorcycle/rider are attached to the
        // invisible physics chassis.
        if (Part) Part->SetVisibility(false, false);
    }

    if (UAnimSequence* BikeRide = FindBestAnimation(TEXT("Riding"), 0.0f, true))
    {
        Motorcycle->PlayAnimation(BikeRide, true);
    }
    ResumeRiderLoop(Bike);
}

void RIPrototypeVisuals::PlaySideAction(ARIBikePawn* Bike, float Side)
{
    USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
    if (!Rider) return;
    if (UAnimSequence* Sequence = FindBestAnimation(TEXT("Punch"), Side, false))
    {
        Rider->PlayAnimation(Sequence, false);
        ResumeLater(Bike, Sequence);
    }
}

void RIPrototypeVisuals::PlayReaction(ARIBikePawn* Bike, float Side)
{
    USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
    if (!Rider) return;
    if (UAnimSequence* Sequence = FindBestAnimation(TEXT("Get_Hits"), Side, false))
    {
        Rider->PlayAnimation(Sequence, false);
        ResumeLater(Bike, Sequence);
    }
}
