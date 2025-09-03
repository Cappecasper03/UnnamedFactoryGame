// Fill out your copyright notice in the Description page of Project Settings.

#include "SelectableComponent.h"

USelectableComponent::USelectableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BoxExtent = FVector( 50 );
}
