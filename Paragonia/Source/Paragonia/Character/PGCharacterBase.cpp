// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PGCharacterBase.h"
#include "Components/WidgetComponent.h"
#include "PaperSpriteComponent.h"
#include "Character/PG_PlayerUIComponent.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/CharacterAttributeSet.h"

APGCharacterBase::APGCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	CharacterAttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
#if !UE_SERVER
	HeadHPWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HeadHPWidgetComp"));
	HeadHPWidgetComp->SetupAttachment(GetMesh());
	HeadHPWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	HeadHPWidgetComp->SetDrawAtDesiredSize(true);
	HeadHPWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 240.f));
	HeadHPWidgetComp->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	HeadHPWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MinimapIcon = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("MinimapIcon"));
	MinimapIcon->SetupAttachment(RootComponent);
	MinimapIcon->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	MinimapIcon->SetRelativeRotation(FRotator(0.f, 90.f, 90.f));
	MinimapIcon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MinimapIcon->SetGenerateOverlapEvents(false);
	MinimapIcon->CastShadow = false;
	MinimapIcon->SetVisibleInSceneCaptureOnly(true);

	UIComponent = CreateDefaultSubobject<UPG_PlayerUIComponent>(TEXT("UIComponent"));
	UIComponent->InitComponents(this, HeadHPWidgetComp, MinimapIcon, CharacterAttributeSet);
#endif
}

void APGCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(UIComponent))
	{
		UIComponent->SetupHeadHPWidget();
	}
}

void APGCharacterBase::SetIngameHPBarColor(int32 TeamType)
{
	if (IsValid(UIComponent))
	{
		UIComponent->SetHPBarColor(TeamType);
	}
}

UAbilitySystemComponent* APGCharacterBase::GetAbilitySystemComponent() const
{
	return ASC;
}

int32 APGCharacterBase::GetTeamID_Implementation() const
{
	return TeamId;
}

bool APGCharacterBase::GetIsDead_Implementation() const
{
	return false;
}

void APGCharacterBase::SetMinimapSprite(UPaperSprite* NewSprite)
{
	if (!MinimapIcon)
	{
		return;
	}

	MinimapIcon->SetSprite(NewSprite);
}