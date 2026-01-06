#include "Controller/PGPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "GameState/PGGameStateBase.h"
#include "PlayerState/PGPlayerState.h"
#include "UI/UW_GameResult.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameMode/PGGameModeBase.h"
#include "Character/PGPlayerCharacterBase.h"
#include "UI/HUDs/PG_IngameHUD.h"
#include "AttributeSet/CharacterAttributeSet.h"
#include "Shop/PGShopComponent.h"
#include "UI/Shop/PGShopWidget.h"
#include "Subsystem/PGCharacterDescriptionSubsystem.h"
#include "Struct/FCharacterResourceInfo.h"
#include "Object/PGNexus.h"
#include "EngineUtils.h" 
#include "Kismet/GameplayStatics.h"

void APGPlayerController::Client_SetExpectedPlayerCount_Implementation(int32 InExpectedPlayerCount)
{
    ExpectedPlayerCount = InExpectedPlayerCount;
}

void APGPlayerController::Client_AllClientsReady_Implementation()
{
    GetWorldTimerManager().ClearTimer(ReadyCheckTimerHandle);

    ShowGameHUD();
    BindIngameHUD();
}

void APGPlayerController::ServerRPC_ReportClientReady_Implementation()
{
    if (APGPlayerState* PS = GetPlayerState<APGPlayerState>())
    {
        PS->bClientReady = true;
    }

    if (APGGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<APGGameModeBase>() : nullptr)
    {
        GM->CheckAllClientsReady();
    }
}

void APGPlayerController::StartReadyCheck()
{
    if (!GetWorld())
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(ReadyCheckTimerHandle);
    GetWorldTimerManager().SetTimer(
        ReadyCheckTimerHandle,
        this,
        &ThisClass::TickReadyCheck,
        ReadyCheckIntervalSeconds,
        true
    );
}

void APGPlayerController::TickReadyCheck()
{
    if (bLocalReadyReported)
    {
        GetWorldTimerManager().ClearTimer(ReadyCheckTimerHandle);
        return;
    }

    if (!AreAllPlayersReplicatedOnThisClient())
    {
        return;
    }

    bLocalReadyReported = true;
    ServerRPC_ReportClientReady();
    GetWorldTimerManager().ClearTimer(ReadyCheckTimerHandle);
}

bool APGPlayerController::AreAllPlayersReplicatedOnThisClient() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    if (ExpectedPlayerCount <= 0)
    {
        return false;
    }

    AGameStateBase* GS = World->GetGameState();
    if (!GS)
    {
        return false;
    }

    APGPlayerState* LocalPS = GetPlayerState<APGPlayerState>();
    if (!IsValid(LocalPS))
    {
        return false;
    }

    if (!IsValid(GetPawn()))
    {
        return false;
    }

    const TArray<APlayerState*>& PlayerArray = GS->PlayerArray;
    if (PlayerArray.Num() < ExpectedPlayerCount)
    {
        return false;
    }

    for (APlayerState* PS : PlayerArray)
    {
        if (!IsValid(PS))
        {
            return false;
        }

        APawn* FoundPawn = PS->GetPawn();
        if (!IsValid(FoundPawn))
        {
            return false;
        }

        if (!FoundPawn->HasActorBegunPlay())
        {
            return false;
        }
    }

    return true;
}
bool APGPlayerController::SetCharacterMinimapIcon(APGPlayerCharacterBase* InCharacter, APGPlayerState* PS, bool IsTeam)
{
    UGameInstance* GI = GetGameInstance();

    if (!IsValid(GI))
    {
        return false;
    }

    UPGCharacterDescriptionSubsystem* CharacterDescSubsys = GI->GetSubsystem<UPGCharacterDescriptionSubsystem>();

    if (!IsValid(CharacterDescSubsys))
    {
        return false;
    }

    const FCharacterResourceInfo* ResourceInfo = CharacterDescSubsys->GetCharacterResource(PS->GetCharID());

    if (!ResourceInfo)
    {
        return false;
    }

    UPaperSprite* Sprite = nullptr;

    if (IsTeam)
    {
        if (ResourceInfo->MinimapTeamIcon.IsValid())
        {
            Sprite = ResourceInfo->MinimapTeamIcon.Get();
        }
        else
        {
            Sprite = ResourceInfo->MinimapTeamIcon.LoadSynchronous();
        }
    }
    else
    {
        if (ResourceInfo->MinimapEnemyIcon.IsValid())
        {
            Sprite = ResourceInfo->MinimapEnemyIcon.Get();
        }
        else
        {
            Sprite = ResourceInfo->MinimapEnemyIcon.LoadSynchronous();
        }
    }

    if (!Sprite)
    {
        return false;
    }

    InCharacter->SetMinimapSprite(Sprite);

    return true;
}

