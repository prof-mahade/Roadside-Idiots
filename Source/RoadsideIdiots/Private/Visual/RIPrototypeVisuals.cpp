#include "Visual/RIPrototypeVisuals.h"

#include "Vehicle/RIBikePawn.h"
#include "Core/RIHealthComponent.h"
#include "Animation/AnimSequence.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Modules/ModuleManager.h"
#include "TimerManager.h"

namespace
{
    const FName MotorcycleComponentName(TEXT("PrototypeMotorcycleVisual"));
    const FName RiderComponentName(TEXT("PrototypeRiderVisual"));
    const FName BandageArmComponentName(TEXT("PrototypeBandageArm"));
    const FName BandageHeadComponentName(TEXT("PrototypeBandageHead"));
    const FName BandageLegComponentName(TEXT("PrototypeBandageLeg"));
    constexpr float VisualYawOffsetDegrees = -90.0f;
    constexpr float VisualHeightOffset = -10.0f;
    constexpr float RiderRearwardOffset = 14.0f;
    constexpr float RiderDownOffset = 8.0f;

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

    UStaticMeshComponent* FindStaticVisualComponent(ARIBikePawn* Bike, const FName ComponentName)
    {
        if (!Bike) return nullptr;

        TArray<UStaticMeshComponent*> Components;
        Bike->GetComponents<UStaticMeshComponent>(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == ComponentName)
            {
                return Component;
            }
        }
        return nullptr;
    }

    void GetAnimationAssets(TArray<FAssetData>& OutAssets)
    {
        OutAssets.Reset();
        GetRegistry().GetAssetsByPath(FName(TEXT("/Game/MotoInteractionAnims/Animations")), OutAssets, true, false);
    }

    UAnimSequence* FindAnimationByAssetName(const TArray<FAssetData>& Assets, const FString& AssetName)
    {
        for (const FAssetData& Asset : Assets)
        {
            if (Asset.AssetName.ToString().Equals(AssetName, ESearchCase::IgnoreCase))
            {
                return Cast<UAnimSequence>(Asset.GetAsset());
            }
        }
        return nullptr;
    }

    UAnimSequence* FindBestActionAnimation(const FString& FolderMarker, const float Side)
    {
        TArray<FAssetData> Assets;
        GetAnimationAssets(Assets);
        UAnimSequence* BestAnimation = nullptr;
        int32 BestScore = TNumericLimits<int32>::Lowest();

        for (const FAssetData& Asset : Assets)
        {
            const FString PackagePath = Asset.PackagePath.ToString();
            if (!PackagePath.Contains(FolderMarker, ESearchCase::IgnoreCase)) continue;

            const FString Name = Asset.AssetName.ToString();
            if (Name.Contains(TEXT("_Bike"), ESearchCase::IgnoreCase)) continue;

            UAnimSequence* Sequence = Cast<UAnimSequence>(Asset.GetAsset());
            if (!Sequence) continue;

            int32 Score = 0;
            if (Name.Contains(TEXT("Punch"), ESearchCase::IgnoreCase)) Score += 25;
            if (Name.Contains(TEXT("Hit"), ESearchCase::IgnoreCase)) Score += 25;
            if (Side < -0.1f && Name.Contains(TEXT("Left"), ESearchCase::IgnoreCase)) Score += 50;
            if (Side > 0.1f && Name.Contains(TEXT("Right"), ESearchCase::IgnoreCase)) Score += 50;
            if (Side < -0.1f && Name.Contains(TEXT("Right"), ESearchCase::IgnoreCase)) Score -= 30;
            if (Side > 0.1f && Name.Contains(TEXT("Left"), ESearchCase::IgnoreCase)) Score -= 30;
            if (Name.Contains(TEXT("Loop"), ESearchCase::IgnoreCase)) Score -= 20;

            if (Score > BestScore)
            {
                BestScore = Score;
                BestAnimation = Sequence;
            }
        }
        return BestAnimation;
    }

    void ApplyNeutralPose(ARIBikePawn* Bike)
    {
        if (!Bike) return;

        USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
        USkeletalMeshComponent* Motorcycle = FindVisualComponent(Bike, MotorcycleComponentName);
        if (!Rider || !Motorcycle) return;

        Motorcycle->SetAnimation(nullptr);
        Motorcycle->SetPosition(0.0f, false);
        Motorcycle->SetPlayRate(0.0f);

        TArray<FAssetData> Assets;
        GetAnimationAssets(Assets);

        UAnimSequence* RiderPose = FindAnimationByAssetName(Assets, TEXT("AS_Mounted_to_Ride"));
        if (!RiderPose)
        {
            RiderPose = FindAnimationByAssetName(Assets, TEXT("AS_Bike_Start"));
        }

        if (RiderPose)
        {
            Rider->PlayAnimation(RiderPose, false);
            Rider->SetPosition(FMath::Max(0.0f, RiderPose->GetPlayLength() - 0.03f), false);
            Rider->SetPlayRate(0.0f);
        }

        UE_LOG(LogTemp, Display, TEXT("RoadsideIdiots visuals: neutral pose=%s"), RiderPose ? *RiderPose->GetName() : TEXT("NONE"));
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
                    ApplyNeutralPose(ValidBike);
                }
            }),
            Delay,
            false);
    }

    void ConfigureIndependentVisual(USkeletalMeshComponent* Component)
    {
        if (!Component) return;
        Component->SetAbsolute(true, true, true);
        Component->SetWorldScale3D(FVector::OneVector);
    }

    UStaticMeshComponent* EnsureBandage(
        ARIBikePawn* Bike,
        USkeletalMeshComponent* Rider,
        UStaticMesh* CubeMesh,
        const FName ComponentName,
        const FName BoneName,
        const FVector RelativeLocation,
        const FRotator RelativeRotation,
        const FVector RelativeScale)
    {
        if (!Bike || !Rider || !CubeMesh) return nullptr;

        UStaticMeshComponent* Bandage = FindStaticVisualComponent(Bike, ComponentName);
        if (!Bandage)
        {
            Bandage = NewObject<UStaticMeshComponent>(Bike, ComponentName);
            Bike->AddInstanceComponent(Bandage);
            Bandage->SetupAttachment(Rider, BoneName);
            Bandage->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Bandage->SetGenerateOverlapEvents(false);
            Bandage->RegisterComponent();
        }

        Bandage->SetStaticMesh(CubeMesh);
        Bandage->SetRelativeLocation(RelativeLocation);
        Bandage->SetRelativeRotation(RelativeRotation);
        Bandage->SetRelativeScale3D(RelativeScale);
        Bandage->SetVisibility(false, true);

        if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
        {
            if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Bandage))
            {
                Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.93f, 0.86f, 0.70f, 1.0f));
                Bandage->SetMaterial(0, Material);
            }
        }

        return Bandage;
    }

    void UpdateBandages(ARIBikePawn* Bike)
    {
        if (!Bike) return;

        URIHealthComponent* Health = Bike->GetHealthComponent();
        if (!Health) return;

        const float MaxHealth = FMath::Max(1.0f, Health->GetMaxHealth());
        const float HealthFraction = Health->GetCurrentHealth() / MaxHealth;

        if (UStaticMeshComponent* ArmBandage = FindStaticVisualComponent(Bike, BandageArmComponentName))
        {
            ArmBandage->SetVisibility(HealthFraction <= 0.75f, true);
        }
        if (UStaticMeshComponent* HeadBandage = FindStaticVisualComponent(Bike, BandageHeadComponentName))
        {
            HeadBandage->SetVisibility(HealthFraction <= 0.50f, true);
        }
        if (UStaticMeshComponent* LegBandage = FindStaticVisualComponent(Bike, BandageLegComponentName))
        {
            LegBandage->SetVisibility(HealthFraction <= 0.25f, true);
        }
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
        Motorcycle->RegisterComponent();
    }
    Motorcycle->SetSkeletalMesh(MotorcycleMesh, true);
    ConfigureIndependentVisual(Motorcycle);

    USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
    if (!Rider)
    {
        Rider = NewObject<USkeletalMeshComponent>(Bike, RiderComponentName);
        Bike->AddInstanceComponent(Rider);
        Rider->SetupAttachment(Bike->GetRootComponent());
        Rider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Rider->SetGenerateOverlapEvents(false);
        Rider->RegisterComponent();
    }
    Rider->SetSkeletalMesh(MannyMesh, true);
    ConfigureIndependentVisual(Rider);

    TArray<UStaticMeshComponent*> GrayboxParts;
    Bike->GetComponents<UStaticMeshComponent>(GrayboxParts);
    for (UStaticMeshComponent* Part : GrayboxParts)
    {
        if (Part) Part->SetVisibility(false, false);
    }

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    EnsureBandage(
        Bike,
        Rider,
        CubeMesh,
        BandageArmComponentName,
        FName(TEXT("upperarm_l")),
        FVector(12.0f, 0.0f, 0.0f),
        FRotator(0.0f, 0.0f, 8.0f),
        FVector(0.16f, 0.105f, 0.105f));
    EnsureBandage(
        Bike,
        Rider,
        CubeMesh,
        BandageHeadComponentName,
        FName(TEXT("head")),
        FVector(0.0f, 0.0f, 8.0f),
        FRotator(5.0f, 18.0f, 0.0f),
        FVector(0.19f, 0.27f, 0.045f));
    EnsureBandage(
        Bike,
        Rider,
        CubeMesh,
        BandageLegComponentName,
        FName(TEXT("calf_r")),
        FVector(13.0f, 0.0f, 0.0f),
        FRotator(0.0f, 0.0f, -6.0f),
        FVector(0.17f, 0.10f, 0.10f));

    ApplyNeutralPose(Bike);
    Update(Bike);
}

