// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Chunk.generated.h"

class UProceduralMeshComponent;
class UHierarchicalInstancedStaticMeshComponent;

USTRUCT()
struct FHexagonVoxelCoordinate
{
	GENERATED_BODY()

	int32 Q;
	int32 R;
	int32 Z;

	bool operator==( const FHexagonVoxelCoordinate& Other ) const { return Q == Other.Q && R == Other.R && Z == Other.Z; }
};

USTRUCT()
struct FHexagonVoxel
{
	GENERATED_BODY()

	FHexagonVoxel() = default;
	FHexagonVoxel( const FHexagonVoxelCoordinate& Coordinate );

	FHexagonVoxelCoordinate GridLocation;
	FVector                 WorldLocation;

	bool operator==( const FHexagonVoxel& Other ) const { return GridLocation == Other.GridLocation; }
};

inline uint32 GetTypeHash( const FHexagonVoxelCoordinate& Coordinate ) { return HashCombine( HashCombine( Coordinate.Q, Coordinate.R ), Coordinate.Z ); }

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

	bool GetVoxel( const FHexagonVoxelCoordinate& VoxelCoordinate, FHexagonVoxel& OutVoxel );

	static FVector VoxelToWorld( const FHexagonVoxelCoordinate& VoxelCoordinate ) { return FHexagonVoxel( VoxelCoordinate ).WorldLocation; }
	static FVector ChunkToWorld( const FIntPoint& ChunkCoordinate );

	static FHexagonVoxelCoordinate WorldToVoxel( const FVector& WorldLocation );
	static FIntPoint               VoxelToChunk( const FHexagonVoxelCoordinate& VoxelCoordinate );
	static FIntPoint               WorldToChunk( const FVector& WorldLocation );

protected:
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

	TMap< FHexagonVoxelCoordinate, FHexagonVoxel > HexagonTiles;

	FIntPoint Coordinate = FIntPoint::ZeroValue;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Chunk" )
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
