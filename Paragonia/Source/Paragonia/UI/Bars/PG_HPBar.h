// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PG_HPBar.generated.h"

class UImage;
class UHorizontalBox;
class UWidgetAnimation;
/**
 * 
 */
UCLASS()
class PARAGONIA_API UPG_HPBar : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	float HandleHealthChanged(float OldValue, float NewValue);

	UFUNCTION()
	void HandleMaxHealthChanged(float OldValue, float NewValue);

	UFUNCTION(BlueprintCallable)
	void AddBuff(int32 buffUID);

	UFUNCTION(BlueprintCallable)
	void SetPlayerColor();

	UFUNCTION(BlueprintCallable)
	void SetTeamColor(int32 TeamType);

protected:
	UPROPERTY(meta = (BindWidget))    
	TObjectPtr<UImage> BarFill;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BarGlow;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Damaged;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> BuffBox;

	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	TSubclassOf<UUserWidget> BuffIconClass;

	UPROPERTY()
	TMap<int32, TObjectPtr<UUserWidget>> CreatedBuffs;

	UPROPERTY()
	float NowHPValue;
	UPROPERTY()
	float MaxHPValue;
};
