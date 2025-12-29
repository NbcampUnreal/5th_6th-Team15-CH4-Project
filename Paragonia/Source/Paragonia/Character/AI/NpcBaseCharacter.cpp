// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/NpcBaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/CharacterAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "PlayerState/PGPlayerState.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/PG_PlayerUIComponent.h"

ANpcBaseCharacter::ANpcBaseCharacter()
	:TeamId(255),
	MaterialCounts(0),
	bIsDead(false),
	DeathAccumulatedTime(0.0f),
	DeathDuration(5.0f)
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SetReplicateMovement(true);

	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	GetCharacterMovement()->bUseRVOAvoidance = true;
	GetCharacterMovement()->AvoidanceConsiderationRadius = 150.0f;

	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
	StateTreeComponent->SetAutoActivate(true);

	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ANpcBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (TeamId != 255)
	{
		APlayerController* LocalPC = UGameplayStatics::GetPlayerController(this, 0);
		if (LocalPC == nullptr)
		{
			return;
		}

		APGPlayerState* PS = LocalPC->GetPlayerState<APGPlayerState>();
		if (!IsValid(PS))
		{
			return;
		}

		if (IsValid(UIComponent))
		{
			UIComponent->SetHPBarColor(TeamId != PS->GetTeamID());
		}
	}
}

void ANpcBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (StateTreeComponent)
	{
		StateTreeComponent->StartLogic();
	}

	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);

		if (CharacterAttributeSet)
		{
			CharacterAttributeSet->OnHealthChanged.AddDynamic(this, &ANpcBaseCharacter::OnHealthChanged);

			CharacterAttributeSet->OutOfHealthChanged.AddDynamic(this, &ANpcBaseCharacter::OnOutOfHealth);
		}

		if (HasAuthority())
		{
			GrantStartupAbilities();
		}
	}
}

void ANpcBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANpcBaseCharacter, TeamId);
	DOREPLIFETIME(ANpcBaseCharacter, bIsDead);
}

void ANpcBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead)
	{
		DeathAccumulatedTime += DeltaTime;

		float Alpha = FMath::Clamp(DeathAccumulatedTime / DeathDuration, 0.0f, 1.0f);

		for (UMaterialInstanceDynamic* MID : DissolveMaterials)
		{
			if (MID != nullptr)
			{
				MID->SetScalarParameterValue(FName("FadeOut"), Alpha);
			}
		}

		if (DeathAccumulatedTime >= DeathDuration)
		{
			bIsDead = false;
		}
	}
}

void ANpcBaseCharacter::HandleDeath(AActor* KillerActor)
{
	if (HasAuthority())
	{
		bool bAlreadyDead = ASC->HasMatchingGameplayTag(DeadTag);
		if (bAlreadyDead == true ||
			bIsDead == true)
		{
			return;
		}

		if (IsValid(ASC) == true
			&& DeadTag.IsValid() == true)
		{
			ASC->AddLooseGameplayTag(DeadTag);
		}

		if (StateTreeComponent)
		{
			StateTreeComponent->SendStateTreeEvent(FStateTreeEvent(DeadTag));
		}

		if (KillerActor)
		{
			APawn* KillerPawn = Cast<APawn>(KillerActor);
			if (KillerPawn)
			{
				if (APGPlayerState* KillerPS = KillerPawn->GetPlayerState<APGPlayerState>())
				{
					KillerPS->AddGold(RewardGoldAmount);

					UE_LOG(LogTemp, Log, TEXT("Npc killed by %s. Gave %d Gold."), *KillerPS->GetPlayerName(), RewardGoldAmount);
				}
			}
		}
	}
}

void ANpcBaseCharacter::StartDeathEffect()
{
	if (HasAuthority())
	{
		Multicast_HandleDeath();
	}
}

void ANpcBaseCharacter::Multicast_HandleDeath_Implementation()
{
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("NoCollision"));

		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

		GetCapsuleComponent()->SetEnableGravity(false);
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		const int32 MaterialCount = MeshComp->GetNumMaterials();
		for (int32 i = 0; i < MaterialCount; ++i)
		{
			UMaterialInstanceDynamic* MID = MeshComp->CreateAndSetMaterialInstanceDynamic(i);
			if (MID != nullptr)
			{
				DissolveMaterials.Add(MID);
			}
		}
	}

	bIsDead = true;
	DeathAccumulatedTime = 0.0f;

	SetActorTickEnabled(true);

	if (HasAuthority())
	{
		SetLifeSpan(DeathDuration + 0.1f);
		SetAttackTarget(nullptr);
	}
}

