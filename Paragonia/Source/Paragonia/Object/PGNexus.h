// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PGObject.h"
#include "PGNexus.generated.h"

class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNexusDestroyed, AActor*, DestroyedActor);

/**
 * 
 */
UCLASS()
class PARAGONIA_API APGNexus : public APGObject
{
	GENERATED_BODY()

public:
	APGNexus();

	int32 GetNexusTeamID() const { return TeamID; }

protected:
	virtual void BeginPlay() override;
	
	virtual void Destroyed() override;

public:
    UPROPERTY(BlueprintAssignable)
    FOnNexusDestroyed OnNexusDestroyed;
};
