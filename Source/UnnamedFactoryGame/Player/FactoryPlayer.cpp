// Fill out your copyright notice in the Description page of Project Settings.

#include "FactoryPlayer.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FactoryPlayerController.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "UnnamedFactoryGame/UnnamedFactoryGame.h"

AFactoryPlayer::AFactoryPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	MovementComponent = CreateDefaultSubobject< UPawnMovementComponent, UFloatingPawnMovement >( FName( "MovementComponent" ) );
}

void AFactoryPlayer::Tick( const float DeltaSeconds ) { Super::Tick( DeltaSeconds ); }

void AFactoryPlayer::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent )
{
	Super::SetupPlayerInputComponent( PlayerInputComponent );

	if( !IsValid( InputMapping ) )
	{
		UE_LOG( UnnamedFactoryGameLog, Error, TEXT( "Input Mapping Context not set" ) )
		return;
	}

	const AFactoryPlayerController* PlayerController = Cast< AFactoryPlayerController >( Controller );
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

	if( Value == 0 || !GetController() )
		return;

	FRotator const ControlSpaceRot = GetController()->GetControlRotation();

	AddMovementInput( FRotationMatrix( ControlSpaceRot ).GetScaledAxis( EAxis::X ), Value );
}

void AFactoryPlayer::MoveRightLeftInput( const FInputActionInstance& Instance )
{
	const float Value = Instance.GetValue().Get< float >();

	if( Value == 0 || !GetController() )
		return;

	FRotator const ControlSpaceRot = GetController()->GetControlRotation();

	AddMovementInput( FRotationMatrix( ControlSpaceRot ).GetScaledAxis( EAxis::Y ), Value );
}

void AFactoryPlayer::MoveUpDownInput( const FInputActionInstance& Instance )
{
	const float Value = Instance.GetValue().Get< float >();

	if( Value == 0 )
		return;

	AddMovementInput( FVector::UpVector, Value );
}

void AFactoryPlayer::LookRightLeftInput( const FInputActionInstance& Instance )
{
	const float Value = Instance.GetValue().Get< float >();

	if( Value == 0 )
		return;

	AddControllerYawInput( Value * MouseSensitivity );
}

void AFactoryPlayer::LookUpDownInput( const FInputActionInstance& Instance )
{
	const float Value = Instance.GetValue().Get< float >();

	if( Value == 0 )
		return;

	AddControllerPitchInput( Value * MouseSensitivity );
}
