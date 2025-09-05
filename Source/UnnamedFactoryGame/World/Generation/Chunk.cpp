// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"

#include "CompGeom/PolygonTriangulation.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "WorldPartition/WorldPartition.h"

const TArray Directions = { FIntPoint( 1, -1 ), FIntPoint( 1, 0 ), FIntPoint( 0, 1 ), FIntPoint( -1, 1 ), FIntPoint( -1, 0 ), FIntPoint( 0, -1 ) };

int   AChunk::StaticSize       = 0;
int   AChunk::StaticHeight     = 0;
float AChunk::StaticNoiseScale = 0;

struct FHexagonVertexKey
{
	TArray< FIntPoint > Coordinates;

	FHexagonVertexKey() { Coordinates.AddZeroed( 3 ); }
	FHexagonVertexKey( const FIntPoint& A, const FIntPoint& B, const FIntPoint& C )
	{
		Coordinates.Add( A );
		Coordinates.Add( B );
		Coordinates.Add( C );
		Coordinates.Sort( []( const FIntPoint& Left, const FIntPoint& Right ) { return Left.X < Right.X || ( Left.X == Right.X && Left.Y < Right.Y ); } );
	}

	bool operator==( const FHexagonVertexKey& Other ) const { return Coordinates == Other.Coordinates; }

	friend uint32 GetTypeHash( const FHexagonVertexKey& Key )
	{
		return HashCombine( HashCombine( GetTypeHash( Key.Coordinates[ 0 ] ), GetTypeHash( Key.Coordinates[ 1 ] ) ), GetTypeHash( Key.Coordinates[ 2 ] ) );
	}
};

AChunk::AChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent  = CreateDefaultSubobject< USceneComponent >( FName( "RootComponent" ) );
	ProceduralMesh = CreateDefaultSubobject< UProceduralMeshComponent >( FName( "ProceduralMesh" ) );

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

				   GenerateMesh( HexagonVoxels );
			   } );
}

void AChunk::GenerateMesh( const TMap< FIntVector, FHexagonVoxel >& HexagonVoxels )
{
	const int QOffset = Coordinate.X * Size;
	const int ROffset = Coordinate.Y * Size;

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

	TArray< FVector >   Vertices;
	TArray< int32 >     Triangles;
	TArray< FVector >   Normals;
	TArray< FVector2D > UVs;

	GenerateMeshRegions( TopVisibleVoxels, true, Vertices, Triangles, Normals, UVs );
	GenerateMeshRegions( BottomVisibleVoxels, false, Vertices, Triangles, Normals, UVs );
	GenerateMeshRegions( SideVisibleVoxels, Vertices, Triangles, Normals, UVs );

	AsyncTask( ENamedThreads::GameThread,
	           [ this, HexagonVoxels, Vertices, Triangles, Normals, UVs ]
	           {
				   HexagonTiles = HexagonVoxels;
				   if( Vertices.IsEmpty() )
					   return;

				   const TArray< FLinearColor >     VertexColors;
				   const TArray< FProcMeshTangent > Tangents;
				   ProceduralMesh->CreateMeshSection_LinearColor( 0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true );
			   } );
}

void AChunk::GenerateMeshRegions( TMap< int32, TArray< FIntPoint > >& VisibleVoxelCoordinates,
                                  const bool                          IsTop,
                                  TArray< FVector >&                  OutVertices,
                                  TArray< int32 >&                    OutTriangles,
                                  TArray< FVector >&                  OutNormals,
                                  TArray< FVector2D >&                OutUVs ) const
{
	for( TPair< int32, TArray< FIntPoint > >& VoxelPlane: VisibleVoxelCoordinates )
	{
		TArray< FIntPoint >& Coordinates = VoxelPlane.Value;

		while( !Coordinates.IsEmpty() )
		{
			TArray< FIntPoint > Region;
			TArray< FIntPoint > Queue;

			const FIntPoint Start = *Coordinates.begin();
			Coordinates.RemoveAt( 0 );
			Region.Add( Start );
			Queue.Add( Start );

			while( !Queue.IsEmpty() )
			{
				const FIntPoint Current = Queue.Pop();

				for( const FIntPoint& Direction: Directions )
				{
					FIntPoint Neighbor = Current + Direction;
					if( !Coordinates.Contains( Neighbor ) )
						continue;

					Coordinates.Remove( Neighbor );
					Region.Add( Neighbor );
					Queue.Add( Neighbor );
				}
			}

			GenerateMeshPolygon( VoxelPlane.Key, Region, IsTop, OutVertices, OutTriangles, OutNormals, OutUVs );
		}
	}
}

