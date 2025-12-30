// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Bars/PG_HPBar.h"
#include "Components/Image.h"

void UPG_HPBar::NativeOnInitialized()
{
	Super::NativeOnInitialized();

}

float UPG_HPBar::HandleHealthChanged(float OldValue, float NewValue)
{
	NowHPValue = NewValue;    

    if (MaxHPValue == 0)
    {
        return 0;
    }

    float SetOldValue = OldValue / MaxHPValue;
    float SetNewValue = NewValue / MaxHPValue;

    UMaterialInstanceDynamic* BarFillMID = BarFill->GetDynamicMaterial();
    UMaterialInstanceDynamic* BarGlowMID = BarGlow->GetDynamicMaterial();

    if (!BarFillMID || !BarGlowMID)
    {
        return 0;
    }

    StopAnimation(Damaged);

    BarFillMID->SetScalarParameterValue(TEXT("HealthCurrent"), SetOldValue);
    BarFillMID->SetScalarParameterValue(TEXT("HealthUpdate"), SetNewValue);
    BarGlowMID->SetScalarParameterValue(TEXT("Health_Current"), SetOldValue);
    BarGlowMID->SetScalarParameterValue(TEXT("Health_Updated"), SetNewValue);

    if (OldValue > NewValue)
    {
        PlayAnimation(Damaged, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
    }

    return SetNewValue;
}

void UPG_HPBar::HandleMaxHealthChanged(float OldValue, float NewValue)
{
	MaxHPValue = NewValue;

    if (MaxHPValue == 0)
    {
        return;
    }

    float SetOldValue = NowHPValue / OldValue;
    float SetNewValue = NowHPValue / NewValue;

    UMaterialInstanceDynamic* BarFillMID = BarFill->GetDynamicMaterial();
    UMaterialInstanceDynamic* BarGlowMID = BarGlow->GetDynamicMaterial();

    if (!BarFillMID || !BarGlowMID)
    {
        return;
    }

    BarFillMID->SetScalarParameterValue(TEXT("HealthCurrent"), SetOldValue);
    BarFillMID->SetScalarParameterValue(TEXT("HealthUpdate"), SetNewValue);
    BarGlowMID->SetScalarParameterValue(TEXT("Health_Current"), SetOldValue);
    BarGlowMID->SetScalarParameterValue(TEXT("Health_Updated"), SetNewValue);

    StopAnimation(Damaged);

    if (SetOldValue > SetNewValue)
    {
        PlayAnimation(Damaged, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
    }
}

void UPG_HPBar::AddBuff(int32 BuffUID)
{
}

void UPG_HPBar::SetPlayerColor()
{
    UMaterialInstanceDynamic* BarFillMID = BarFill->GetDynamicMaterial();

    if (!BarFillMID)
    {
        return;
    }

    BarFillMID->SetScalarParameterValue(TEXT("PlayerCheck"), 1);
}

//0 팀
//1 적
void UPG_HPBar::SetTeamColor(bool TeamType)
{
    UMaterialInstanceDynamic* BarFillMID = BarFill->GetDynamicMaterial();

    if (!BarFillMID)
    {
        return;
    }

    BarFillMID->SetScalarParameterValue(TEXT("PlayerCheck"), 0);
    BarFillMID->SetScalarParameterValue(TEXT("TeamCheck"), TeamType);
}
