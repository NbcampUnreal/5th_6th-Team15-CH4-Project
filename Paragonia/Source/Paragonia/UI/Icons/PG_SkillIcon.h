#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "PG_SkillIcon.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class PARAGONIA_API UPG_SkillIcon : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable)
    void InitSlot(FGameplayTag InCooldownTag);

    UFUNCTION(BlueprintCallable)
	void StartCooldown();

    UFUNCTION(BlueprintCallable)
    void UpdateCooldown(float Remaining, float Duration);

    UFUNCTION(BlueprintCallable)
    void ClearCooldown();

protected:
    virtual void NativeOnInitialized() override;
protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> SkillImage;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> CooldownText;

private:
    FGameplayTag CooldownTag;
};
