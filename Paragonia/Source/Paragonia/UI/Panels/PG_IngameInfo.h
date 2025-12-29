// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PG_IngameInfo.generated.h"

class UPG_HPBar;
class UTextBlock;
class UCharacterAttributeSet;
/**
 * 
 */
UCLASS()
class PARAGONIA_API UPG_IngameInfo : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetHPBarColor(int32 TeamType);

	void BindToAttributeSet(UCharacterAttributeSet* InAttrSet);

	UFUNCTION()
	void HandleHealthChanged(float OldValue, float NewValue);

	UFUNCTION()
	void HandleMaxHealthChanged(float OldValue, float NewValue);

protected:
	virtual void NativeDestruct() override;

private:

	void UnbindFromAttributeSet();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_HPBar> PlayerHPBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerName;

private:
	UPROPERTY()
	TObjectPtr<UCharacterAttributeSet> BoundAttrSet;

};