int32 ANpcBaseCharacter::GetTeamID_Implementation() const
{
	return TeamId;
}

bool ANpcBaseCharacter::GetIsDead_Implementation() const
{
	return bIsDead;
}

void ANpcBaseCharacter::SetTeamId(int32 NewTeamId)
{
	if (HasAuthority())
	{
		TeamId = NewTeamId;

		OnRep_TeamId();
	}
}

void ANpcBaseCharacter::SetAttackTarget(AActor* NewTarget)
{
	if (HasAuthority())
	{
		CurrentAttackTarget = NewTarget;
	}
}

AActor* ANpcBaseCharacter::GetAttackTarget() const
{
	return CurrentAttackTarget.Get();
}

bool ANpcBaseCharacter::CanAttack() const
{
	if (CurrentAttackTarget.IsValid() == false)
	{
		return false;
	}

	float DistSq = GetSquaredDistanceTo(CurrentAttackTarget.Get());
	float AttackRangeSq = AttackRange * AttackRange;

	return DistSq <= AttackRangeSq;
}

void ANpcBaseCharacter::SetAttackRange(float InRange)
{
	if (HasAuthority())
	{
		AttackRange = InRange;
	}
}

void ANpcBaseCharacter::SetSightRange(float InRange)
{
	if (HasAuthority())
	{
		SightRange = InRange;
	}
}

void ANpcBaseCharacter::SetRotationToTarget(AActor* TargetActor)
{
	if (IsValid(TargetActor) == false)
	{
		return;
	}

	FVector MyLoc = GetActorLocation();
	FVector TargetLoc = TargetActor->GetActorLocation();

	MyLoc.Z = 0.0f;
	TargetLoc.Z = 0.0f;

	FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(MyLoc, TargetLoc);

	SetActorRotation(FRotator(0.0f, LookAtRot.Yaw, 0.0f));
}

void ANpcBaseCharacter::GrantStartupAbilities()
{
	if (IsValid(ASC) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("ANpcBaseCharacter::GrantStartupAbilities - ASC is Not Valid!"));
		return;
	}

	if (StartupAbilities.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ANpcBaseCharacter::GrantStartupAbilities - StartupAbilities is Empty!"));
		return;
	}

	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec Spec(AbilityClass, 1);
		ASC->GiveAbility(Spec);
	}
}

void ANpcBaseCharacter::OnHealthChanged(float OldValue, float NewValue)
{
	/*if (NewValue <= 0.0f &&
		OldValue > 0.0f)
	{
		HandleDeath();
	}*/
}

void ANpcBaseCharacter::OnRep_TeamId()
{
	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(this, 0);
	if (LocalPC == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ANpcBaseCharacter::OnRep_TeamId - LocalPC is Null?!"));
		return;
	}

	USkeletalMesh* MeshToUse = nullptr;

	int32 MyTeamId = 255;
	if (APGPlayerState* PS = LocalPC->GetPlayerState<APGPlayerState>())
	{
		if (IsValid(PS))
		{
			MyTeamId = PS->GetTeamID();
		}
	}
	if (IsValid(UIComponent))
	{
		UIComponent->SetHPBarColor(TeamId != MyTeamId);
	}

	if (TeamId == MyTeamId)
	{
		MeshToUse = AllyMesh;
	}
	else
	{
		MeshToUse = EnemyMesh;
	}

	if (MeshToUse && GetMesh())
	{
		// ABP 를 공유하기에 SkeltalMesh 에셋만 교체
		if (GetMesh()->GetSkeletalMeshAsset() != MeshToUse)
		{
			GetMesh()->SetSkeletalMeshAsset(MeshToUse);
		}
	}
}

void ANpcBaseCharacter::OnOutOfHealth(AActor* InstigatorActor)
{
	HandleDeath(InstigatorActor);
}