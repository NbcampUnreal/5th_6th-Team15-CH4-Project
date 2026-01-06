#include "Character/PGPlayerCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "AbilitySystemComponent.h"
#include "PlayerState/PGPlayerState.h"
#include "AttributeSet/CharacterAttributeSet.h"
#include "GA/PGGameplayAbilityBase.h"
#include "Struct/FAttackData.h"
#include "GameMode/PGGameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "Subsystem/PGAttributeDataSubsystem.h"
#include "Struct/FCharacterAttributeData.h"
#include "PlayerStart/PGPlayerStart.h"
#include "Controller/PGPlayerController.h"
#include "Components/WidgetComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PaperSpriteComponent.h"
#include "Character/PG_PlayerUIComponent.h"
#include "EngineUtils.h"

APGPlayerCharacterBase::APGPlayerCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->TargetOffset = FVector(0.f, 0.f, 150.f);
	SpringArm->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	MiniMapSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("MiniMapSpringArm"));
	MiniMapSpringArm->SetupAttachment(RootComponent);
	MiniMapSpringArm->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	MiniMapSpringArm->TargetArmLength = 3000.0f;
	MiniMapSpringArm->bUsePawnControlRotation = false;

	MinimapCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MinimapCaptureComponent"));
	MinimapCaptureComponent->SetupAttachment(MiniMapSpringArm, USpringArmComponent::SocketName);
	MinimapCaptureComponent->bCaptureEveryFrame = false;
	MinimapCaptureComponent->bCaptureOnMovement = false;
	MinimapCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void APGPlayerCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitializeActorInfo();

	if (HasAuthority())
	{
		InitializeAbilities();
		InitializeAttributes();
		InitializeAttributesData();
	}
}

void APGPlayerCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (IsValid(UIComponent))
	{
		UIComponent->SetupHeadHPWidget();
	}
}

void APGPlayerCharacterBase::OnRep_Controller()
{
	Super::OnRep_Controller();

	InitializeActorInfo();
	BindAttributeChangeDelegates();

	FInputModeGameOnly InputMode;
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetInputMode(InputMode);
	}

	BindCooldownTagEvent();

	if (IsValid(UIComponent))
	{
		UIComponent->SetupHeadHPWidget();
	}
}

UTextureRenderTarget2D* APGPlayerCharacterBase::GetMinimapRenderTarget()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return nullptr;
	}

	if (!IsValid(UIComponent))
	{
		return nullptr;
	}

	MinimapRT = UIComponent->GetMinimapRenderTarget();

	if (!MinimapRT)
	{
		return nullptr;
	}

	if (MinimapCaptureComponent)
	{
		MinimapCaptureComponent->TextureTarget = MinimapRT;
		MinimapCaptureComponent->CaptureScene();
	}

	return MinimapRT;
}

void APGPlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();

}

void APGPlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (IsValid(EnhancedInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::StartJump);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ThisClass::Attack);
		EnhancedInputComponent->BindAction(SkillQAction, ETriggerEvent::Triggered, this, &ThisClass::SkillQ);
		EnhancedInputComponent->BindAction(SkillEAction, ETriggerEvent::Triggered, this, &ThisClass::SkillE);
		EnhancedInputComponent->BindAction(SkillRAction, ETriggerEvent::Triggered, this, &ThisClass::SkillR);

		EnhancedInputComponent->BindAction(ToggleShop, ETriggerEvent::Started, this, &ThisClass::ToggleShopInput);
	}

	if (IsValid(UIComponent))
	{
		UIComponent->UpdateHeadHPVisibility();

		if (MinimapCaptureComponent)
		{
			MinimapCaptureComponent->bCaptureOnMovement = true;
			MinimapCaptureComponent->bCaptureEveryFrame = false;
			MinimapCaptureComponent->ShowFlags.SetSkeletalMeshes(false);
			MinimapCaptureComponent->ShowFlags.SetParticles(false);

			MinimapCaptureComponent->MarkRenderStateDirty();
		}
	}
}

void APGPlayerCharacterBase::Move(const FInputActionValue& Value)
{
	if (!IsLocallyControlled() || bInputLock)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.X);
		AddMovementInput(RightDirection, MovementVector.Y);
	}
}

void APGPlayerCharacterBase::Look(const FInputActionValue& Value)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APGPlayerCharacterBase::StartJump(const FInputActionValue& Value)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	ASC->AbilityLocalInputPressed(static_cast<int32>(EPGAbilityInputID::Jump));
}

