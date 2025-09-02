// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"

#include "FactoryPlayer.generated.h"

class UInputAction;
class UInputMappingContext;
class UCameraComponent;

UCLASS()
class UNNAMEDFACTORYGAME_API AFactoryPlayer : public APawn
{
	GENERATED_BODY()

public:
	AFactoryPlayer();

	virtual void Tick( float DeltaSeconds ) override;

	virtual void SetupPlayerInputComponent( UInputComponent* PlayerInputComponent ) override;

	virtual UPawnMovementComponent* GetMovementComponent() const override { return MovementComponent; }

	static AFactoryPlayer* Get( const UObject* WorldContextObject );

protected:
	void MoveForwardBackwardInput( const FInputActionInstance& Instance );
	void MoveRightLeftInput( const FInputActionInstance& Instance );
	void MoveUpDownInput( const FInputActionInstance& Instance );
	void LookRightLeftInput( const FInputActionInstance& Instance );
	void LookUpDownInput( const FInputActionInstance& Instance );
	void ChangeSpeedInput( const FInputActionInstance& Instance );

	UPROPERTY( EditDefaultsOnly, Category = "Camera" )
	TObjectPtr< UCameraComponent > CameraComponent;

	UPROPERTY( EditDefaultsOnly, Category = "Input|Context" )
	TObjectPtr< UInputMappingContext > InputMapping;

	UPROPERTY( EditDefaultsOnly, Category = "Input|Actions" )
	TObjectPtr< UInputAction > MoveForwardBackwardAction;
	UPROPERTY( EditDefaultsOnly, Category = "Input|Actions" )
	TObjectPtr< UInputAction > MoveRightLeftAction;
	UPROPERTY( EditDefaultsOnly, Category = "Input|Actions" )
	TObjectPtr< UInputAction > MoveUpDownAction;
	UPROPERTY( EditDefaultsOnly, Category = "Input|Actions" )
	TObjectPtr< UInputAction > LookRightLeftAction;
	UPROPERTY( EditDefaultsOnly, Category = "Input|Actions" )
	TObjectPtr< UInputAction > LookUpDownAction;
	UPROPERTY( EditDefaultsOnly, Category = "Input|Actions" )
	TObjectPtr< UInputAction > ChangeSpeedAction;

	UPROPERTY( EditDefaultsOnly, Category = "Input|Sensitivity" )
	float MouseSensitivity = .45f;

	UPROPERTY( EditDefaultsOnly, Category = "Movement" )
	TObjectPtr< UPawnMovementComponent > MovementComponent;

	float SpeedMultiplier = .2f;
};
