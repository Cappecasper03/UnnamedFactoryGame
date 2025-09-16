// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"

#include "FactoryPlayer.generated.h"

class UToolManagerComponent;
class AFactoryPlayerController;
class UInputAction;
class UInputMappingContext;
class UCameraComponent;

UCLASS()
class UNNAMEDFACTORYGAME_API AFactoryPlayer : public APawn
{
	GENERATED_BODY()

public:
	AFactoryPlayer();

	static AFactoryPlayer* Get( const UObject* WorldContextObject );

	virtual void SetupPlayerInputComponent( UInputComponent* PlayerInputComponent ) override;

	virtual UPawnMovementComponent* GetMovementComponent() const override { return MovementComponent; }

protected:
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Camera" )
	TObjectPtr< UCameraComponent > CameraComponent;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Context" )
	TObjectPtr< UInputMappingContext > InputMapping;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Actions|Movement" )
	TObjectPtr< UInputAction > MoveForwardBackwardAction;
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Actions|Movement" )
	TObjectPtr< UInputAction > MoveRightLeftAction;
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Actions|Movement" )
	TObjectPtr< UInputAction > MoveUpDownAction;
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Actions|Movement" )
	TObjectPtr< UInputAction > LookRightLeftAction;
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Actions|Movement" )
	TObjectPtr< UInputAction > LookUpDownAction;
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Actions|Movement" )
	TObjectPtr< UInputAction > ChangeSpeedAction;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Actions" )
	TObjectPtr< UInputAction > ToggleInteractiveModeAction;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Actions|Interact" )
	TObjectPtr< UInputAction > SelectInteractableAction;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Movement" )
	TObjectPtr< UPawnMovementComponent > MovementComponent;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Tools" )
	TObjectPtr< UToolManagerComponent > ToolManager;

private:
	void MoveForwardBackwardInput( const FInputActionInstance& Instance );
	void MoveRightLeftInput( const FInputActionInstance& Instance );
	void MoveUpDownInput( const FInputActionInstance& Instance );
	void LookRightLeftInput( const FInputActionInstance& Instance );
	void LookUpDownInput( const FInputActionInstance& Instance );
	void ChangeSpeedInput( const FInputActionInstance& Instance );

	void ToggleInteractiveModeInput();

	void SelectInteractableInput();

	float MouseSensitivity = .45f;
	float SpeedMultiplier  = .5f;

	bool IsInteractiveMode = false;
};
