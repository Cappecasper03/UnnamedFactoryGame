// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"

#include "CompGeom/PolygonTriangulation.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "WorldPartition/WorldPartition.h"

int   AChunk::StaticSize       = 0;
int   AChunk::StaticHeight     = 0;
float AChunk::StaticNoiseScale = 0;

AChunk::AChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent  = CreateDefaultSubobject< USceneComponent >( FName( "RootComponent" ) );
	ProceduralMesh = CreateDefaultSubobject< UProceduralMeshComponent >( FName( "ProceduralMesh" ) );

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

FVector AChunk::ChunkToWorld( const FIntPoint& ChunkCoordinate ) { return VoxelToWorld( FIntVector3( ChunkCoordinate.X * StaticSize, ChunkCoordinate.Y * StaticSize, 0 ) ); }

FIntVector3 AChunk::WorldToVoxel( const FVector& WorldLocation )
{
	static constexpr float TwoThirds     = 2.f / 3.f;
	static constexpr float NegativeThird = -1.f / 3.f;
	static const float     Root3Divided3 = Root3 / 3;

	const float QFloat = TwoThirds * WorldLocation.X / HexagonRadius;
	const float RFloat = ( NegativeThird * WorldLocation.X + Root3Divided3 * WorldLocation.Y ) / HexagonRadius;

	const float QRounded = FMath::RoundToInt( QFloat );
	const float RRounded = FMath::RoundToInt( RFloat );
	const float SRounded = FMath::RoundToInt( -QFloat - RFloat );

	const float QDiff = FMath::Abs( QRounded - QFloat );
	const float RDiff = FMath::Abs( RRounded - RFloat );
	const float SDiff = FMath::Abs( SRounded - ( -QFloat - RFloat ) );

	int32 FinalQ = FMath::RoundToInt( QFloat );
	int32 FinalR = FMath::RoundToInt( RFloat );

	if( QDiff > RDiff && QDiff > SDiff )
		FinalQ = -FinalR - FMath::RoundToInt( -QFloat - RFloat );
	else if( RDiff > SDiff )
		FinalR = -FinalQ - FMath::RoundToInt( -QFloat - RFloat );

	FIntVector3 Coordinate;
	Coordinate.X = FinalQ;
	Coordinate.Y = FinalR;
	Coordinate.Z = FMath::RoundToInt( WorldLocation.Z / HexagonHeight );
	return Coordinate;
}

FIntPoint AChunk::VoxelToChunk( const FIntVector3& VoxelCoordinate )
{
	const int32 X = FMath::FloorToInt( static_cast< float >( VoxelCoordinate.X ) / StaticSize );
	const int32 Y = FMath::FloorToInt( static_cast< float >( VoxelCoordinate.Y ) / StaticSize );
	return FIntPoint( X, Y );
}

FIntPoint AChunk::WorldToChunk( const FVector& WorldLocation )
{
	const FIntVector3 VoxelCoordinate = WorldToVoxel( WorldLocation );
	return VoxelToChunk( VoxelCoordinate );
}

void AChunk::GenerateVoxels()
{
	HexagonTiles.Empty();
	ProceduralMesh->ClearAllMeshSections();

	AsyncTask( ENamedThreads::AnyBackgroundThreadNormalTask,
	           [ this ]
	           {
				   const int QOffset = Coordinate.X * Size;
				   const int ROffset = Coordinate.Y * Size;

				   TMap< FIntVector3, FHexagonVoxel > HexagonVoxels;
				   for( int Q = 0; Q < Size; ++Q )
				   {
					   for( int R = 0; R < Size; ++R )
					   {
						   const float PerlinNoise = FMath::PerlinNoise2D( FVector2D( Q + QOffset, R + ROffset ) * NoiseScale );
						   const int64 TileHeight  = FMath::RoundToInt( FMath::GetMappedRangeValueClamped( FVector2D( -1.0f, 1.0f ), FVector2D( 0, Height ), PerlinNoise ) );

						   for( int Z = 0; Z < Height; ++Z )
						   {
							   const EVoxelType Type            = Z > TileHeight ? EVoxelType::Air : EVoxelType::Ground;
							   FIntVector3      VoxelCoordinate = { Q + QOffset, R + ROffset, Z };
							   FHexagonVoxel    Voxel( VoxelCoordinate, Type );
							   HexagonVoxels.Add( VoxelCoordinate, Voxel );
						   }
					   }
				   }

				   GenerateMesh( HexagonVoxels );
			   } );
}

