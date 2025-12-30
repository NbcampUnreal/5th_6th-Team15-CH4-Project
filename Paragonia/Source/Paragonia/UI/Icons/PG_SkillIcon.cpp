#include "UI/Icons/PG_SkillIcon.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPG_SkillIcon::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (SkillImage)
	{
		SkillImage->SetVisibility(ESlateVisibility::Hidden);
	}

	if (CooldownText)
	{
		CooldownText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPG_SkillIcon::InitSlot(FGameplayTag InCooldownTag)
{
	CooldownTag = InCooldownTag;

	ClearCooldown();
}

void UPG_SkillIcon::StartCooldown()
{
	if (SkillImage)
	{
		SkillImage->SetVisibility(ESlateVisibility::Visible);
	}

	if (CooldownText)
	{
		CooldownText->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPG_SkillIcon::UpdateCooldown(float Remaining, float Duration)
{
	if (Remaining <= 0.f || Duration <= 0.f)
	{
		ClearCooldown();
		return;
	}

	if (CooldownText)
	{
		const float RemainTime = FMath::Max(0.f, Remaining);
		CooldownText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), RemainTime)));
	}
}

void UPG_SkillIcon::ClearCooldown()
{
	if (SkillImage)
	{
		SkillImage->SetVisibility(ESlateVisibility::Hidden);
	}

	if (CooldownText)
	{
		CooldownText->SetVisibility(ESlateVisibility::Hidden);
	}
}