APGPlayerController::APGPlayerController()
{
    ShopComponent = CreateDefaultSubobject<UPGShopComponent>(TEXT("ShopComponent"));
}

bool APGPlayerController::BindIngameHUD()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    AGameStateBase* GS = World->GetGameState();
    if (!GS)
    {
        return false;
    }

    const TArray<APlayerState*>& PlayerArray = GS->PlayerArray;

    if (PlayerArray.Num() < ExpectedPlayerCount)
    {
        return false;
    }

    APGPlayerState* LocalPS = GetPlayerState<APGPlayerState>();
    if (!IsValid(LocalPS))
    {
        return false;
    }

    for (TActorIterator<APGNexus> It(GetWorld()); It; ++It)
    {
        APGNexus* Nexus = *It;
        if (Nexus && Nexus->ActorHasTag(FName("Nexus")))
        {
            if (Nexus->GetNexusTeamID() != LocalPS->GetTeamID())
            {
                IngameHUD->BindSlot(EHPBarSlot::EnemyNexus, Nexus->GetObjectAttributeSet());
            }
            else
            {
                IngameHUD->BindSlot(EHPBarSlot::OurNexus, Nexus->GetObjectAttributeSet());
            }
        }
    }

    if (!SetMyHPBar(LocalPS))
    {
        return false;
    }

    if (!SetTeamHPBar(PlayerArray, LocalPS))
    {
        return false;
    }

    return true;
}

void APGPlayerController::Client_KillInfo_Implementation(APGPlayerState* KillerPS, APGPlayerState* VictimPS)
{
    APGPlayerState* LocalPS = GetPlayerState<APGPlayerState>();
    if (!IsValid(LocalPS))
    {
        return;
    }

    IngameHUD->OnKillEvent(LocalPS, KillerPS, VictimPS);
}

bool APGPlayerController::SetMyHPBar(APGPlayerState* LocalPS)
{
    APGPlayerCharacterBase* FoundMyCharacter = LocalPS->GetPawn<APGPlayerCharacterBase>();
    if (!IsValid(FoundMyCharacter))
    {
        return false;
    }

    if (!FoundMyCharacter->HasActorBegunPlay())
    {
        return false;
    }

    UCharacterAttributeSet* MyAttributeSet = FoundMyCharacter->GetCharacterAttributeSet();

    if (!IsValid(MyAttributeSet))
    {
        return false;
    }

    if (!SetCharacterMinimapIcon(FoundMyCharacter, LocalPS, true))
    {
        return false;
    }

    APGPlayerState* MyPS = Cast<APGPlayerState>(PlayerState);
    if (!MyPS)
    {
        return false;
    }

    IngameHUD->BindSlot(EHPBarSlot::Player, MyAttributeSet);
    //IngameHUD->InitInventory(MyPS->GetInventoryComponent());
    IngameHUD->InitMinimap(FoundMyCharacter->GetMinimapRenderTarget());

    IngameHUD->InitGold(MyPS);

    return true;
}

