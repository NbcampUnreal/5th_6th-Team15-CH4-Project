#include "Subsystem/ConnectSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

void UConnectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GetGameInstance()->IsDedicatedServerInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ConnectSubsystem] I am Dedicated Server. Creating Session..."));
		CreateGameSession();
	}
	else
	{
		Login();
	}
}

// =========================================================
// [CLIENT] 로그인
// =========================================================
void UConnectSubsystem::Login()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OSS] Subsystem=%s"),
			Subsystem ? *Subsystem->GetSubsystemName().ToString() : TEXT("NULL"));

		IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
		if (Identity.IsValid())
		{
			ELoginStatus::Type Status = Identity->GetLoginStatus(0);

			// 1. 이미 로그인이 되어 있다면? -> 바로 성공 처리하고 다음 단계로!
			if (Status == ELoginStatus::LoggedIn)
			{
				UE_LOG(LogTemp, Warning, TEXT("[ConnectSubsystem] Already Logged In! Skipping Login Process."));

				// 수동으로 '로그인 완료' 함수를 호출해서 다음 로직(세션 찾기 등)이 실행되게 합니다.
				FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
				OnLoginComplete(0, true, *UserId, TEXT("AlreadyLoggedIn"));
				return;
			}

			// 2. 로그인이 안 되어 있다면? -> 원래대로 로그인 시도
			FOnlineAccountCredentials Credentials;
			Credentials.Type = TEXT("AccountPortal");
			Credentials.Id = TEXT("");
			Credentials.Token = TEXT("");

			Identity->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateUObject(this, &UConnectSubsystem::OnLoginComplete));

			Identity->Login(0, Credentials);
		}
	}
}

void UConnectSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("[ConnectSubsystem] Login Success! UserNetId: %s"), *UserId.ToString());
		bIsLoggedIn = true;

		FindAndJoinSession();

		if (OnLoginSuccessDelegate.IsBound())
		{
			OnLoginSuccessDelegate.Broadcast();
		}
	}
	else
	{
		bIsLoggedIn = false;
		UE_LOG(LogTemp, Error, TEXT("[ConnectSubsystem] Login Failed: %s"), *Error);
	}
}

// =========================================================
// [SERVER] 방 만들기
// =========================================================
void UConnectSubsystem::CreateGameSession()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	UE_LOG(LogTemp, Warning, TEXT("[OSS] Subsystem=%s"),
		Subsystem ? *Subsystem->GetSubsystemName().ToString() : TEXT("NULL"));

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		// 혹시 기존 세션이 있다면 정리
		auto ExistingSession = SessionInterface->GetNamedSession(FName("MySession"));
		if (ExistingSession)
		{
			SessionInterface->DestroySession(FName("MySession"));
		}

		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UConnectSubsystem::OnCreateSessionComplete));

		FOnlineSessionSettings Settings;

		Settings.bIsDedicated = true;       // 중요: 데디 서버임을 명시
		Settings.bIsLANMatch = false;       // 스팀/에픽 망 사용
		//Settings.bIsLANMatch = true;       // 테스트용
		Settings.NumPublicConnections = 10; // 최대 인원
		Settings.bShouldAdvertise = true;   // 검색 허용
		Settings.bUsesPresence = false;     // 데디 서버는 플레이어가 아니므로 Presence(상태) 없음
		Settings.bUseLobbiesIfAvailable = false;
		Settings.bAllowJoinInProgress = true;
		Settings.bAllowJoinViaPresence = false;

		// 매치 타입 태그 (클라이언트가 이걸로 검색함)
		Settings.Set(FName("MatchType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineService);

		UE_LOG(LogTemp, Log, TEXT("[ConnectSubsystem] Creating Dedicated Session..."));
		SessionInterface->CreateSession(0, FName("MySession"), Settings);
	}
}

void UConnectSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		// 서버는 이미 맵을 로드하고 있으므로 이동(Travel) 불필요
		UE_LOG(LogTemp, Warning, TEXT("[ConnectSubsystem] SERVER SESSION CREATED! Ready for clients."));

		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (!Subsystem) 
			return;

		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

		auto Named = SessionInterface->GetNamedSession(FName("MySession"));
		if (Named)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Server] State=%d ShouldAdvertise=%d NumPublic=%d"),
				(int32)Named->SessionState,
				(int32)Named->SessionSettings.bShouldAdvertise,
				Named->SessionSettings.NumPublicConnections);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[ConnectSubsystem] Server Session Creation Failed!"));
	}
}

