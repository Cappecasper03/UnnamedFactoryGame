// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"

#include "CompGeom/PolygonTriangulation.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "ProceduralHexagonMeshComponent.h"
#include "WorldPartition/WorldPartition.h"

int32 AChunk::StaticSize       = 0;
int32 AChunk::StaticHeight     = 0;
float AChunk::StaticNoiseScale = 0;

AChunk::AChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject< USceneComponent >( TEXT( "RootComponent" ) );
	Mesh          = CreateDefaultSubobject< UProceduralHexagonMeshComponent >( TEXT( "Mesh" ) );

	StaticSize       = Size;
	StaticHeight     = Height;
	StaticNoiseScale = NoiseScale;
}

void AChunk::Generate( const FIntPoint& ChunkCoordinate )
{
	Coordinate = ChunkCoordinate;

	GenerateVoxels();
}

void AChunk::SetVisible()
{
	Mesh->SetVisibility( true );
	GetWorld()->GetTimerManager().SetTimer( VisibilityTimer, [ this ] { Mesh->SetVisibility( false ); }, 1, false );
	GetWorld()->GetTimerManager().SetTimer( LoadedTimer, [ this ] { Destroy(); }, 30, false );
}

FVector AChunk::ChunkToWorld( const FIntPoint& ChunkCoordinate )
{
	return FHexagonVoxel::VoxelToWorld( FIntVector( ChunkCoordinate.X * StaticSize, ChunkCoordinate.Y * StaticSize, 0 ) );
}

FIntPoint AChunk::VoxelToChunk( const FIntVector& VoxelCoordinate )
{
	const int32 X = FMath::FloorToInt( static_cast< float >( VoxelCoordinate.X ) / StaticSize );
	const int32 Y = FMath::FloorToInt( static_cast< float >( VoxelCoordinate.Y ) / StaticSize );
	return FIntPoint( X, Y );
}

FIntPoint AChunk::WorldToChunk( const FVector& WorldLocation )
{
	const FIntVector VoxelCoordinate = FHexagonVoxel::WorldToVoxel( WorldLocation );
	return VoxelToChunk( VoxelCoordinate );
}

void AChunk::GenerateVoxels()
{
	HexagonTiles.Empty();

	AsyncTask( ENamedThreads::AnyBackgroundThreadNormalTask,
	           [ this ]
	           {
				   const int32 QOffset = Coordinate.X * Size;
				   const int32 ROffset = Coordinate.Y * Size;

				   TMap< FIntVector, FHexagonVoxel > HexagonVoxels;
				   for( int32 Q = -1; Q <= Size; ++Q )
				   {
					   for( int32 R = -1; R <= Size; ++R )
					   {
						   const float PerlinNoise = FMath::PerlinNoise2D( FVector2D( Q + QOffset, R + ROffset ) * NoiseScale );
						   const int64 TileHeight  = FMath::RoundToInt( FMath::GetMappedRangeValueClamped( FVector2D( -1.0f, 1.0f ), FVector2D( 0, Height ), PerlinNoise ) );

						   for( int32 Z = 0; Z < Height; ++Z )
						   {
							   const EVoxelType Type            = Z > TileHeight ? EVoxelType::Air : EVoxelType::Ground;
							   FIntVector       VoxelCoordinate = { Q + QOffset, R + ROffset, Z };
							   FHexagonVoxel    Voxel( VoxelCoordinate, Type );
							   HexagonVoxels.Add( VoxelCoordinate, Voxel );
						   }
					   }
				   }

				   static FSkipGenerationDelegate SkipGenerationDelegate;
				   SkipGenerationDelegate.BindLambda(
					   [ this, QOffset, ROffset ]( const FHexagonVoxel& Voxel )
					   {
						   const FIntVector& VoxelCoordinate = Voxel.GridLocation;
						   return VoxelCoordinate.X == QOffset - 1 || VoxelCoordinate.X == QOffset + Size || VoxelCoordinate.Y == ROffset - 1
			                   || VoxelCoordinate.Y == ROffset + Size;
					   } );

				   Mesh->Generate( HexagonVoxels, true, SkipGenerationDelegate );
			   } );
}