void APGPlayerCharacterBase::StopJump(const FInputActionValue& Value)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	StopJumping();
}

void APGPlayerCharacterBase::Attack(const FInputActionValue& Value)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	ASC->AbilityLocalInputPressed(static_cast<int32>(EPGAbilityInputID::Attack));
}

void APGPlayerCharacterBase::SkillQ(const FInputActionValue& Value)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	ASC->AbilityLocalInputPressed(static_cast<int32>(EPGAbilityInputID::SkillQ));
}

void APGPlayerCharacterBase::SkillE(const FInputActionValue& Value)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	ASC->AbilityLocalInputPressed(static_cast<int32>(EPGAbilityInputID::SkillE));
}

void APGPlayerCharacterBase::SkillR(const FInputActionValue& Value)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	ASC->AbilityLocalInputPressed(static_cast<int32>(EPGAbilityInputID::SkillR));
}

void APGPlayerCharacterBase::ToggleShopInput()
{
	if (APGPlayerController* PC = Cast<APGPlayerController>(GetController()))
	{
		PC->ToggleShop();
	}
}

void APGPlayerCharacterBase::InitializeActorInfo()
{
	ASC->InitAbilityActorInfo(this, this);
}

void APGPlayerCharacterBase::InitializeAbilities()
{
	if (!IsValid(ASC))
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : AllAbilities)
	{
		if (!IsValid(AbilityClass))
		{
			continue;
		}

		UGameplayAbility* AbilityCDO = AbilityClass->GetDefaultObject<UGameplayAbility>();
		int32 InputID = 0;

		const UPGGameplayAbilityBase* PGAbility = Cast<UPGGameplayAbilityBase>(AbilityCDO);
		if (IsValid(PGAbility))
		{
			InputID = static_cast<int32>(PGAbility->InputID);
		}

		FGameplayAbilitySpec Spec(AbilityClass, 1, InputID, this);
		if (!ASC->FindAbilitySpecFromClass(AbilityClass))
		{
			ASC->GiveAbility(Spec);
		}
	}

	const FGameplayTag AirborneTag = FGameplayTag::RequestGameplayTag("Character.State.Airborne");
	ASC->RegisterGameplayTagEvent(AirborneTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnAirborneTagChanged);
}

void APGPlayerCharacterBase::InitializeAttributes()
{
	if (!IsValid(ASC))
	{
		UE_LOG(LogTemp, Warning, TEXT("APlayerCharacterBase::InitializeAttributes - ASC is not valid"));
		return;
	}

	APGPlayerState* PS = GetPlayerState<APGPlayerState>();
	if (!IsValid(PS))
	{
		UE_LOG(LogTemp, Warning, TEXT("APlayerCharacterBase::InitializeAttributes - PlayerState is not valid"));
		return;
	}

	ASC->AddAttributeSetSubobject<UCharacterAttributeSet>(CharacterAttributeSet);

	BindAttributeChangeDelegates();
}

void APGPlayerCharacterBase::InitializeAttributesData()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(CharacterAttributeSet))
	{
		UE_LOG(LogTemp, Warning, TEXT("APlayerCharacterBase::InitializeAttributesData - CharacterAttributeSet is not valid"));
		return;
	}
	
	UPGAttributeDataSubsystem* AttributeSubsystem = GetGameInstance()->GetSubsystem<UPGAttributeDataSubsystem>();
	if (!IsValid(AttributeSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("APlayerCharacterBase::InitializeAttributesData - AttributeSubsystem is not valid"));
		return;
	}

	const FCharacterAttributeData* AttributeData = AttributeSubsystem->GetAttributeDataByName(CharacterName);
	if (AttributeData)
	{
		CharacterAttributeSet->InitMaxHealth(AttributeData->MaxHealth);
		CharacterAttributeSet->InitHealth(AttributeData->Health);
		CharacterAttributeSet->InitDefense(AttributeData->Defense);
		CharacterAttributeSet->InitAttackPower(AttributeData->AttackPower);
		CharacterAttributeSet->InitMoveSpeed(AttributeData->MoveSpeed);
		GetCharacterMovement()->MaxWalkSpeed = AttributeData->MoveSpeed;

		CharacterAttributeSet->OutOfHealthChanged.AddDynamic(this, &ThisClass::OnOutOfHealth);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("APlayerCharacterBase::InitializeAttributesData - No AttributeData found for %s"), *CharacterName.ToString());
	}
}

