// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexagonVoxel.h"

#include "Chunk.generated.h"

class UProceduralMeshComponent;
class UHierarchicalInstancedStaticMeshComponent;

UCLASS( Abstract )
class UNNAMEDFACTORYGAME_API AChunk : public AActor
{
	GENERATED_BODY()

public:
	AChunk();

	void Generate( const FIntPoint& ChunkCoordinate );

	void SetVisible();

	bool GetVoxel( const FIntVector& VoxelCoordinate, FHexagonVoxel& OutVoxel ) const { return GetVoxel( HexagonTiles, VoxelCoordinate, OutVoxel ); }
	bool GetVoxel( const FVector& WorldLocation, FHexagonVoxel& OutVoxel ) const { return GetVoxel( HexagonTiles, WorldLocation, OutVoxel ); }

	static FVector VoxelToWorld( const FIntVector& VoxelCoordinate ) { return FHexagonVoxel( VoxelCoordinate ).WorldLocation; }
	static FVector ChunkToWorld( const FIntPoint& ChunkCoordinate );

	static FIntVector WorldToVoxel( const FVector& WorldLocation );
	static FIntPoint  VoxelToChunk( const FIntVector& VoxelCoordinate );
	static FIntPoint  WorldToChunk( const FVector& WorldLocation );

private:
	void GenerateVoxels();
	void GenerateMesh( const TMap< FIntVector, FHexagonVoxel >& HexagonVoxels );

	void GenerateMeshRegions( TMap< int32, TArray< FIntPoint > >& VisibleVoxelCoordinates,
	                          bool                                IsTop,
	                          TArray< FVector >&                  OutVertices,
	                          TArray< int32 >&                    OutTriangles,
	                          TArray< FVector >&                  OutNormals ) const;
	void GenerateMeshRegions( TMap< FIntVector, TArray< int32 > >& VisibleVoxelCoordinates,
	                          TArray< FVector >&                   OutVertices,
	                          TArray< int32 >&                     OutTriangles,
	                          TArray< FVector >&                   OutNormals ) const;

	void GenerateMeshPolygon( int32                PolygonHeight,
	                          TArray< FIntPoint >& Region,
	                          bool                 IsTop,
	                          TArray< FVector >&   OutVertices,
	                          TArray< int32 >&     OutTriangles,
	                          TArray< FVector >&   OutNormals ) const;
	void GenerateMeshPolygon( const FIntVector&  PolygonCoordinate,
	                          TArray< int32 >&   Region,
	                          TArray< FVector >& OutVertices,
	                          TArray< int32 >&   OutTriangles,
	                          TArray< FVector >& OutNormals ) const;

	float Signed2DPolygonArea( const TArray< FVector >& Polygon ) const;

	static bool GetVoxel( const TMap< FIntVector, FHexagonVoxel >& Map, const FIntVector& VoxelCoordinate, FHexagonVoxel& OutVoxel );
	static bool GetVoxel( const TMap< FIntVector, FHexagonVoxel >& Map, const FVector& WorldLocation, FHexagonVoxel& OutVoxel );

	TMap< FIntVector, FHexagonVoxel > HexagonTiles;

	FIntPoint Coordinate = FIntPoint::ZeroValue;

	UPROPERTY( EditDefaultsOnly, Category = "Chunk" )
	TObjectPtr< UProceduralMeshComponent > ProceduralMesh;

	UPROPERTY( EditDefaultsOnly, Category = "Chunk" )
	int Size = 16;
	UPROPERTY( EditDefaultsOnly, Category = "Chunk" )
	int Height = 32;
	UPROPERTY( EditDefaultsOnly, Category = "Chunk" )
	float NoiseScale = .02f;

	FTimerHandle VisibilityTimer;

	static int   StaticSize;
	static int   StaticHeight;
	static float StaticNoiseScale;
};
