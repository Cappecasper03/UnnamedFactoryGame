// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "ToolManagerComponent.generated.h"

class UBaseToolComponent;

UENUM( BlueprintType )
enum class EToolType : uint8
{
	None,
	Mining,
};

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class UNNAMEDFACTORYGAME_API UToolManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UToolManagerComponent();

	UFUNCTION( BlueprintCallable )
	void ToggleTool( EToolType ToolType );

	UFUNCTION( BlueprintCallable )
	void ActivateTool( EToolType ToolType );

	UFUNCTION( BlueprintCallable )
	void DeactivateTool();

protected:
	virtual void OnRegister() override;

private:
	UPROPERTY()
	TMap< EToolType, TObjectPtr< UBaseToolComponent > > Tools;

	UPROPERTY()
	TObjectPtr< UBaseToolComponent > ActiveTool     = nullptr;
	EToolType                        ActiveToolType = EToolType::None;
};
