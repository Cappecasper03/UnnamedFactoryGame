// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HexagonVoxel.generated.h"

static constexpr float HexagonRadius = 50;
static constexpr float HexagonHeight = 100;
static const float     Root3         = FMath::Sqrt( 3.0f );
static const float     Root3Divided2 = Root3 / 2;

static const TArray CoordinateDirections = { FIntPoint( 1, -1 ), FIntPoint( 1, 0 ), FIntPoint( 0, 1 ), FIntPoint( -1, 1 ), FIntPoint( -1, 0 ), FIntPoint( 0, -1 ) };

static const TArray HexagonDirections = {
	FIntVector( 1, -1, 0 ), FIntVector( 1, 0, 0 ),  FIntVector( 0, 1, 0 ), FIntVector( -1, 1, 0 ),
	FIntVector( -1, 0, 0 ), FIntVector( 0, -1, 0 ), FIntVector( 0, 0, 1 ), FIntVector( 0, 0, -1 ),
};

UENUM()
enum class EVoxelType : uint8
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

	static FVector    VoxelToWorld( const FIntVector& VoxelCoordinate ) { return FHexagonVoxel( VoxelCoordinate ).WorldLocation; }
	static FIntVector WorldToVoxel( const FVector& WorldLocation );

	static bool GetVoxel( const TMap< FIntVector, FHexagonVoxel >& Map, const FIntVector& VoxelCoordinate, FHexagonVoxel& OutVoxel );
	static bool GetVoxel( const TMap< FIntVector, FHexagonVoxel >& Map, const FVector& WorldLocation, FHexagonVoxel& OutVoxel );
};