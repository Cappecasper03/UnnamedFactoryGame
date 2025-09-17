// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralHexagonMeshComponent.h"

#include "CompGeom/PolygonTriangulation.h"
#include "IndexTypes.h"
#include "UnnamedFactoryGame/World/Generation/Chunk.h"

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

void UProceduralHexagonMeshComponent::Generate( const TMap< FIntVector, FHexagonVoxel >& HexagonVoxels,
                                                const bool                               GenerateCollision,
                                                FSkipGenerationDelegate                  SkipGenerationDelegate )
{
	TMap< int32, TArray< FIntPoint > >  TopVisibleVoxels;
	TMap< int32, TArray< FIntPoint > >  BottomVisibleVoxels;
	TMap< FIntVector, TArray< int32 > > SideVisibleVoxels;

	for( const TPair< FIntVector, FHexagonVoxel >& HexagonTile: HexagonVoxels )
	{
		const FHexagonVoxel& Voxel           = HexagonTile.Value;
		const FIntVector&    VoxelCoordinate = Voxel.GridLocation;

		if( Voxel.Type == EVoxelType::Air )
			continue;

		if( SkipGenerationDelegate.IsBound() && SkipGenerationDelegate.Execute( Voxel ) )
			continue;

		FHexagonVoxel TopVoxel;
		if( !FHexagonVoxel::GetVoxel( HexagonVoxels, VoxelCoordinate + FIntVector( 0, 0, 1 ), TopVoxel ) || TopVoxel.Type == EVoxelType::Air )
			TopVisibleVoxels.FindOrAdd( VoxelCoordinate.Z ).Add( FIntPoint( VoxelCoordinate.X, VoxelCoordinate.Y ) );

		FHexagonVoxel BottomVoxel;
		if( !FHexagonVoxel::GetVoxel( HexagonVoxels, VoxelCoordinate - FIntVector( 0, 0, 1 ), BottomVoxel ) || BottomVoxel.Type == EVoxelType::Air )
			BottomVisibleVoxels.FindOrAdd( VoxelCoordinate.Z ).Add( FIntPoint( VoxelCoordinate.X, VoxelCoordinate.Y ) );

		for( int32 i = 0; i < 6; ++i )
		{
			const FIntPoint  Direction      = CoordinateDirections[ i ];
			const FIntVector SideCoordinate = { VoxelCoordinate.X + Direction.X, VoxelCoordinate.Y + Direction.Y, VoxelCoordinate.Z };

			FHexagonVoxel SideVoxel;
			if( !FHexagonVoxel::GetVoxel( HexagonVoxels, SideCoordinate, SideVoxel ) || SideVoxel.Type == EVoxelType::Air )
				SideVisibleVoxels.FindOrAdd( FIntVector( VoxelCoordinate.X, VoxelCoordinate.Y, i ) ).Add( VoxelCoordinate.Z );
		}
	}

	TArray< FVector >   Vertices;
	TArray< int32 >     Triangles;
	TArray< FVector >   Normals;
	TArray< FVector2D > UVs;

	GenerateRegions( TopVisibleVoxels, true, Vertices, Triangles, Normals, UVs );
	GenerateRegions( BottomVisibleVoxels, false, Vertices, Triangles, Normals, UVs );
	GenerateRegions( SideVisibleVoxels, Vertices, Triangles, Normals, UVs );

	AsyncTask( ENamedThreads::GameThread,
	           [ this, Vertices, Triangles, Normals, UVs, GenerateCollision ]
	           {
				   if( Vertices.IsEmpty() )
					   return;

				   const TArray< FLinearColor >     VertexColors;
				   const TArray< FProcMeshTangent > Tangents;
				   CreateMeshSection_LinearColor( 0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, GenerateCollision );
			   } );
}

void UProceduralHexagonMeshComponent::GenerateRegions( TMap< int32, TArray< FIntPoint > >& VisibleVoxelCoordinates,
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

				for( const FIntPoint& Direction: CoordinateDirections )
				{
					FIntPoint Neighbor = Current + Direction;
					if( !Coordinates.Contains( Neighbor ) )
						continue;

					Coordinates.Remove( Neighbor );
					Region.Add( Neighbor );
					Queue.Add( Neighbor );
				}
			}

			GeneratePolygon( VoxelPlane.Key, Region, IsTop, OutVertices, OutTriangles, OutNormals, OutUVs );
		}
	}
}

void UProceduralHexagonMeshComponent::GenerateRegions( TMap< FIntVector, TArray< int32 > >& VisibleVoxelCoordinates,
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

			GeneratePolygon( VoxelPlane.Key, Region, OutVertices, OutTriangles, OutNormals, OutUVs );
		}
	}
}

void UProceduralHexagonMeshComponent::GeneratePolygon( const int32          PolygonHeight,
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
		for( int32 i = 0; i < 6; ++i )
		{
			const FIntPoint& Neighbor = VoxelCoordinate + CoordinateDirections[ i ];
			if( Region.Contains( Neighbor ) )
				continue;

			const FIntPoint& ThirdHexagon1 = VoxelCoordinate + CoordinateDirections[ ( i + 5 ) % 6 ];
			const FIntPoint& ThirdHexagon2 = VoxelCoordinate + CoordinateDirections[ ( i + 1 ) % 6 ];

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
	for( int32 i = 0; i < Polygons.Num(); ++i )
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
		if( Signed2DPolygonArea( InnerPolygon ) < 0 )
			Algo::Reverse( InnerPolygon );

		int32 MainBridgeIndex  = -1;
		int32 InnerBridgeIndex = -1;
		float MinDistance      = TNumericLimits< float >::Max();

		for( int32 i = 0; i < MainPolygon.Num(); ++i )
		{
			for( int32 j = 0; j < InnerPolygon.Num(); ++j )
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
		for( int32 i = 0; i <= MainBridgeIndex; ++i )
			NewOuterPolygon.Add( MainPolygon[ i ] );

		for( int32 i = 0; i <= InnerPolygon.Num(); ++i )
			NewOuterPolygon.Add( InnerPolygon[ ( InnerBridgeIndex + i ) % InnerPolygon.Num() ] );

		for( int32 i = MainBridgeIndex; i < MainPolygon.Num(); ++i )
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

void UProceduralHexagonMeshComponent::GeneratePolygon( const FIntVector&    PolygonCoordinate,
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

	for( int32 i = 0; i < 4; ++i )
		OutNormals.Add( Normal );

	OutUVs.Add( FVector2D( 0, 1 ) );
	OutUVs.Add( FVector2D( 1, 1 ) );
	OutUVs.Add( FVector2D( 0, 0 ) );
	OutUVs.Add( FVector2D( 1, 0 ) );
}

float UProceduralHexagonMeshComponent::Signed2DPolygonArea( const TArray< FVector >& Polygon ) const
{
	float Area = 0;
	for( int32 j = 0; j < Polygon.Num(); ++j )
	{
		const FVector& Point1  = Polygon[ j ];
		const FVector& Point2  = Polygon[ ( j + 1 ) % Polygon.Num() ];
		Area                  += Point1.X * Point2.Y - Point2.X * Point1.Y;
	}

	return Area;
}