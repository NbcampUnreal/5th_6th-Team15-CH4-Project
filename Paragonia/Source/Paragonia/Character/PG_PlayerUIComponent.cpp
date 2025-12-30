// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PG_PlayerUIComponent.h"

#include "Character/PGCharacterBase.h"
#include "AttributeSet/CharacterAttributeSet.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "Components/WidgetComponent.h"
#include "UI/Panels/PG_IngameInfo.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerState/PGPlayerState.h"

UPG_PlayerUIComponent::UPG_PlayerUIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
	bHeadHPBound = false;
	Accum = 0.f;
}

void UPG_PlayerUIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter)
	{
		return;
	}

	if (!HeadHPWidgetComp)
	{
		return;
	}

	if (!IsNetMode(NM_Standalone) && !GetWorld())
	{
		return;
	}

	if (!IsValid(CameraManager))
	{
		CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
		return;
	}

	const FVector CamLoc = CameraManager->GetCameraLocation();
	const FVector WidgetLoc = HeadHPWidgetComp->GetComponentLocation();
	const FRotator LookRot = (CamLoc - WidgetLoc).Rotation();

	HeadHPWidgetComp->SetWorldRotation(FRotator(0.f, LookRot.Yaw, 0.f));

}

void UPG_PlayerUIComponent::InitComponents(APGCharacterBase* InOwnerCharacter, UWidgetComponent* InHeadHPWidgetComp, UPaperSpriteComponent* InMinimapIcon, UCharacterAttributeSet* InCharacterAttributeSet)
{
	OwnerCharacter = InOwnerCharacter;
	HeadHPWidgetComp = InHeadHPWidgetComp;
	MinimapIcon = InMinimapIcon;
	CharacterAttributeSet = InCharacterAttributeSet;
}

void UPG_PlayerUIComponent::SetupHeadHPWidget()
{
	if (!IsValid(HeadHPWidgetComp))
	{
		return;
	}

	if (!HeadHPWidgetClass)
	{
		return;
	}

	if (HeadHPWidgetComp->GetWidgetClass() != HeadHPWidgetClass)
	{
		HeadHPWidgetComp->SetWidgetClass(HeadHPWidgetClass);
	}

	HeadHPWidgetComp->InitWidget();

	HeadHPWidget = Cast<UPG_IngameInfo>(HeadHPWidgetComp->GetUserWidgetObject());

	if (!IsValid(HeadHPWidget))
	{
		return;
	}

	BindHeadHPDelegatesOnce();
}

void UPG_PlayerUIComponent::BindHeadHPDelegatesOnce()
{
	if (bHeadHPBound)
	{
		return;
	}

	if (!IsValid(CharacterAttributeSet) || !IsValid(HeadHPWidget))
	{
		return;
	}

	if (!OwnerCharacter)
	{
		return;
	}

	HeadHPWidget->BindToAttributeSet(CharacterAttributeSet);

	bHeadHPBound = true;
}

void UPG_PlayerUIComponent::UpdateHeadHPVisibility()
{
	if (!IsValid(HeadHPWidgetComp) || !IsValid(OwnerCharacter))
	{
		return;
	}

	HeadHPWidgetComp->SetHiddenInGame(true);
	HeadHPWidgetComp->SetVisibility(false);
}


UTextureRenderTarget2D* UPG_PlayerUIComponent::GetMinimapRenderTarget()
{
	if (!MinimapRT)
	{
		MinimapRT = NewObject<UTextureRenderTarget2D>(this);
		MinimapRT->InitAutoFormat(512, 512);
		MinimapRT->ClearColor = FLinearColor::Black;
		MinimapRT->UpdateResourceImmediate(true);
	}

	return MinimapRT;
}

void UPG_PlayerUIComponent::SetHPBarColor(bool TeamType)
{
	if (!IsValid(HeadHPWidget))
	{
		return;
	}

	HeadHPWidget->SetHPBarColor(TeamType);
}

void UPG_PlayerUIComponent::SetPlayerNickName(const FString& InNickName, bool CheckTeam)
{
	if (!IsValid(HeadHPWidget))
	{
		return;
	}

	HeadHPWidget->SetNickName(InNickName, CheckTeam);
}
