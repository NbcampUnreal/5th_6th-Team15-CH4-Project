#include "GA/LtBelica/GA_SkillE_LtBelica.h"

UGA_SkillE_LtBelica::UGA_SkillE_LtBelica()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	TaskName = FName(TEXT("SkillETask"));
	TargetingStartMontageSectionName = FName(TEXT("TargetingStart"));
	TargetingEndtMontageSectionName = FName(TEXT("TargetingEnd"));

	bUseHitResult = false;
}