void AChunk::GenerateMesh( const TMap< FIntVector3, FHexagonVoxel >& HexagonVoxels )
{
	TArray< FVector >   Vertices;
	TArray< int32 >     Triangles;
	TArray< FVector >   Normals;
	TArray< FVector2D > UVs;

	const TArray Directions = { FIntPoint( 1, -1 ), FIntPoint( 1, 0 ), FIntPoint( 0, 1 ), FIntPoint( -1, 1 ), FIntPoint( -1, 0 ), FIntPoint( 0, -1 ) };

	for( const TPair< FIntVector3, FHexagonVoxel >& HexagonTile: HexagonVoxels )
	{
		const FHexagonVoxel& Voxel           = HexagonTile.Value;
		const FIntVector3&   VoxelCoordinate = Voxel.GridLocation;

		if( Voxel.Type == EVoxelType::Air )
			continue;

		FHexagonVoxel TopVoxel;
		if( !GetVoxel( HexagonVoxels, VoxelCoordinate + FIntVector3( 0, 0, 1 ), TopVoxel ) || TopVoxel.Type == EVoxelType::Air )
			CreateHexagonTop( Voxel, Vertices, Triangles, Normals, UVs );

		FHexagonVoxel BottomVoxel;
		if( !GetVoxel( HexagonVoxels, VoxelCoordinate - FIntVector3( 0, 0, 1 ), BottomVoxel ) || BottomVoxel.Type == EVoxelType::Air )
			CreateHexagonBottom( Voxel, Vertices, Triangles, Normals, UVs );

		for( int i = 0; i < 6; ++i )
		{
			const FIntPoint   Direction      = Directions[ i ];
			const FIntVector3 SideCoordinate = { VoxelCoordinate.X + Direction.X, VoxelCoordinate.Y + Direction.Y, VoxelCoordinate.Z };

			FHexagonVoxel SideVoxel;
			if( !GetVoxel( HexagonVoxels, SideCoordinate, SideVoxel ) || SideVoxel.Type == EVoxelType::Air )
				CreateHexagonSide( Voxel, i, Vertices, Triangles, Normals, UVs );
		}
	}

	AsyncTask( ENamedThreads::GameThread,
	           [ this, HexagonVoxels, Vertices, Triangles, Normals, UVs ]
	           {
				   HexagonTiles = HexagonVoxels;
				   if( Vertices.IsEmpty() )
					   return;

				   const TArray< FLinearColor >     VertexColors;
				   const TArray< FProcMeshTangent > Tangents;
				   ProceduralMesh->CreateMeshSection_LinearColor( 0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, false );
			   } );
}

void AChunk::CreateHexagonTop( const FHexagonVoxel& Voxel,
                               TArray< FVector >&   OutVertices,
                               TArray< int32 >&     OutTriangles,
                               TArray< FVector >&   OutNormals,
                               TArray< FVector2D >& OutUVs ) const
{
	const int32   Index     = OutVertices.Num();
	const FVector TopCenter = Voxel.WorldLocation + FVector( 0, 0, HexagonHeight );

	OutVertices.Add( TopCenter );
	OutUVs.Add( FVector2D( .5f, .5f ) );

	for( int i = 0; i < 6; ++i )
	{
		const float Angle = FMath::DegreesToRadians( 60 * i );
		OutVertices.Add( TopCenter + FVector( FMath::Cos( Angle ), FMath::Sin( Angle ), 0 ) * HexagonRadius );

		OutTriangles.Add( Index );
		OutTriangles.Add( Index + 1 + ( i + 1 ) % 6 );
		OutTriangles.Add( Index + 1 + i );

		OutUVs.Add( FVector2D( FMath::Cos( Angle ) * .5f + .5f, FMath::Sin( Angle ) * .5f + .5f ) );
	}

	OutNormals.AddUninitialized( 7 );
	for( int i = 0; i < 7; ++i )
		OutNormals[ Index + i ] = FVector::UpVector;
}

