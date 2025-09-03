// Fill out your copyright notice in the Description page of Project Settings.

#include "FactoryPlayerController.h"

#include "Kismet/GameplayStatics.h"

AFactoryPlayerController* AFactoryPlayerController::Get( const UObject* WorldContextObject )
{
	if( !IsValid( WorldContextObject ) )
		return nullptr;

	return Cast< AFactoryPlayerController >( UGameplayStatics::GetPlayerController( WorldContextObject, 0 ) );
}