bool APGPlayerController::SetTeamHPBar(const TArray<APlayerState*>& PlayerArray, APGPlayerState* LocalPS)
{
    int index = 0;

    for (APlayerState* PS : PlayerArray)
    {
        if (!IsValid(PS))
        {
            return false;
        }

        APGPlayerState* PGPS = Cast<APGPlayerState>(PS);

        int32 PSTeamId = 255;
        if (IsValid(PGPS))
        {
            PSTeamId = PGPS->GetTeamID();
        }

        int32 LocalPSTeamId = 255;
        if (IsValid(LocalPS))
        {
            LocalPSTeamId = LocalPS->GetTeamID();
        }

        if (PGPS == LocalPS)
        {
            continue;
        }

        APGPlayerCharacterBase* FoundCharacter = PS->GetPawn<APGPlayerCharacterBase>();
        if (!IsValid(FoundCharacter))
        {
            return false;
        }

        if (!FoundCharacter->HasActorBegunPlay())
        {
            return false;
        }

        FoundCharacter->SetIngameInfo(PSTeamId != LocalPSTeamId, PGPS->GetPlayerNickName());

        if (PSTeamId != LocalPSTeamId)
        {
            continue;
        }

        UCharacterAttributeSet* TeamAttributeSet = FoundCharacter->GetCharacterAttributeSet();

        if (!IsValid(TeamAttributeSet))
        {
            return false;
        }

        if (!SetCharacterMinimapIcon(FoundCharacter, PGPS, LocalPS->GetTeamID() == PGPS->GetTeamID()))
        {
            return false;
        }

        switch (index)
        {
        case 0:
            IngameHUD->BindSlot(EHPBarSlot::Team1, TeamAttributeSet);
            IngameHUD->InitTeam1IngameIcon(PGPS->GetCharID(), PGPS->GetPlayerNickName());
            break;
        default:
            IngameHUD->BindSlot(EHPBarSlot::Team2, TeamAttributeSet);
            IngameHUD->InitTeam2IngameIcon(PGPS->GetCharID(), PGPS->GetPlayerNickName());
            break;
        }

        index++;
    }

    return true;
}

void APGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContexts, 0);
		}
	}
}

void APGPlayerController::BeginPlay()
{
	Super::BeginPlay();

    APGGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<APGGameStateBase>() : nullptr;
    if (GS)
    {
        // Delegate에 함수 바인딩 (동적 바인딩)
        GS->OnTeamResultChanged.AddDynamic(this, &APGPlayerController::OnTeamResultChanged);
    }

    if (IsLocalPlayerController())
    {
        StartReadyCheck();
        ShowLoadingHUD();
    }
}

void APGPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    APGGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<APGGameStateBase>() : nullptr;
    if (GS)
    {
        GS->OnTeamResultChanged.RemoveDynamic(this, &APGPlayerController::OnTeamResultChanged);
    }
}

#pragma region GameResult
void APGPlayerController::OnTeamResultChanged(ETeamResult NewResult)
{
    APGGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<APGGameStateBase>() : nullptr;
    if (!GS)
    {
        return;
    }

    if (LastTeamResult != GS->TeamResult)
    {
        LastTeamResult = GS->TeamResult;

        // 자신의 PlayerState 팀 정보 얻기
        APGPlayerState* MyPS = Cast<APGPlayerState>(PlayerState);
        if (!MyPS)
        {
            return;
        }

        int32 TeamId = 255;
        if (IsValid(MyPS))
        {
            TeamId = MyPS->GetTeamID();
        }

        bool bIsWinner = false;
        switch (GS->TeamResult)
        {
        case ETeamResult::Team1Win:
            bIsWinner = (TeamId == 0);
            break;
        case ETeamResult::Team2Win:
            bIsWinner = (TeamId == 1);
            break;
        default:
            break;
        }

        // UI 업데이트 및 게임 종료 처리 예시
        if (bIsWinner)
        {
            ShowWinWidget(1);
        }
        else
        {
            ShowWinWidget(0);
        }
    }

    // 계속해서 팀 결과 감지를 위한 타이머를 설정하거나 OnRep에 연결 가능
}

void APGPlayerController::ShowWinWidget(uint8 IsWin)
{
    if (LoadingHUD)
    {
        LoadingHUD->RemoveFromParent();
        LoadingHUD = nullptr;
    }

    if (IngameHUD)
    {
        IngameHUD->RemoveFromParent();
        IngameHUD = nullptr;
    }

    if (IsValid(GameResultUIClass) == true)
    {
        GameResultUI = CreateWidget<UUW_GameResult>(this, GameResultUIClass);
        if (IsValid(GameResultUI) == true)
        {
            GameResultUI->AddToViewport(0);

            FText GameResultText = (IsWin == 1)
                ? FText::FromString(TEXT("Victory"))
                : FText::FromString(TEXT("Defeat")); 
			GameResultUI->ResultText->SetText(GameResultText);

            FInputModeUIOnly Mode;
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            SetInputMode(Mode);

            bShowMouseCursor = true;
        }
    }
}