void APGPlayerCharacterBase::BindAttributeChangeDelegates()
{
	if (!IsValid(ASC))
	{
		return;
	}

	ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetDefenseAttribute()).AddUObject(this, &ThisClass::OnDefenseChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetAttackPowerAttribute()).AddUObject(this, &ThisClass::OnAttackPowerChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &ThisClass::OnMoveSpeedChanged);
}

void APGPlayerCharacterBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.0f)
	{
		if (bIsDead == 0.0f) {
			ServerRPCSetDeadState(true);
		}
	}
}

void APGPlayerCharacterBase::OnDefenseChanged(const FOnAttributeChangeData& Data)
{
}

void APGPlayerCharacterBase::OnAttackPowerChanged(const FOnAttributeChangeData& Data)
{
}

void APGPlayerCharacterBase::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

void APGPlayerCharacterBase::OnAirborneTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		if (HasAuthority())
		{
			LaunchCharacter(FVector(0.f, 0.f, 400.f), true, true);
		}
	}
}

void APGPlayerCharacterBase::BindCooldownTagEvent()
{
	if (!IsValid(ASC))
	{
		return;
	}

	TArray<FGameplayTag> CooldownTags;
	CooldownTags.Add(FGameplayTag::RequestGameplayTag(FName("Cooldown.Skill.Q")));
	CooldownTags.Add(FGameplayTag::RequestGameplayTag(FName("Cooldown.Skill.E")));
	CooldownTags.Add(FGameplayTag::RequestGameplayTag(FName("Cooldown.Skill.R")));

	for (const FGameplayTag& Tag : CooldownTags)
	{
		ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnCooldownTagChanged);
	}
}

void APGPlayerCharacterBase::OnCooldownTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	OnCooldownTagChangedDelegate.Broadcast(CallbackTag, NewCount);

	if (NewCount > 0)
	{
		StartCooldownTick(CallbackTag);
	}
	else
	{
		StopCooldownTick(CallbackTag);
		OnCooldownTimeChangedDelegate.Broadcast(CallbackTag, 0.f, 0.f);
	}
}

void APGPlayerCharacterBase::StartCooldownTick(FGameplayTag CooldownTag)
{
	if (CooldownTickTimerHandles.Contains(CooldownTag))
	{
		return;
	}

	FTimerHandle Handle;
	CooldownTickTimerHandles.Add(CooldownTag, Handle);

	GetWorldTimerManager().SetTimer(
		CooldownTickTimerHandles[CooldownTag],
		FTimerDelegate::CreateUObject(this, &ThisClass::TickCooldown, CooldownTag),
		0.1f,
		true
	);

	TickCooldown(CooldownTag);
}

void APGPlayerCharacterBase::StopCooldownTick(FGameplayTag CooldownTag)
{
	if (FTimerHandle* Handle = CooldownTickTimerHandles.Find(CooldownTag))
	{
		GetWorldTimerManager().ClearTimer(*Handle);
		CooldownTickTimerHandles.Remove(CooldownTag);
	}
}

void APGPlayerCharacterBase::TickCooldown(FGameplayTag CooldownTag)
{
	float Remaining = 0.f;
	float Duration = 0.f;
	if (GetCooldownRemainingAndDurationByTag(CooldownTag, Remaining, Duration))
	{
		OnCooldownTimeChangedDelegate.Broadcast(CooldownTag, Remaining, Duration);
	}
	else
	{
		StopCooldownTick(CooldownTag);
		OnCooldownTimeChangedDelegate.Broadcast(CooldownTag, 0.f, 0.f);
	}
}

bool APGPlayerCharacterBase::GetCooldownRemainingAndDurationByTag(FGameplayTag CooldownTag, float& OutRemaining, float& OutDuration) const
{
	OutRemaining = 0.f;
	OutDuration = 0.f;

	if (!IsValid(ASC))
	{
		return false;
	}

	FGameplayTagContainer CooldownTags;
	CooldownTags.AddTag(CooldownTag);

	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);
	const TArray<FActiveGameplayEffectHandle> Handles = ASC->GetActiveEffects(Query);
	if (Handles.Num() == 0)
	{
		return false;
	}

	const float Now = GetWorld()->GetTimeSeconds();

	float BestRemaining = 0.f;
	float BestDuration = 0.f;

	for (const FActiveGameplayEffectHandle& Handle : Handles)
	{
		const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
		if (!ActiveGE)
		{
			continue;
		}

		const float Duration = ActiveGE->GetDuration();
		const float Remaining = ActiveGE->GetTimeRemaining(Now);

		if (Remaining > BestRemaining)
		{
			BestRemaining = Remaining;
			BestDuration = Duration;
		}
	}

	OutRemaining = BestRemaining;
	OutDuration = BestDuration;

	return true;
}

