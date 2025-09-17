// Fill out your copyright notice in the Description page of Project Settings.

#include "HexagonVoxel.h"

FHexagonVoxel::FHexagonVoxel( const FIntVector& Coordinate, const EVoxelType VoxelType )
{
	GridLocation = Coordinate;

	WorldLocation.X = HexagonRadius * ( 1.5f * Coordinate.X );
	WorldLocation.Y = HexagonRadius * ( Root3 * Coordinate.Y + Root3Divided2 * Coordinate.X );
	WorldLocation.Z = HexagonHeight * Coordinate.Z;

	Type = VoxelType;
}

FIntVector FHexagonVoxel::WorldToVoxel( const FVector& WorldLocation )
{
	static constexpr float TwoThirds     = 2.f / 3.f;
	static constexpr float NegativeThird = -1.f / 3.f;
	static const float     Root3Divided3 = Root3 / 3;

	const float QFloat = TwoThirds * WorldLocation.X / HexagonRadius;
	const float RFloat = ( NegativeThird * WorldLocation.X + Root3Divided3 * WorldLocation.Y ) / HexagonRadius;
	const float SFloat = -QFloat - RFloat;

	const float QRounded = FMath::RoundToInt( QFloat );
	const float RRounded = FMath::RoundToInt( RFloat );
	const float SRounded = FMath::RoundToInt( SFloat );

	const float QDiff = FMath::Abs( QRounded - QFloat );
	const float RDiff = FMath::Abs( RRounded - RFloat );
	const float SDiff = FMath::Abs( SRounded - SFloat );

	int32 FinalQ = QRounded;
	int32 FinalR = RRounded;

	if( QDiff > RDiff && QDiff > SDiff )
		FinalQ = -FinalR - SRounded;
	else if( RDiff > SDiff )
		FinalR = -FinalQ - SRounded;

	FIntVector Coordinate;
	Coordinate.X = FinalQ;
	Coordinate.Y = FinalR;
	Coordinate.Z = FMath::FloorToInt( WorldLocation.Z / HexagonHeight );
	return Coordinate;
}

bool FHexagonVoxel::GetVoxel( const TMap< FIntVector, FHexagonVoxel >& Map, const FIntVector& VoxelCoordinate, FHexagonVoxel& OutVoxel )
{
	const FHexagonVoxel* Voxel = Map.Find( VoxelCoordinate );
	if( !Voxel )
		return false;

	OutVoxel = *Voxel;
	return true;
}

bool FHexagonVoxel::GetVoxel( const TMap< FIntVector, FHexagonVoxel >& Map, const FVector& WorldLocation, FHexagonVoxel& OutVoxel )
{
	return GetVoxel( Map, WorldToVoxel( WorldLocation ), OutVoxel );
}