#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGKillZone.generated.h"

class UBoxComponent;
class UGameplayEffect;

UCLASS()
class PARAGONIA_API APGKillZone : public AActor
{
	GENERATED_BODY()

public:
	APGKillZone();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Volume")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float KillDamage = 99999.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, Category = "Data")
	TObjectPtr<UDataTable> KillZoneDataTable;

	UPROPERTY(EditAnywhere, Category = "Data")
	FName RowName;

};