// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "UnnamedFactoryGame/World/Generation/HexagonVoxel.h"

#include "NavigationComponent.generated.h"

USTRUCT()
struct FVoxelNode
{
	GENERATED_BODY()

	float       Cost       = 0;
	FIntVector3 Coordinate = FIntVector3::ZeroValue;

	float CalculateFutureCost( const FVoxelNode& Other ) const;

	bool operator<( const FVoxelNode& Other ) const { return Cost < Other.Cost; }
	bool operator==( const FVoxelNode& Other ) const { return Coordinate == Other.Coordinate; }
};

inline int32 GetTypeHash( const FVoxelNode& Node ) { return HashCombine( HashCombine( Node.Coordinate.X, Node.Coordinate.Y ), Node.Coordinate.Z ); }

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class UNNAMEDFACTORYGAME_API UNavigationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNavigationComponent();

	bool CalculatePath( const FVector& TargetLocation, TArray< FHexagonVoxel >& OutPath ) const;

private:
	TArray< FHexagonVoxel > ReconstructPath( const TMap< FIntVector3, FIntVector3 >& ParentMap, FIntVector3& CurrentNode ) const;
};