void APGPlayerCharacterBase::SetSpawningAbilityLock(bool bLock)
{
	if (bLock)
	{
		ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Spawning")));
		SetInputLock(true);
	}
	else
	{
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Spawning")));
		SetInputLock(false);
	}
}

void APGPlayerCharacterBase::SetInputLock(bool bLock)
{
	bInputLock = bLock;
	if (bInputLock)
	{
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->SetMovementMode(MOVE_None);
	}
	else
	{
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

void APGPlayerCharacterBase::DrawDebugAttackCollision_Implementation(const FColor& DrawColor, FVector TraceStart, FVector TraceEnd, FVector Forward, const FAttackData& AttackData)
{
	FVector Center = TraceStart + (TraceEnd - TraceStart) * 0.5f;

	switch (AttackData.SweepShape)
	{
		case EPGAttackShape::Sphere:
		{
			DrawDebugSphere(
				GetWorld(),
				Center,
				AttackData.Radius,
				16,
				DrawColor,
				false,
				3.0f
			);
		}
		break;

		case EPGAttackShape::Capsule:
		{
			float HalfHeight = AttackData.Range * 0.5f;

			DrawDebugCapsule(
				GetWorld(),
				Center,
				HalfHeight,
				AttackData.Radius,
				FRotationMatrix::MakeFromZ(Forward).ToQuat(),
				DrawColor,
				false,
				3.0f
			);
		}
		break;

		case EPGAttackShape::Box:
		{
			float HalfHeight = 88.f;

			if (ACharacter* Char = Cast<ACharacter>(this))
			{
				if (UCapsuleComponent* Capsule = Char->GetCapsuleComponent())
				{
					HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
				}
			}

			FVector Extent;
			Extent.X = AttackData.Range * 0.5f;
			Extent.Y = AttackData.Radius;
			Extent.Z = HalfHeight;

			DrawDebugBox(
				GetWorld(),
				Center,
				Extent,
				GetActorRotation().Quaternion(),
				DrawColor,
				false,
				3.0f
			);
		}
		break;
	}
}

#pragma region Respawn
void APGPlayerCharacterBase::ServerRPCSetDeadState_Implementation(bool bDead)
{
	SetDeadState(bDead);
}

void APGPlayerCharacterBase::OnRep_Dead()
{
	if (bIsDead == 1) 
	{ // 사망
		if (APGGameModeBase* GM = GetWorld()->GetAuthGameMode<APGGameModeBase>())
		{
			GM->HandleCharacterDeath(this, GetController());
		}

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			DisableInput(PC);
		}

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->StopMovementImmediately();
			MoveComp->DisableMovement();
			MoveComp->GravityScale = 0.0f;
		}

		UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
		if (IsValid(CapsuleComp))
		{
			CapsuleComp->SetCollisionProfileName(TEXT("NoCollision"));
		}

		ClearAllTagsAndEffectOnDeath();
	}
	else { // 리스폰

		UE_LOG(LogTemp, Warning, TEXT("OnRespawn: %s"), *GetNameSafe(this));

		FTransform SpawnTransform = GetRespawnLocationForController();
		MovePlayerToRespawnPoint(SpawnTransform);

		if (USkeletalMeshComponent* SkelMesh = GetMesh())
		{
			SkelMesh->SetVisibility(true, true);
			SkelMesh->SetSimulatePhysics(false);

			GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			GetMesh()->SetRelativeLocation({ 0, 0, -90 });
			GetMesh()->SetRelativeRotation({ 0, -90, 0 });
		}

		if (UCapsuleComponent* Capsule = GetCapsuleComponent())
		{
			Capsule->SetCollisionProfileName(TEXT("Pawn"));
		}

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
			MoveComp->GravityScale = 1.0f;
		}

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			EnableInput(PC);
		}

		ResetCharacterStateOnRespawn();
	}
}

void APGPlayerCharacterBase::SetDeadState(bool bDead)
{
	if (bIsDead == bDead)
	{
		return;
	}

	bIsDead = bDead;

	OnRep_Dead();
}

int32 APGPlayerCharacterBase::GetTeamID_Implementation() const
{
	APGPlayerState* PS = GetPlayerState<APGPlayerState>();
	if (IsValid(PS))
	{
		return PS->GetTeamID();
	}
	return -1;
}

