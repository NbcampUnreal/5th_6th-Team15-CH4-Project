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
class UPGInventorySlotWidget;
class UPG_KillLog;
class UTextBlock;
class APGPlayerState;


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

	UFUNCTION()
	void InitInventory(UPGInventoryComponent* InInventoryComponent);

	UFUNCTION()
	void UnbindInventory();

	UFUNCTION()
	void RefreshAll();

	UFUNCTION()
	void OnKillEvent(APGPlayerState* ClientPS, class APGPlayerState* KillerPS, class APGPlayerState* VictimPS);

	void InitGold(APGPlayerState* InPS);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleGoldChange(int32 NewGold);
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

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGInventorySlotWidget> Item0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGInventorySlotWidget> Item1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGInventorySlotWidget> Item2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGInventorySlotWidget> Item3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGInventorySlotWidget> Item4;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGInventorySlotWidget> Item5;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_KillLog> KillLog0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_KillLog> KillLog1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_KillLog> KillLog2;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_KillLog> KillLog3;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_KillLog> KillLog4;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPG_KillLog> KillLog5;

	UFUNCTION()
	void InitKillLogSlots();

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* GoldText;

private:
	TMap<FGameplayTag, TObjectPtr<UPG_SkillIcon>> CooldownTagToWidget;

	UPROPERTY()
	TMap<EHPBarSlot, TObjectPtr<UCharacterAttributeSet>> BoundAttrSets;

	UPROPERTY()
	TMap<EHPBarSlot, TObjectPtr<UPG_HPBar>> HPBars;

	UPROPERTY()
	TMap<EHPBarSlot, TObjectPtr<UPG_AttrSetBindProxy>> BindProxies;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPG_KillLog>> KillLogSlots;

	UPROPERTY()
	TMap<int32, TObjectPtr<UPG_KillLog>> KillLogMap;

	UPROPERTY()
	TObjectPtr<UPGInventoryComponent> InventoryComponent;

	UPG_KillLog* GetOrAssignSlot(int32 PlayerId);
#pragma region Chatting

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UChatWidget> ChatWidget;

public:
	void PrintChatMessageString(const FString& PlayerName, const FString& InChatMessageString, const int32& InTeamID);
#pragma endregion

};