void AChunk::GenerateMeshRegions( TMap< FIntVector, TArray< int32 > >& VisibleVoxelCoordinates,
                                  TArray< FVector >&                   OutVertices,
                                  TArray< int32 >&                     OutTriangles,
                                  TArray< FVector >&                   OutNormals,
                                  TArray< FVector2D >&                 OutUVs ) const
{
	for( TPair< FIntVector, TArray< int32 > >& VoxelPlane: VisibleVoxelCoordinates )
	{
		TArray< int32 >& Heights = VoxelPlane.Value;

		while( !Heights.IsEmpty() )
		{
			TArray< int32 > Region;
			TArray< int32 > Queue;

			const int32 Start = *Heights.begin();
			Heights.RemoveAt( 0 );
			Region.Add( Start );
			Queue.Add( Start );

			while( !Queue.IsEmpty() )
			{
				const int32 Current = Queue.Pop();

				for( const int32 Direction: { 1, -1 } )
				{
					int32 Neighbor = Current + Direction;
					if( !Heights.Contains( Neighbor ) )
						continue;

					Heights.Remove( Neighbor );
					Region.Add( Neighbor );
					Queue.Add( Neighbor );
				}
			}

			GenerateMeshPolygon( VoxelPlane.Key, Region, OutVertices, OutTriangles, OutNormals, OutUVs );
		}
	}
}

void AChunk::GenerateMeshPolygon( const int32          PolygonHeight,
                                  TArray< FIntPoint >& Region,
                                  const bool           IsTop,
                                  TArray< FVector >&   OutVertices,
                                  TArray< int32 >&     OutTriangles,
                                  TArray< FVector >&   OutNormals,
                                  TArray< FVector2D >& OutUVs ) const
{
	if( Region.IsEmpty() )
		return;

	TMap< FHexagonVertexKey, FHexagonVertexKey > EdgeConnections;
	TMap< FHexagonVertexKey, FVector >           KeyToLocation;

	for( const FIntPoint& VoxelCoordinate: Region )
	{
		for( int i = 0; i < 6; ++i )
		{
			const FIntPoint& Neighbor = VoxelCoordinate + Directions[ i ];
			if( Region.Contains( Neighbor ) )
				continue;

			const FIntPoint& ThirdHexagon1 = VoxelCoordinate + Directions[ ( i + 5 ) % 6 ];
			const FIntPoint& ThirdHexagon2 = VoxelCoordinate + Directions[ ( i + 1 ) % 6 ];

			const FHexagonVertexKey Key1( VoxelCoordinate, Neighbor, ThirdHexagon1 );
			const FHexagonVertexKey Key2( VoxelCoordinate, Neighbor, ThirdHexagon2 );

			EdgeConnections.Add( Key1, Key2 );

			const FVector Center1  = FHexagonVoxel( FIntVector( VoxelCoordinate.X, VoxelCoordinate.Y, PolygonHeight + IsTop ) ).WorldLocation;
			const FVector Center2  = FHexagonVoxel( FIntVector( Neighbor.X, Neighbor.Y, PolygonHeight + IsTop ) ).WorldLocation;
			const FVector Center31 = FHexagonVoxel( FIntVector( ThirdHexagon1.X, ThirdHexagon1.Y, PolygonHeight + IsTop ) ).WorldLocation;
			const FVector Center32 = FHexagonVoxel( FIntVector( ThirdHexagon2.X, ThirdHexagon2.Y, PolygonHeight + IsTop ) ).WorldLocation;

			const FVector Corner1 = ( Center1 + Center2 + Center31 ) / 3;
			const FVector Corner2 = ( Center1 + Center2 + Center32 ) / 3;

			KeyToLocation.FindOrAdd( Key1, Corner1 );
			KeyToLocation.FindOrAdd( Key2, Corner2 );
		}
	}

	if( EdgeConnections.IsEmpty() )
		return;

	TArray< TArray< FVector > > Polygons;
	while( !EdgeConnections.IsEmpty() )
	{
		TArray< FVector >       Loop;
		const FHexagonVertexKey Start   = EdgeConnections.begin().Key();
		FHexagonVertexKey       Current = Start;

		while( EdgeConnections.Contains( Current ) )
		{
			Loop.Add( KeyToLocation.FindChecked( Current ) );
			Current = EdgeConnections.FindAndRemoveChecked( Current );
		}

		Polygons.Add( Loop );
	}

	if( Polygons.IsEmpty() )
		return;

	int32 OuterLoopIndex = -1;
	float MaxArea        = 0;
	for( int i = 0; i < Polygons.Num(); ++i )
	{
		const float Area = FMath::Abs( Signed2DPolygonArea( Polygons[ i ] ) );
		if( Area < MaxArea )
			continue;

		MaxArea        = Area;
		OuterLoopIndex = i;
	}

	TArray< FVector > MainPolygon = Polygons[ OuterLoopIndex ];
	Polygons.RemoveAt( OuterLoopIndex );

	if( Signed2DPolygonArea( MainPolygon ) < 0 )
		Algo::Reverse( MainPolygon );

	for( TArray< FVector >& InnerPolygon: Polygons )
	{
		if( Signed2DPolygonArea( InnerPolygon ) > 0 )
			Algo::Reverse( InnerPolygon );

		int32 MainBridgeIndex  = -1;
		int32 InnerBridgeIndex = -1;
		float MinDistance      = TNumericLimits< float >::Max();

		for( int i = 0; i < MainPolygon.Num(); ++i )
		{
			for( int j = 0; j < InnerPolygon.Num(); ++j )
			{
				const float Distance = FVector::DistSquared( MainPolygon[ i ], InnerPolygon[ j ] );
				if( Distance > MinDistance )
					continue;

				MinDistance      = Distance;
				MainBridgeIndex  = i;
				InnerBridgeIndex = j;
			}
		}

		TArray< FVector > NewOuterPolygon;
		for( int i = 0; i <= MainBridgeIndex; ++i )
			NewOuterPolygon.Add( MainPolygon[ i ] );

		for( int i = 0; i <= InnerPolygon.Num(); ++i )
			NewOuterPolygon.Add( InnerPolygon[ ( InnerBridgeIndex + i ) % InnerPolygon.Num() ] );

		for( int i = MainBridgeIndex; i < MainPolygon.Num(); ++i )
			NewOuterPolygon.Add( MainPolygon[ i ] );

		MainPolygon = NewOuterPolygon;
	}

	TArray< UE::Geometry::FIndex3i > Triangles;
	PolygonTriangulation::TriangulateSimplePolygon( MainPolygon, Triangles );

	const int32   Index = OutVertices.Num();
	const FVector Normal( 0, 0, IsTop ? 1 : -1 );
	for( const FVector& Vertex: MainPolygon )
	{
		OutVertices.Add( Vertex );
		OutNormals.Add( Normal );
		OutUVs.AddZeroed();
	}

	for( const UE::Geometry::FIndex3i& Triangle: Triangles )
	{
		OutTriangles.Add( Index + Triangle.A );
		OutTriangles.Add( Index + ( IsTop ? Triangle.B : Triangle.C ) );
		OutTriangles.Add( Index + ( IsTop ? Triangle.C : Triangle.B ) );
	}
}