bool APGPlayerCharacterBase::GetIsDead() const
{
	return bIsDead;
}

void APGPlayerCharacterBase::MovePlayerToRespawnPoint(FTransform SpawnTransform = FTransform(FRotator::ZeroRotator, FVector::ZeroVector))
{
	FVector SpawnLocation = SpawnTransform.GetLocation();
	FRotator SpawnRotation = SpawnTransform.GetRotation().Rotator();

	this->TeleportTo(SpawnLocation, SpawnRotation, false, true);
}

FTransform APGPlayerCharacterBase::GetRespawnLocationForController() const
{
	AController* PlayerController = GetController();
	if (!PlayerController) return FTransform(FRotator::ZeroRotator, FVector::ZeroVector);

	APGPlayerState* PS = PlayerController->GetPlayerState<APGPlayerState>();
	if (!PS)
	{
		return FTransform(FRotator::ZeroRotator, FVector::ZeroVector);
	}

	int32 SpawnTeamID = PS->GetTeamID();

	APGGameModeBase* GM = GetWorld()->GetAuthGameMode<APGGameModeBase>();
	if (GM)
	{
		FTransform TeamSpawnTransform = GM->GetTeamSpawnTransform(SpawnTeamID);
		return TeamSpawnTransform;
	}

	return FTransform(FRotator::ZeroRotator, FVector::ZeroVector);
}

void APGPlayerCharacterBase::ResetCharacterStateOnRespawn()
{
	if (!HasAuthority() || !IsValid(CharacterAttributeSet))
	{
		return;
	}

	CharacterAttributeSet->SetHealth(CharacterAttributeSet->GetMaxHealth());
}

void APGPlayerCharacterBase::ClearAllTagsAndEffectOnDeath()
{
	if (!IsValid(ASC))
	{
		return;
	}

	if (HasAuthority())
	{
		ASC->CancelAllAbilities();
	}

	ASC->RemoveAllGameplayCues();

	const FGameplayTag StateRoot = FGameplayTag::RequestGameplayTag(TEXT("Character.State"));

	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);

	for (const FGameplayTag& Tag : OwnedTags)
	{
		if (Tag.MatchesTag(StateRoot))
		{
			ASC->SetLooseGameplayTagCount(Tag, 0);
		}
	}
}

void APGPlayerCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGPlayerCharacterBase, bIsDead);
}

void APGPlayerCharacterBase::OnOutOfHealth(AActor* InstigatorActor)
{
	if (HasAuthority())
	{
		HandlePlayerDeathReward(InstigatorActor);

		APGPlayerState* VictimPS = GetPlayerState<APGPlayerState>();
		if (!IsValid(VictimPS))
		{
			return;
		}

		APGPlayerState* KillerPS = nullptr;
		if (IsValid(InstigatorActor))
		{
			if (APGPlayerCharacterBase* KillerChar = Cast<APGPlayerCharacterBase>(InstigatorActor))
			{
				KillerPS = KillerChar->GetPlayerState<APGPlayerState>();
			}
			else if (APGPlayerController* KillerPC = Cast<APGPlayerController>(InstigatorActor))
			{
				KillerPS = KillerPC->GetPlayerState<APGPlayerState>();
			}
		}

		for (TActorIterator<APGPlayerController> It(GetWorld()); It; ++It)
		{
			APGPlayerController* PC = *It;
			if (IsValid(PC) == true)
			{
				PC->Client_KillInfo(KillerPS, VictimPS);
			}
		}
	}
}

void APGPlayerCharacterBase::HandlePlayerDeathReward(AActor* InstigatorActor)
{
	APGPlayerState* VictimPS = GetPlayerState<APGPlayerState>();
	if (!IsValid(VictimPS))
	{
		return;
	}

	APGPlayerState* KillerPS = nullptr;
	if (IsValid(InstigatorActor))
	{
		if (APGPlayerCharacterBase* KillerChar = Cast<APGPlayerCharacterBase>(InstigatorActor))
		{
			KillerPS = KillerChar->GetPlayerState<APGPlayerState>();
		}
		else if (APGPlayerController* KillerPC = Cast<APGPlayerController>(InstigatorActor))
		{
			KillerPS = KillerPC->GetPlayerState<APGPlayerState>();
		}
	}
	if (IsValid(KillerPS) && KillerPS != VictimPS)
	{
		KillerPS->AddGold(RewardGold);
	}
}

#pragma endregion