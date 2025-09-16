// Fill out your copyright notice in the Description page of Project Settings.

#include "FactoryPlayer.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FactoryPlayerController.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Tools/ToolManagerComponent.h"
#include "UnnamedFactoryGame/UnnamedFactoryGame.h"
#include "UnnamedFactoryGame/World/Generation/Chunk.h"

AFactoryPlayer::AFactoryPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject< UCameraComponent >( TEXT( "CameraComponent" ) );
	RootComponent   = CameraComponent;

	MovementComponent = CreateDefaultSubobject< UPawnMovementComponent, UFloatingPawnMovement >( TEXT( "MovementComponent" ) );

	ToolManager = CreateDefaultSubobject< UToolManagerComponent >( TEXT( "ToolManager" ) );
}

void AFactoryPlayer::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent )
{
	Super::SetupPlayerInputComponent( PlayerInputComponent );

	if( !IsValid( InputMapping ) )
	{
		UE_LOG( UnnamedFactoryGameLog, Error, TEXT( "Input Mapping Context not set" ) )
		return;
	}

	const AFactoryPlayerController* PlayerController = AFactoryPlayerController::Get( this );
	if( !IsValid( PlayerController ) )
		return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem >( PlayerController->GetLocalPlayer() );
	if( !IsValid( Subsystem ) )
		return;

	Subsystem->AddMappingContext( InputMapping, 0 );

	UEnhancedInputComponent* Input = Cast< UEnhancedInputComponent >( PlayerInputComponent );
	if( !IsValid( Input ) )
		return;

	Input->BindAction( MoveForwardBackwardAction, ETriggerEvent::Triggered, this, &AFactoryPlayer::MoveForwardBackwardInput );
	Input->BindAction( MoveRightLeftAction, ETriggerEvent::Triggered, this, &AFactoryPlayer::MoveRightLeftInput );
	Input->BindAction( MoveUpDownAction, ETriggerEvent::Triggered, this, &AFactoryPlayer::MoveUpDownInput );
	Input->BindAction( LookRightLeftAction, ETriggerEvent::Triggered, this, &AFactoryPlayer::LookRightLeftInput );
	Input->BindAction( LookUpDownAction, ETriggerEvent::Triggered, this, &AFactoryPlayer::LookUpDownInput );
	Input->BindAction( ChangeSpeedAction, ETriggerEvent::Triggered, this, &AFactoryPlayer::ChangeSpeedInput );

	Input->BindAction( ToggleInteractiveModeAction, ETriggerEvent::Triggered, this, &AFactoryPlayer::ToggleInteractiveModeInput );

	Input->BindAction( SelectInteractableAction, ETriggerEvent::Triggered, this, &AFactoryPlayer::SelectInteractableInput );
}

AFactoryPlayer* AFactoryPlayer::Get( const UObject* WorldContextObject )
{
	if( !IsValid( WorldContextObject ) )
		return nullptr;

	return Cast< AFactoryPlayer >( UGameplayStatics::GetPlayerPawn( WorldContextObject->GetWorld(), 0 ) );
}

void AFactoryPlayer::MoveForwardBackwardInput( const FInputActionInstance& Instance )
{
	const float Value = Instance.GetValue().Get< float >();

	if( !Controller )
		return;

	FRotator const ControlSpaceRot = Controller->GetControlRotation();
	FVector        Direction       = FRotationMatrix( ControlSpaceRot ).GetScaledAxis( EAxis::X );

	Direction.Z = 0;
	Direction.Normalize();

	AddMovementInput( Direction, Value * SpeedMultiplier );
}

void AFactoryPlayer::MoveRightLeftInput( const FInputActionInstance& Instance )
{
	const float Value = Instance.GetValue().Get< float >();

	if( !Controller )
		return;

	FRotator const ControlSpaceRot = Controller->GetControlRotation();

	AddMovementInput( FRotationMatrix( ControlSpaceRot ).GetScaledAxis( EAxis::Y ), Value * SpeedMultiplier );
}

void AFactoryPlayer::MoveUpDownInput( const FInputActionInstance& Instance )
{
	const float Value = Instance.GetValue().Get< float >();

	AddMovementInput( FVector::UpVector, Value * SpeedMultiplier );
}

void AFactoryPlayer::LookRightLeftInput( const FInputActionInstance& Instance )
{
	if( IsInteractiveMode )
		return;

	const float Value = Instance.GetValue().Get< float >();

	AddControllerYawInput( Value * MouseSensitivity );
}

void AFactoryPlayer::LookUpDownInput( const FInputActionInstance& Instance )
{
	if( IsInteractiveMode )
		return;

	const float Value = Instance.GetValue().Get< float >();

	AddControllerPitchInput( Value * MouseSensitivity );
}

void AFactoryPlayer::ChangeSpeedInput( const FInputActionInstance& Instance )
{
	const float Value = Instance.GetValue().Get< float >();

	if( IsInteractiveMode ) {}
	else
	{
		SpeedMultiplier += Value / 10;
		SpeedMultiplier  = FMath::Clamp( SpeedMultiplier, .1f, 1 );
	}
}

void AFactoryPlayer::ToggleInteractiveModeInput()
{
	IsInteractiveMode = !IsInteractiveMode;

	AFactoryPlayerController* PlayerController = AFactoryPlayerController::Get( this );
	if( !IsValid( PlayerController ) )
		return;

	PlayerController->SetShowMouseCursor( IsInteractiveMode );

	int32 ViewportSizeX, ViewportSizeY;
	PlayerController->GetViewportSize( ViewportSizeX, ViewportSizeY );
	PlayerController->SetMouseLocation( ViewportSizeX / 2, ViewportSizeY / 2 );

	if( IsInteractiveMode )
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture( false );
		InputMode.SetLockMouseToViewportBehavior( EMouseLockMode::DoNotLock );
		PlayerController->SetInputMode( InputMode );
	}
	else
	{
		PlayerController->SetInputMode( FInputModeGameOnly() );
		ToolManager->DeactivateTool();
	}
}

void AFactoryPlayer::SelectInteractableInput()
{
	if( !IsInteractiveMode )
		return;
}
