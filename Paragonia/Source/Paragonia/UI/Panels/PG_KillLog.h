// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PG_KillLog.generated.h"

class UPG_PlayerIcon;
class UImage;
class UTextBlock; 
class APGPlayerState;
/**
 * 
 */
UCLASS()
class PARAGONIA_API UPG_KillLog : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void ResetSlot();

	void InitIfNeeded(APGPlayerState* InPlayerState, APGPlayerState* DeathPlayerState);

	void ShowKillLog(APGPlayerState* KillerPlayerState);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr <UPG_PlayerIcon> VictimIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr <UPG_PlayerIcon> KillerIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> TeamImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LogLabel;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<class UWidgetAnimation> PulseAnim;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<class UWidgetAnimation> SetInit;

private:
	bool bInited = false;
	int32 BoundPlayerId = INDEX_NONE;
};
