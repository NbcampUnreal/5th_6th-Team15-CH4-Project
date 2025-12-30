// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PG_InGameTeamSimpleInfo.generated.h"

class UPG_HPBar;
class UPG_PlayerIcon;
/**
 * 
 */
UCLASS()
class PARAGONIA_API UPG_InGameTeamSimpleInfo : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void InitTeamSimpleInfo(int32 CharacterID);
	void InitTeamSimpleInfo(int32 CharacterID, const FString& PlayerName);

	UPG_HPBar* GetTeamHPBar() const { return HPBar; }

	UFUNCTION()
	void HandleHealthChanged(float OldValue, float NewValue);

	UFUNCTION()
	void HandleMaxHealthChanged(float OldValue, float NewValue);

	UFUNCTION(BlueprintCallable)
	void AddBuff(int32 buffUID);

	UFUNCTION(BlueprintCallable)
	void SetTeamColor();
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_PlayerIcon> PlayerIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_HPBar> HPBar;
};
