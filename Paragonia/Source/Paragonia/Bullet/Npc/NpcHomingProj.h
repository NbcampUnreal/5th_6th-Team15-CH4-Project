// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "NpcHomingProj.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent;

UCLASS()
class PARAGONIA_API ANpcHomingProj : public AActor
{
	GENERATED_BODY()

public:
	ANpcHomingProj();

	void InitializeProjectile(AActor* Target, float Speed);
	void DeactivateProjectile();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_HomingTarget();

	UFUNCTION()
	void CheckTargetStatus();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> MovementComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UParticleSystemComponent> TrailFXComp;

	UPROPERTY(EditDefaultsOnly, Category = "Time|Life")
	float MaxLifeTime;

	UPROPERTY(EditDefaultsOnly, Category = "Time|TargetCheck")
	float TargetCheckTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	FGameplayTag HitEventTag;

	UPROPERTY(ReplicatedUsing = OnRep_HomingTarget)
	AActor* HomingTargetActor;

	FTimerHandle TargetCheckTimerHandle;

	bool bHitConfirmed = false;
};