// =========================================================
// [CLIENT] 방 찾기 및 입장
// =========================================================
void UConnectSubsystem::FindAndJoinSession()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem)
		return;

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		auto ExistingSession = SessionInterface->GetNamedSession(FName("MySession"));

		if (ExistingSession != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ConnectSubsystem] Old Session Found! Destroying it before searching..."));

			// 2. 찌꺼기가 있다면 파괴(Destroy)부터 진행
			// 파괴가 끝나면 OnDestroySessionComplete가 호출되도록 연결
			DestroySessionDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
				FOnDestroySessionCompleteDelegate::CreateUObject(this, &UConnectSubsystem::OnDestroySessionComplete));

			SessionInterface->DestroySession(FName("MySession"));

			return;
		}

		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->bIsLanQuery = false;
		SessionSearch->MaxSearchResults = 20000;

		SessionSearch->QuerySettings.Set(FName("SEARCH_PRESENCE"), false, EOnlineComparisonOp::Equals);

		SessionSearch->QuerySettings.Set(FName("MatchType"), FString("FreeForAll"), EOnlineComparisonOp::Equals);

		SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UConnectSubsystem::OnFindSessionsComplete));

		UE_LOG(LogTemp, Log, TEXT("[ConnectSubsystem] Finding Sessions..."));
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UConnectSubsystem::TravelToGame()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 데디케이티드 서버거나, 리슨 서버의 호스트인지 확인 (HasAuthority로도 충분)
	if (!GetGameInstance()->IsDedicatedServerInstance() && !World->GetAuthGameMode())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ConnectSubsystem] StartGame is Server Only!"));
		return;
	}

	// 2. INI 파일에서 GameLevelPath 읽어오기
	FString GameLevelPath;
	if (GConfig)
	{
		GConfig->GetString(
			TEXT("/Script/Paragonia.ConnectSubsystem"), // 섹션 이름
			TEXT("GameLevelPath"),                      // 키 이름
			GameLevelPath,                              // 저장할 변수
			GGameIni                                    // 파일 (DefaultGame.ini)
		);
	}

	if (GameLevelPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ConnectSubsystem] GameLevelPath in INI is Empty! Using Default."));
		GameLevelPath = TEXT("/Game/Paragonia/Maps/Game");
	}

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->StartSession(FName("MySession"));
		}
	}

	FString Url = GameLevelPath;

	UE_LOG(LogTemp, Log, TEXT("[ConnectSubsystem] Starting Game... Traveling to: %s"), *Url);

	World->ServerTravel(Url);
}

void UConnectSubsystem::TravelToLobby()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	if (!GetGameInstance()->IsDedicatedServerInstance() && !World->GetAuthGameMode())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ConnectSubsystem] TravelToLobby is Server Only!"));
		return;
	}

	FString LobbyLevelPath;
	if (GConfig)
	{
		GConfig->GetString(
			TEXT("/Script/Paragonia.ConnectSubsystem"),
			TEXT("LobbyLevelPath"),
			LobbyLevelPath,
			GGameIni
		);
	}

	if (LobbyLevelPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ConnectSubsystem] LobbyLevelPath is Empty! Using Default."));
		LobbyLevelPath = TEXT("/Game/Paragonia/Maps/Lobby");
	}

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->EndSession(FName("MySession"));
		}
	}

	FString Url = LobbyLevelPath;
	Url += TEXT("?listen");

	UE_LOG(LogTemp, Log, TEXT("[ConnectSubsystem] Returning to Lobby: %s"), *Url);

	World->ServerTravel(Url);
}

void UConnectSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("[ConnectSubsystem] Find Found %d Sessions."), SessionSearch->SearchResults.Num());

		if (SessionSearch->SearchResults.Num() > 0)
		{
			FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[0];

			IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
			IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

			SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UConnectSubsystem::OnJoinSessionComplete));

			UE_LOG(LogTemp, Log, TEXT("[ConnectSubsystem] Joining Session..."));
			SessionInterface->JoinSession(0, FName("MySession"), Result);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ConnectSubsystem] No Sessions Found."));
	}
}

void UConnectSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

		FString ConnectString;
		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
		{
			APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
			if (PC)
			{
				UE_LOG(LogTemp, Log, TEXT("[ConnectSubsystem] Traveling to Server: %s"), *ConnectString);
				PC->ClientTravel(ConnectString, TRAVEL_Absolute);
			}
		}
	}
}

void UConnectSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
		}
	}

	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("[ConnectSubsystem] Old Session Destroyed. Restarting FindAndJoinSession..."));

		FindAndJoinSession();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[ConnectSubsystem] Failed to destroy old session."));
	}
}