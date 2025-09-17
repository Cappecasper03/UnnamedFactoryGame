// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "BaseToolComponent.generated.h"

UCLASS( Abstract, ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class UNNAMEDFACTORYGAME_API UBaseToolComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBaseToolComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	virtual void UpdateSize( int32 SizeChange ) {}

protected:
	FHitResult InteractableData;
};
