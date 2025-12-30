// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PG_PlayerUIComponent.generated.h"

class APGCharacterBase;
class APlayerCameraManager;
class USceneCaptureComponent2D;
class UWidgetComponent;
class UPG_IngameInfo;
class UTextureRenderTarget2D;
class UPaperSpriteComponent;
class UCharacterAttributeSet;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PARAGONIA_API UPG_PlayerUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPG_PlayerUIComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitComponents(APGCharacterBase* InOwnerCharacter, UWidgetComponent* InHeadHPWidgetComp, UPaperSpriteComponent* InMinimapIcon, UCharacterAttributeSet* InCharacterAttributeSet);

	void SetupHeadHPWidget();

	void BindHeadHPDelegatesOnce();

	void UpdateHeadHPVisibility();

	void SetHPBarColor(bool TeamType);
	void SetPlayerNickName(const FString& InNickName, bool CheckTeam = true);

	UTextureRenderTarget2D* GetMinimapRenderTarget();
protected:

	UPROPERTY()
	TObjectPtr<APGCharacterBase> OwnerCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HeadHPWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPG_IngameInfo> HeadHPWidgetClass;

	UPROPERTY()
	TObjectPtr<UPG_IngameInfo> HeadHPWidget;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> MinimapRT;

	UPROPERTY()
	TObjectPtr<UPaperSpriteComponent> MinimapIcon;

	UPROPERTY()
	TObjectPtr<UCharacterAttributeSet> CharacterAttributeSet;

	UPROPERTY()
	TObjectPtr<APlayerCameraManager> CameraManager;
private:

	float Accum;
	bool bHeadHPBound;
};
