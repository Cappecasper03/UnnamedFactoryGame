// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexagonVoxel.h"

#include "Chunk.generated.h"

class UProceduralHexagonMeshComponent;
class UHierarchicalInstancedStaticMeshComponent;

UCLASS( Abstract )
class UNNAMEDFACTORYGAME_API AChunk : public AActor
{
	GENERATED_BODY()

public:
	AChunk();

	void Generate( const FIntPoint& ChunkCoordinate );

	void SetVisible();

	bool GetVoxel( const FIntVector& VoxelCoordinate, FHexagonVoxel& OutVoxel ) const { return FHexagonVoxel::GetVoxel( HexagonTiles, VoxelCoordinate, OutVoxel ); }
	bool GetVoxel( const FVector& WorldLocation, FHexagonVoxel& OutVoxel ) const { return FHexagonVoxel::GetVoxel( HexagonTiles, WorldLocation, OutVoxel ); }

	static FVector ChunkToWorld( const FIntPoint& ChunkCoordinate );

	static FIntPoint VoxelToChunk( const FIntVector& VoxelCoordinate );
	static FIntPoint WorldToChunk( const FVector& WorldLocation );

protected:
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Chunk" )
	TObjectPtr< UProceduralHexagonMeshComponent > Mesh;

private:
	void GenerateVoxels();

	TMap< FIntVector, FHexagonVoxel > HexagonTiles;

	FIntPoint Coordinate = FIntPoint::ZeroValue;

	UPROPERTY( EditDefaultsOnly, Category = "Chunk" )
	int32 Size = 16;
	UPROPERTY( EditDefaultsOnly, Category = "Chunk" )
	int32 Height = 32;
	UPROPERTY( EditDefaultsOnly, Category = "Chunk" )
	float NoiseScale = .02f;

	FTimerHandle VisibilityTimer;
	FTimerHandle LoadedTimer;

	static int32 StaticSize;
	static int32 StaticHeight;
	static float StaticNoiseScale;
};
