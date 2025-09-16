// Fill out your copyright notice in the Description page of Project Settings.

#include "ToolManagerComponent.h"

#include "MiningToolComponent.h"

UToolManagerComponent::UToolManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UToolManagerComponent::ToggleTool( const EToolType ToolType )
{
	if( ToolType == EToolType::None )
		return;

	if( ActiveToolType == ToolType )
		DeactivateTool();
	else
		ActivateTool( ToolType );
}

void UToolManagerComponent::ActivateTool( const EToolType ToolType )
{
	if( ActiveToolType != ToolType )
		DeactivateTool();

	if( ToolType == EToolType::None )
		return;

	const TObjectPtr< UBaseToolComponent >* Tool = Tools.Find( ToolType );
	if( !Tool )
		return;

	( *Tool )->Activate();
	ActiveTool     = *Tool;
	ActiveToolType = ToolType;
}

void UToolManagerComponent::DeactivateTool()
{
	if( !IsValid( ActiveTool ) )
		return;

	ActiveTool->Deactivate();
	ActiveTool     = nullptr;
	ActiveToolType = EToolType::None;
}
void UToolManagerComponent::OnRegister()
{
	Super::OnRegister();

	AActor* Owner = GetOwner();
	if( !IsValid( Owner ) )
		return;

	UMiningToolComponent* MiningTool = NewObject< UMiningToolComponent >( Owner );
	if( MiningTool )
	{
		MiningTool->RegisterComponent();
		Tools.Add( EToolType::Mining, MiningTool );
	}
}
