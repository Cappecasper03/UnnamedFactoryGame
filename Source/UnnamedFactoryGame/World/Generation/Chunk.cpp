// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"

#include "CompGeom/PolygonTriangulation.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "UnnamedFactoryGame/WorldGeneration/ProceduralHexagonMeshComponent.h"
#include "WorldPartition/WorldPartition.h"

int   AChunk::StaticSize       = 0;
int   AChunk::StaticHeight     = 0;
float AChunk::StaticNoiseScale = 0;

AChunk::AChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent  = CreateDefaultSubobject< USceneComponent >( FName( "RootComponent" ) );
	ProceduralMesh = CreateDefaultSubobject< UProceduralHexagonMeshComponent >( FName( "ProceduralMesh" ) );

	static ConstructorHelpers::FObjectFinder< UMaterial > MaterialFinder( TEXT( "/Game/Art/Materials/HexagonVoxel_WorldAligned_M" ) );
	if( MaterialFinder.Succeeded() )
		ProceduralMesh->SetMaterial( 0, MaterialFinder.Object );

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
	ProceduralMesh->SetVisibility( true );
	GetWorld()->GetTimerManager().SetTimer( VisibilityTimer, [ this ] { ProceduralMesh->SetVisibility( false ); }, 1, false );
}

FVector AChunk::ChunkToWorld( const FIntPoint& ChunkCoordinate ) { return VoxelToWorld( FIntVector( ChunkCoordinate.X * StaticSize, ChunkCoordinate.Y * StaticSize, 0 ) ); }

FIntVector AChunk::WorldToVoxel( const FVector& WorldLocation )
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
	Coordinate.Z = FMath::RoundToInt( WorldLocation.Z / HexagonHeight );
	return Coordinate;
}

FIntPoint AChunk::VoxelToChunk( const FIntVector& VoxelCoordinate )
{
	const int32 X = FMath::FloorToInt( static_cast< float >( VoxelCoordinate.X ) / StaticSize );
	const int32 Y = FMath::FloorToInt( static_cast< float >( VoxelCoordinate.Y ) / StaticSize );
	return FIntPoint( X, Y );
}

FIntPoint AChunk::WorldToChunk( const FVector& WorldLocation )
{
	const FIntVector VoxelCoordinate = WorldToVoxel( WorldLocation );
	return VoxelToChunk( VoxelCoordinate );
}

void AChunk::GenerateVoxels()
{
	HexagonTiles.Empty();

	AsyncTask( ENamedThreads::AnyBackgroundThreadNormalTask,
	           [ this ]
	           {
				   const int QOffset = Coordinate.X * Size;
				   const int ROffset = Coordinate.Y * Size;

				   TMap< FIntVector, FHexagonVoxel > HexagonVoxels;
				   for( int Q = -1; Q <= Size; ++Q )
				   {
					   for( int R = -1; R <= Size; ++R )
					   {
						   const float PerlinNoise = FMath::PerlinNoise2D( FVector2D( Q + QOffset, R + ROffset ) * NoiseScale );
						   const int64 TileHeight  = FMath::RoundToInt( FMath::GetMappedRangeValueClamped( FVector2D( -1.0f, 1.0f ), FVector2D( 0, Height ), PerlinNoise ) );

						   for( int Z = 0; Z < Height; ++Z )
						   {
							   const EVoxelType Type            = Z > TileHeight ? EVoxelType::Air : EVoxelType::Ground;
							   FIntVector       VoxelCoordinate = { Q + QOffset, R + ROffset, Z };
							   FHexagonVoxel    Voxel( VoxelCoordinate, Type );
							   HexagonVoxels.Add( VoxelCoordinate, Voxel );
						   }
					   }
				   }

				   TMap< int32, TArray< FIntPoint > >  TopVisibleVoxels;
				   TMap< int32, TArray< FIntPoint > >  BottomVisibleVoxels;
				   TMap< FIntVector, TArray< int32 > > SideVisibleVoxels;

				   for( const TPair< FIntVector, FHexagonVoxel >& HexagonTile: HexagonVoxels )
				   {
					   const FHexagonVoxel& Voxel           = HexagonTile.Value;
					   const FIntVector&    VoxelCoordinate = Voxel.GridLocation;

					   if( Voxel.Type == EVoxelType::Air )
						   continue;

					   if( VoxelCoordinate.X == QOffset - 1 || VoxelCoordinate.X == QOffset + Size || VoxelCoordinate.Y == ROffset - 1 || VoxelCoordinate.Y == ROffset + Size )
						   continue;

					   FHexagonVoxel TopVoxel;
					   if( !GetVoxel( HexagonVoxels, VoxelCoordinate + FIntVector( 0, 0, 1 ), TopVoxel ) || TopVoxel.Type == EVoxelType::Air )
						   TopVisibleVoxels.FindOrAdd( VoxelCoordinate.Z ).Add( FIntPoint( VoxelCoordinate.X, VoxelCoordinate.Y ) );

					   FHexagonVoxel BottomVoxel;
					   if( !GetVoxel( HexagonVoxels, VoxelCoordinate - FIntVector( 0, 0, 1 ), BottomVoxel ) || BottomVoxel.Type == EVoxelType::Air )
						   BottomVisibleVoxels.FindOrAdd( VoxelCoordinate.Z ).Add( FIntPoint( VoxelCoordinate.X, VoxelCoordinate.Y ) );

					   for( int i = 0; i < 6; ++i )
					   {
						   const FIntPoint  Direction      = Directions[ i ];
						   const FIntVector SideCoordinate = { VoxelCoordinate.X + Direction.X, VoxelCoordinate.Y + Direction.Y, VoxelCoordinate.Z };

						   FHexagonVoxel SideVoxel;
						   if( !GetVoxel( HexagonVoxels, SideCoordinate, SideVoxel ) || SideVoxel.Type == EVoxelType::Air )
							   SideVisibleVoxels.FindOrAdd( FIntVector( VoxelCoordinate.X, VoxelCoordinate.Y, i ) ).Add( VoxelCoordinate.Z );
					   }
				   }

				   ProceduralMesh->Generate( TopVisibleVoxels, BottomVisibleVoxels, SideVisibleVoxels );
			   } );
}

bool AChunk::GetVoxel( const TMap< FIntVector, FHexagonVoxel >& Map, const FIntVector& VoxelCoordinate, FHexagonVoxel& OutVoxel )
{
	const FHexagonVoxel* Voxel = Map.Find( VoxelCoordinate );
	if( !Voxel )
		return false;

	OutVoxel = *Voxel;
	return true;
}

bool AChunk::GetVoxel( const TMap< FIntVector, FHexagonVoxel >& Map, const FVector& WorldLocation, FHexagonVoxel& OutVoxel )
{
	return GetVoxel( Map, WorldToVoxel( WorldLocation ), OutVoxel );
}