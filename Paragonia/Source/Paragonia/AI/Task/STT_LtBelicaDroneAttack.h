#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "Character/AI/LtBelicaDrone.h"
#include "GameplayTagContainer.h"
#include "STT_LtBelicaDroneAttack.generated.h"

USTRUCT()
struct FSTT_LtBelicaDroneAttackInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Categories = "Ability"))
	FGameplayTag HitCheckTag;
};

USTRUCT(meta = (DisplayName = "Drone Attack", Category = "Task|AI"))
struct PARAGONIA_API FSTT_LtBelicaDroneAttack : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_LtBelicaDroneAttackInstanceData;

	FSTT_LtBelicaDroneAttack() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	TStateTreeExternalDataHandle<ALtBelicaDrone> DroneHandle;
};
