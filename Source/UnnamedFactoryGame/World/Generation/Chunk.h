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

	UFUNCTION( CallInEditor )
	void Generate() { Generate( Coordinate ); }
	void Generate( const FIntPoint& ChunkCoordinate );

	void SetVisible();

	bool GetVoxel( const FIntVector3& VoxelCoordinate, FHexagonVoxel& OutVoxel ) const { return GetVoxel( HexagonTiles, VoxelCoordinate, OutVoxel ); }
	bool GetVoxel( const FVector& WorldLocation, FHexagonVoxel& OutVoxel ) const { return GetVoxel( HexagonTiles, WorldLocation, OutVoxel ); }

	static FVector VoxelToWorld( const FIntVector3& VoxelCoordinate ) { return FHexagonVoxel( VoxelCoordinate ).WorldLocation; }
	static FVector ChunkToWorld( const FIntPoint& ChunkCoordinate );

	static FIntVector3 WorldToVoxel( const FVector& WorldLocation );
	static FIntPoint   VoxelToChunk( const FIntVector3& VoxelCoordinate );
	static FIntPoint   WorldToChunk( const FVector& WorldLocation );

private:
	void GenerateVoxels();
	void GenerateMesh( const TMap< FIntVector3, FHexagonVoxel >& HexagonVoxels );

	void CreateHexagonTop( const FHexagonVoxel& Voxel,
	                       TArray< FVector >&   OutVertices,
	                       TArray< int32 >&     OutTriangles,
	                       TArray< FVector >&   OutNormals,
	                       TArray< FVector2D >& OutUVs ) const;
	void CreateHexagonBottom( const FHexagonVoxel& Voxel,
	                          TArray< FVector >&   OutVertices,
	                          TArray< int32 >&     OutTriangles,
	                          TArray< FVector >&   OutNormals,
	                          TArray< FVector2D >& OutUVs ) const;
	void CreateHexagonSide( const FHexagonVoxel& Voxel,
	                        int32                SideIndex,
	                        TArray< FVector >&   OutVertices,
	                        TArray< int32 >&     OutTriangles,
	                        TArray< FVector >&   OutNormals,
	                        TArray< FVector2D >& OutUVs ) const;

	static bool GetVoxel( const TMap< FIntVector3, FHexagonVoxel >& Map, const FIntVector3& VoxelCoordinate, FHexagonVoxel& OutVoxel );
	static bool GetVoxel( const TMap< FIntVector3, FHexagonVoxel >& Map, const FVector& WorldLocation, FHexagonVoxel& OutVoxel );

	TMap< FIntVector3, FHexagonVoxel > HexagonTiles;

	FIntPoint Coordinate = FIntPoint::ZeroValue;

	UPROPERTY( EditDefaultsOnly, Category = "Chunk" )
	TObjectPtr< UProceduralMeshComponent > ProceduralMesh;

	UPROPERTY( EditDefaultsOnly, Category = "Chunk" )
	int Size = 16;
	UPROPERTY( EditDefaultsOnly, Category = "Chunk" )
	int Height = 32;
	UPROPERTY( EditDefaultsOnly, Category = "Chunk" )
	float NoiseScale = .01f;

	FTimerHandle VisibilityTimer;

	static int   StaticSize;
	static int   StaticHeight;
	static float StaticNoiseScale;
};
