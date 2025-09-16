// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseToolComponent.h"

#include "UnnamedFactoryGame/Player/FactoryPlayer.h"
#include "UnnamedFactoryGame/Player/FactoryPlayerController.h"

UBaseToolComponent::UBaseToolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBaseToolComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled( false );
	SetActiveFlag( false );
}

void UBaseToolComponent::TickComponent( const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	const FVector                   PlayerLocation   = AFactoryPlayer::Get( this )->GetActorLocation();
	const AFactoryPlayerController* PlayerController = AFactoryPlayerController::Get( this );

	FVector Location;
	FVector Direction;
	PlayerController->DeprojectMousePositionToWorld( Location, Direction );

	GetWorld()->LineTraceSingleByChannel( InteractableData, PlayerLocation, PlayerLocation + Direction * 5000, ECC_Visibility );
	DrawDebugSphere( GetWorld(), InteractableData.ImpactPoint, 10, 10, FColor::Red, false );
}
