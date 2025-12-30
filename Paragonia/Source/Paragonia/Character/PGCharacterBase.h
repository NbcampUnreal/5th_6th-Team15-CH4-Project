// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "Interface/PGTeamStatusInterface.h"
#include "PGCharacterBase.generated.h"

class UPaperSprite;
class UTextureRenderTarget2D;
class UAbilitySystemComponent;
class UPG_PlayerUIComponent;
class UWidgetComponent;
class UPaperSpriteComponent;

UCLASS()
class PARAGONIA_API APGCharacterBase : public ACharacter, public IAbilitySystemInterface, public IPGTeamStatusInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APGCharacterBase();

public:

	UFUNCTION(BlueprintCallable)
	UCharacterAttributeSet* GetCharacterAttributeSet() const { return CharacterAttributeSet; }

	UFUNCTION(BlueprintCallable)
	void SetMinimapSprite(UPaperSprite* NewSprite);

	void SetIngameInfo(bool TeamType, const FString& InNickName);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual int32 GetTeamID_Implementation() const override;
	virtual bool GetIsDead_Implementation() const override;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute")
	TObjectPtr<UCharacterAttributeSet> CharacterAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UPG_PlayerUIComponent> UIComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HeadHPWidgetComp;

	UPROPERTY()
	TObjectPtr<UPaperSpriteComponent> MinimapIcon;

protected:

	int32 TeamId = 0;
};
