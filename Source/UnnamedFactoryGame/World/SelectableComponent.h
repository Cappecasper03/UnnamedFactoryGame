// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/BoxComponent.h"
#include "CoreMinimal.h"

#include "SelectableComponent.generated.h"

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class UNNAMEDFACTORYGAME_API USelectableComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	USelectableComponent();
};
