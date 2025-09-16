// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HexagonVoxel.generated.h"

static constexpr float HexagonRadius = 50;
static constexpr float HexagonHeight = 100;
static const float     Root3         = FMath::Sqrt( 3.0f );
static const float     Root3Divided2 = Root3 / 2;

static const TArray Directions = { FIntPoint( 1, -1 ), FIntPoint( 1, 0 ), FIntPoint( 0, 1 ), FIntPoint( -1, 1 ), FIntPoint( -1, 0 ), FIntPoint( 0, -1 ) };

UENUM()
enum class EVoxelType
{
	Air,
	Ground,
};

USTRUCT()
struct FHexagonVoxel
{
	GENERATED_BODY()

	FHexagonVoxel() = default;
	FHexagonVoxel( const FIntVector& Coordinate, const EVoxelType VoxelType = EVoxelType::Air );

	FIntVector GridLocation  = FIntVector::ZeroValue;
	FVector    WorldLocation = FVector::ZeroVector;
	EVoxelType Type          = EVoxelType::Air;

	bool operator==( const FHexagonVoxel& Other ) const { return GridLocation == Other.GridLocation; }
};