#include "AI/Task/STT_LtBelicaDroneAttack.h"

#include "Character/PGPlayerCharacterBase.h"
#include "Bullet/PGLtBelicaDroneBullet.h"

#include "Kismet/KismetMathLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "StateTreeLinker.h"
#include "StateTreeExecutionContext.h"

bool FSTT_LtBelicaDroneAttack::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(DroneHandle);
	return true;
}

EStateTreeRunStatus FSTT_LtBelicaDroneAttack::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_LtBelicaDroneAttack::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	ALtBelicaDrone* NPC = &Context.GetExternalData(DroneHandle);
	if (!IsValid(NPC))
	{
		return EStateTreeRunStatus::Failed;
	}

	AActor* TargetActor = NPC->GetAttackTarget();
	if (!IsValid(TargetActor))
	{
		return EStateTreeRunStatus::Failed;
	}

	FVector NPCLoc = NPC->GetActorLocation();
	FVector TargetLoc = TargetActor->GetActorLocation();
	FRotator Rot = UKismetMathLibrary::FindLookAtRotation(NPCLoc, TargetLoc);
	NPC->SetActorRotation(FQuat(FRotator(0.0f, Rot.Yaw, 0.0f)));

	FActorSpawnParameters Param;
	Param.Owner = NPC;
	Param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* Bullet = NPC->GetWorld()->SpawnActor<APGLtBelicaDroneBullet>(APGLtBelicaDroneBullet::StaticClass(), TargetLoc, FRotator::ZeroRotator, Param);
	if (!IsValid(Bullet))
	{
		UE_LOG(LogTemp, Warning, TEXT("FSTT_LtBelicaDroneAttack::Tick - Bullet Spawn Fail"));
		return EStateTreeRunStatus::Failed;
	}

	APGLtBelicaDroneBullet* DroneBullet = Cast<APGLtBelicaDroneBullet>(Bullet);
	if (IsValid(DroneBullet))
	{
		DroneBullet->InitBullet(NPC->GetAttackData());
		DroneBullet->ProcessHitCheck();
	}

	NPC->CreateBulletEffect();
	NPC->SetAttackTarget(nullptr);

	return EStateTreeRunStatus::Succeeded;
}
