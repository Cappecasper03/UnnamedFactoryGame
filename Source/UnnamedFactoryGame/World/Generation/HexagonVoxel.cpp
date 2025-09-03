// Fill out your copyright notice in the Description page of Project Settings.

#include "HexagonVoxel.h"

FHexagonVoxel::FHexagonVoxel( const FIntVector3& Coordinate, const EVoxelType VoxelType )
{
	GridLocation = Coordinate;

	WorldLocation.X = HexagonRadius * ( 1.5f * Coordinate.X );
	WorldLocation.Y = HexagonRadius * ( Root3 * Coordinate.Y + Root3Divided2 * Coordinate.X );
	WorldLocation.Z = HexagonHeight * Coordinate.Z;

	Type = VoxelType;
}