// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseUnit.h"

#include "Components/BoxComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "UnnamedFactoryGame/World/Pathfinding/NavigationComponent.h"

ABaseUnit::ABaseUnit()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject< USceneComponent >( TEXT( "SceneComponent" ) );

	MeshComponent = CreateDefaultSubobject< UStaticMeshComponent >( TEXT( "MeshComponent" ) );
	MeshComponent->SetupAttachment( RootComponent );

	CollisionComponent = CreateDefaultSubobject< UBoxComponent >( TEXT( "CollisionComponent" ) );
	CollisionComponent->SetupAttachment( MeshComponent );
	CollisionComponent->SetBoxExtent( FVector( 50 ) );

	MovementComponent   = CreateDefaultSubobject< UFloatingPawnMovement >( TEXT( "MovementComponent" ) );
	NavigationComponent = CreateDefaultSubobject< UNavigationComponent >( TEXT( "NavigationComponent" ) );
}

void ABaseUnit::Tick( const float DeltaSeconds )
{
	Super::Tick( DeltaSeconds );

	if( CurrentPath.IsEmpty() )
		return;

	const FVector NewLocation = FMath::VInterpConstantTo( GetActorLocation(), CurrentPath[ 0 ].WorldLocation, DeltaSeconds, 100 );
	SetActorLocation( NewLocation );

	const FVector Difference = NewLocation - CurrentPath[ 0 ].WorldLocation;
	if( Difference.IsNearlyZero() )
		CurrentPath.RemoveAt( 0 );
}

void ABaseUnit::MoveTo( const FVector& Location )
{
	NavigationComponent->CalculatePath( Location, CurrentPath );
}
