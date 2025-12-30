#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "UI/Bars/PG_AttrSetBindProxy.h"
#include "GameplayTagContainer.h"
#include "PG_IngameHUD.generated.h"

class UPG_HPBar;
class UPG_InGameTeamSimpleInfo;
class UPG_MiniMap;
class UPG_SkillIcon;
class UTextureRenderTarget2D;
class UCharacterAttributeSet;
class UWidgetAnimation;
class UChatWidget;

UCLASS()
class PARAGONIA_API UPG_IngameHUD : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	void BindSlot(EHPBarSlot InSlot, UCharacterAttributeSet* Set);

	UFUNCTION()
	void HandleHealthChangedBySlot(EHPBarSlot InSlot, float OldValue, float NewValue);

	UFUNCTION()
	void HandleMaxHealthChangedBySlot(EHPBarSlot InSlot, float OldValue, float NewValue);

	UFUNCTION(BlueprintCallable)
	void InitMinimap(UTextureRenderTarget2D* InRT);

	UFUNCTION()
	void InitTeam1IngameIcon(int32 CharacterID, const FString& PlayerNickName1);

	UFUNCTION()
	void InitTeam2IngameIcon(int32 CharacterID, const FString& PlayerNickName2);

	UFUNCTION()
	void HandleCooldownTimeChanged(FGameplayTag CooldownTag, float Remaining, float Duration);

	UFUNCTION()
	void HandleCooldownTagChanged(FGameplayTag CooldownTag, int32 NewCount);

protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeDestruct() override;
private:
	void BindCooldownToSkillIcon();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_MiniMap> MiniMapWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_HPBar> PlayerHPBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_InGameTeamSimpleInfo> Team1HPBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_InGameTeamSimpleInfo> Team2HPBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_HPBar> OurNexusHPBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_HPBar> EnemyNexusHPBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_SkillIcon> SkillIconQ;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_SkillIcon> SkillIconE;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_SkillIcon> SkillIconR;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> OnDamaged;

private:
	TMap<FGameplayTag, TObjectPtr<UPG_SkillIcon>> CooldownTagToWidget;

	UPROPERTY()
	TMap<EHPBarSlot, TObjectPtr<UCharacterAttributeSet>> BoundAttrSets;

	UPROPERTY()
	TMap<EHPBarSlot, TObjectPtr<UPG_HPBar>> HPBars;

	UPROPERTY()
	TMap<EHPBarSlot, TObjectPtr<UPG_AttrSetBindProxy>> BindProxies;

#pragma region Chatting

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UChatWidget> ChatWidget;

public:
	void PrintChatMessageString(const FString& PlayerName, const FString& InChatMessageString, const int32& InTeamID);

#pragma endregion

};
