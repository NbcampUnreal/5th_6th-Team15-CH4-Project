// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Icons/PG_PlayerIcon.h"
#include "Struct/FCharacterResourceInfo.h"
#include "Subsystem/PGCharacterDescriptionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UPG_PlayerIcon::SetPlayerNumberId(int32 InPlayerNumberID) 
{ 
	PlayerNumberID = InPlayerNumberID; 
}

void UPG_PlayerIcon::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UGameInstance* GI = GetGameInstance())
	{
		CharacterDescSubsys = GI->GetSubsystem<UPGCharacterDescriptionSubsystem>();
	}

	if (!CharacterDescSubsys)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPG_PlayerIcon: CharacterDescSubsys is null."));
	}
}

void UPG_PlayerIcon::ApplyIconTexture(UTexture2D* Tex)
{
	if (!IconImage || !Tex) return;

	IconImage->SetBrushFromTexture(Tex, true);
	IconImage->SetOpacity(0.5f);
}

int32 UPG_PlayerIcon::ApplyIcon()
{
	if (IconImage)
	{
		IconImage->SetOpacity(1.f);
	}

	return SelectCharacterID;
}

void UPG_PlayerIcon::SetPlayerIcon(const int32 CharacterUID)
{
	SelectCharacterID = CharacterUID;

	if (!CharacterDescSubsys)
	{
		if (UGameInstance* GI = GetGameInstance())
			CharacterDescSubsys = GI->GetSubsystem<UPGCharacterDescriptionSubsystem>();
	}

	if (!CharacterDescSubsys)
		return;

	const FCharacterResourceInfo* ResourceInfo = CharacterDescSubsys->GetCharacterResource(CharacterUID);
	if (!ResourceInfo)
		return;

	UTexture2D* Tex = nullptr;

	if (ResourceInfo->CircleIcon.IsValid())
		Tex = ResourceInfo->CircleIcon.Get();
	else
		Tex = ResourceInfo->CircleIcon.LoadSynchronous();

	ApplyIconTexture(Tex);
}

void UPG_PlayerIcon::SetPlayerIcon(const FName& CharacterName)
{
	if (!CharacterDescSubsys)
	{
		if (UGameInstance* GI = GetGameInstance())
			CharacterDescSubsys = GI->GetSubsystem<UPGCharacterDescriptionSubsystem>();
	}

	if (!CharacterDescSubsys)
		return;

	const FCharacterResourceInfo* ResourceInfo = CharacterDescSubsys->GetCharacterResource(CharacterName);
	if (!ResourceInfo)
		return;

	UTexture2D* Tex = nullptr;

	if (ResourceInfo->CircleIcon.IsValid())
		Tex = ResourceInfo->CircleIcon.Get();
	else
		Tex = ResourceInfo->CircleIcon.LoadSynchronous();

	ApplyIconTexture(Tex);
}

void UPG_PlayerIcon::SetPlayerNameText(const FText& InName)
{
	if (PlayerNameBase)
	{
		PlayerNameBase->SetVisibility(ESlateVisibility::Visible);
	}

	if (PlayerName)
	{
		PlayerName->SetText(InName);
	}
}