void AChunk::CreateHexagonBottom( const FHexagonVoxel& Voxel,
                                  TArray< FVector >&   OutVertices,
                                  TArray< int32 >&     OutTriangles,
                                  TArray< FVector >&   OutNormals,
                                  TArray< FVector2D >& OutUVs ) const
{
	const int32   Index        = OutVertices.Num();
	const FVector BottomCenter = Voxel.WorldLocation;

	OutVertices.Add( BottomCenter );
	OutUVs.Add( FVector2D( .5f, .5f ) );

	for( int i = 0; i < 6; ++i )
	{
		const float Angle = FMath::DegreesToRadians( 60 * i );
		OutVertices.Add( BottomCenter + FVector( FMath::Cos( Angle ), FMath::Sin( Angle ), 0 ) * HexagonRadius );

		OutTriangles.Add( Index );
		OutTriangles.Add( Index + 1 + i );
		OutTriangles.Add( Index + 1 + ( i + 1 ) % 6 );

		OutUVs.Add( FVector2D( FMath::Cos( Angle ) * .5f + .5f, FMath::Sin( Angle ) * .5f + .5f ) );
	}

	OutNormals.AddUninitialized( 7 );
	for( int i = 0; i < 7; ++i )
		OutNormals[ Index + i ] = FVector::DownVector;
}

void AChunk::CreateHexagonSide( const FHexagonVoxel& Voxel,
                                const int32          SideIndex,
                                TArray< FVector >&   OutVertices,
                                TArray< int32 >&     OutTriangles,
                                TArray< FVector >&   OutNormals,
                                TArray< FVector2D >& OutUVs ) const
{
	const int32   Index        = OutVertices.Num();
	const FVector BottomCenter = Voxel.WorldLocation;
	const FVector TopCenter    = BottomCenter + FVector( 0, 0, HexagonHeight );

	const float   Angle1      = FMath::DegreesToRadians( 60 * SideIndex - 60 );
	const float   Angle2      = FMath::DegreesToRadians( 60 * ( SideIndex + 1 ) - 60 );
	const FVector BottomLeft  = BottomCenter + FVector( FMath::Cos( Angle1 ), FMath::Sin( Angle1 ), 0 ) * HexagonRadius;
	const FVector BottomRight = BottomCenter + FVector( FMath::Cos( Angle2 ), FMath::Sin( Angle2 ), 0 ) * HexagonRadius;
	const FVector TopLeft     = TopCenter + FVector( FMath::Cos( Angle1 ), FMath::Sin( Angle1 ), 0 ) * HexagonRadius;
	const FVector TopRight    = TopCenter + FVector( FMath::Cos( Angle2 ), FMath::Sin( Angle2 ), 0 ) * HexagonRadius;

	OutVertices.Add( BottomLeft );
	OutVertices.Add( BottomRight );
	OutVertices.Add( TopLeft );
	OutVertices.Add( TopRight );

	OutTriangles.Add( Index );
	OutTriangles.Add( Index + 2 );
	OutTriangles.Add( Index + 1 );

	OutTriangles.Add( Index + 1 );
	OutTriangles.Add( Index + 2 );
	OutTriangles.Add( Index + 3 );

	const float   QuadAngle        = FMath::DegreesToRadians( 60 * SideIndex - 30 );
	const FVector QuadBottomCenter = BottomCenter + FVector( FMath::Cos( QuadAngle ), FMath::Sin( QuadAngle ), 0 ) * HexagonRadius;
	const FVector Normal           = ( QuadBottomCenter - BottomCenter ).GetSafeNormal2D();

	for( int i = 0; i < 4; ++i )
		OutNormals.Add( Normal );

	OutUVs.Add( FVector2D( 0, 1 ) );
	OutUVs.Add( FVector2D( 1, 1 ) );
	OutUVs.Add( FVector2D( 0, 0 ) );
	OutUVs.Add( FVector2D( 1, 0 ) );
}

bool AChunk::GetVoxel( const TMap< FIntVector3, FHexagonVoxel >& Map, const FIntVector3& VoxelCoordinate, FHexagonVoxel& OutVoxel )
{
	const FHexagonVoxel* Voxel = Map.Find( VoxelCoordinate );
	if( !Voxel )
		return false;

	OutVoxel = *Voxel;
	return true;
}