void AChunk::GenerateMeshPolygon( const FIntVector&    PolygonCoordinate,
                                  TArray< int32 >&     Region,
                                  TArray< FVector >&   OutVertices,
                                  TArray< int32 >&     OutTriangles,
                                  TArray< FVector >&   OutNormals,
                                  TArray< FVector2D >& OutUVs ) const
{
	if( Region.IsEmpty() )
		return;

	Region.Sort();
	const FVector TopCenter    = FHexagonVoxel( FIntVector( PolygonCoordinate.X, PolygonCoordinate.Y, Region.Last() ) ).WorldLocation + FVector( 0, 0, HexagonHeight );
	const FVector BottomCenter = FHexagonVoxel( FIntVector( PolygonCoordinate.X, PolygonCoordinate.Y, Region[ 0 ] ) ).WorldLocation;

	const int32 Side   = PolygonCoordinate.Z;
	const float Angle1 = FMath::DegreesToRadians( 60 * Side - 60 );
	const float Angle2 = FMath::DegreesToRadians( 60 * ( Side + 1 ) - 60 );

	const FVector BottomLeft  = BottomCenter + FVector( FMath::Cos( Angle1 ), FMath::Sin( Angle1 ), 0 ) * HexagonRadius;
	const FVector BottomRight = BottomCenter + FVector( FMath::Cos( Angle2 ), FMath::Sin( Angle2 ), 0 ) * HexagonRadius;
	const FVector TopLeft     = TopCenter + FVector( FMath::Cos( Angle1 ), FMath::Sin( Angle1 ), 0 ) * HexagonRadius;
	const FVector TopRight    = TopCenter + FVector( FMath::Cos( Angle2 ), FMath::Sin( Angle2 ), 0 ) * HexagonRadius;

	const int32 Index = OutVertices.Num();
	OutVertices.Add( TopLeft );
	OutVertices.Add( TopRight );
	OutVertices.Add( BottomLeft );
	OutVertices.Add( BottomRight );

	OutTriangles.Add( Index );
	OutTriangles.Add( Index + 1 );
	OutTriangles.Add( Index + 2 );

	OutTriangles.Add( Index + 1 );
	OutTriangles.Add( Index + 3 );
	OutTriangles.Add( Index + 2 );

	const float   QuadAngle        = FMath::DegreesToRadians( 60 * Side - 30 );
	const FVector QuadBottomCenter = BottomCenter + FVector( FMath::Cos( QuadAngle ), FMath::Sin( QuadAngle ), 0 ) * HexagonRadius;
	const FVector Normal           = ( QuadBottomCenter - BottomCenter ).GetSafeNormal2D();

	for( int i = 0; i < 4; ++i )
		OutNormals.Add( Normal );

	OutUVs.Add( FVector2D( 0, 1 ) );
	OutUVs.Add( FVector2D( 1, 1 ) );
	OutUVs.Add( FVector2D( 0, 0 ) );
	OutUVs.Add( FVector2D( 1, 0 ) );
}

float AChunk::Signed2DPolygonArea( const TArray< FVector >& Polygon ) const
{
	float Area = 0;
	for( int j = 0; j < Polygon.Num(); ++j )
	{
		const FVector& Point1  = Polygon[ j ];
		const FVector& Point2  = Polygon[ ( j + 1 ) % Polygon.Num() ];
		Area                  += Point1.X * Point2.Y - Point2.X * Point1.Y;
	}

	return Area;
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