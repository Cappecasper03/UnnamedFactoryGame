// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FactoryPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class UNNAMEDFACTORYGAME_API AFactoryPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	static AFactoryPlayerController* Get( const UObject* WorldContextObject );
};