void RIPrototypeVisuals::Update(ARIBikePawn* Bike)
{
    if (!Bike) return;

    USkeletalMeshComponent* Motorcycle = FindVisualComponent(Bike, MotorcycleComponentName);
    USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
    if (!Motorcycle || !Rider) return;

    const FRotator ActorRotation = Bike->GetActorRotation();
    const bool bClearlyCrashed = Bike->GetActorUpVector().Z < 0.55f;
    const FRotator VisualRotation = bClearlyCrashed
        ? FRotator(ActorRotation.Pitch, ActorRotation.Yaw + VisualYawOffsetDegrees, ActorRotation.Roll)
        : FRotator(0.0f, ActorRotation.Yaw + VisualYawOffsetDegrees, 0.0f);

    const FVector VisualLocation = Bike->GetActorLocation() + FVector(0.0f, 0.0f, VisualHeightOffset);
    const FVector BikeForward = Bike->GetActorForwardVector().GetSafeNormal2D();
    const FVector RiderLocation = VisualLocation - BikeForward * RiderRearwardOffset - FVector::UpVector * RiderDownOffset;

    Motorcycle->SetWorldLocationAndRotation(VisualLocation, VisualRotation);
    Rider->SetWorldLocationAndRotation(RiderLocation, VisualRotation);
    Motorcycle->SetWorldScale3D(FVector::OneVector);
    Rider->SetWorldScale3D(FVector::OneVector);

    UpdateBandages(Bike);
}

void RIPrototypeVisuals::PlaySideAction(ARIBikePawn* Bike, float Side)
{
    USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
    if (!Rider) return;

    if (UAnimSequence* Sequence = FindBestActionAnimation(TEXT("Punch"), Side))
    {
        Rider->SetPlayRate(1.0f);
        Rider->PlayAnimation(Sequence, false);
        ResumeLater(Bike, Sequence);
    }
}

void RIPrototypeVisuals::PlayReaction(ARIBikePawn* Bike, float Side)
{
    USkeletalMeshComponent* Rider = FindVisualComponent(Bike, RiderComponentName);
    if (!Rider) return;

    if (UAnimSequence* Sequence = FindBestActionAnimation(TEXT("Get_Hits"), Side))
    {
        Rider->SetPlayRate(1.0f);
        Rider->PlayAnimation(Sequence, false);
        ResumeLater(Bike, Sequence);
    }
}
