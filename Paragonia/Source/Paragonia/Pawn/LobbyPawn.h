// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LobbyPawn.generated.h"

UCLASS()
class PARAGONIA_API ALobbyPawn : public APawn
{
	GENERATED_BODY()

public:
	ALobbyPawn();

	void StartMoveToTarget();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
	FVector TargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
	FRotator StartRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
	FRotator TargetRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
	float MoveSpeed = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
	float RotationInterpSpeed = 5.f;
protected:
	virtual void Tick(float DeltaSeconds) override;

private:
	bool bIsMovingToTarget = false;
	float ArriveTolerance = 20.f;   // 오차 허용
};