void APGPlayerController::ShowGameHUD()
{
    if (LoadingHUD)
    {
        LoadingHUD->RemoveFromParent();
        LoadingHUD = nullptr;
    }

    if (GameResultUI)
    {
        GameResultUI->RemoveFromParent();
        GameResultUI = nullptr;
    }

    if (IngameHUDClass)
    {
        IngameHUD = CreateWidget<UPG_IngameHUD>(this, IngameHUDClass);
        if (IngameHUD)
        {
            IngameHUD->AddToViewport();
            bShowMouseCursor = false;
            SetInputMode(FInputModeGameOnly());
        }
    }
}

void APGPlayerController::ShowLoadingHUD()
{
    if (GameResultUI)
    {
        GameResultUI->RemoveFromParent();
        GameResultUI = nullptr;
    }

    if (IngameHUD)
    {
        IngameHUD->RemoveFromParent();
        IngameHUD = nullptr;
    }

    if (LoadingHUDClass)
    {
        LoadingHUD = CreateWidget<UUserWidget>(this, LoadingHUDClass);
        if (LoadingHUD)
        {
            LoadingHUD->AddToViewport();
            bShowMouseCursor = false;
            SetInputMode(FInputModeUIOnly());
        }
    }
}

void APGPlayerController::ToggleShop()
{
    // 없으면 생성
    if (!ShopWidget && ShopWidgetClass)
    {
        ShopWidget = CreateWidget<UPGShopWidget>(this, ShopWidgetClass);
        if (ShopWidget)
        {
            ShopWidget->AddToViewport(50);

            if (ShopComponent)
            {
                ShopWidget->InitWithShopComponent(ShopComponent);
            }
        }
    }

    if (!ShopWidget) return;

    const bool bIsVisible = ShopWidget->IsInViewport() && ShopWidget->GetVisibility() != ESlateVisibility::Collapsed;

    if (!bIsVisible)
    {
        // 열기
        ShopWidget->SetVisibility(ESlateVisibility::Visible);

        FInputModeGameAndUI Mode;
        Mode.SetWidgetToFocus(ShopWidget->TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(Mode);

        bShowMouseCursor = true;
    }
    else
    {
        // 닫기
        ShopWidget->SetVisibility(ESlateVisibility::Collapsed);

        SetInputMode(FInputModeGameOnly());
        bShowMouseCursor = false;
    }
}

#pragma endregion GameResult

#pragma region Chatting

void APGPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
    if (IsLocalController())
    {
        APGPlayerState* PGPS = GetPlayerState<APGPlayerState>();
        if (!IsValid(PGPS))
        {
            return;
        }

        ServerRPCPrintChatMessageString(InChatMessageString);
    }
}

void APGPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
    AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
    if (!IsValid(GM))
    {
        return;
    }

    APGGameModeBase* PGGM = Cast<APGGameModeBase>(GM);
    if (!IsValid(PGGM))
    {
        return;
    }

    APGPlayerCharacterBase* PlayerCharacter = Cast<APGPlayerCharacterBase>(GetPawn());
    if (!IsValid(PlayerCharacter))
    {
        return;
    }

    PGGM->PrintChatMessageString(this, InChatMessageString, IPGTeamStatusInterface::Execute_GetTeamID(PlayerCharacter));
}

void APGPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& PlayerName, const FString& InChatMessageString, const int32& InTeamID)
{
    PrintChatMessageString(PlayerName, InChatMessageString, InTeamID);
}

void APGPlayerController::PrintChatMessageString(const FString& PlayerName, const FString& InChatMessageString, const int32& InTeamID)
{
    if (!IsValid(IngameHUD)) return;

    IngameHUD->PrintChatMessageString(PlayerName, InChatMessageString, InTeamID);
}

#pragma endregion

