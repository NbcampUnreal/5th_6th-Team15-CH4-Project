#pragma once

#include "CoreMinimal.h"
#include "Character/AI/NpcBaseCharacter.h"
#include "Struct/FAttackData.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "LtBelicaDrone.generated.h"

UCLASS()
class PARAGONIA_API ALtBelicaDrone : public ANpcBaseCharacter
{
	GENERATED_BODY()
	
public:	
	ALtBelicaDrone();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void CreateBulletEffect();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDurationOver();

	virtual void PossessedBy(AController* NewController) override;

	void InitAttackData(const FAttackData& InData);
	FAttackData GetAttackData() const;

protected:
	FAttackData AttackData;
};
