// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"

#include "ProceduralHexagonMeshComponent.generated.h"

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class UNNAMEDFACTORYGAME_API UProceduralHexagonMeshComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()

public:
	void Generate( TMap< int32, TArray< FIntPoint > >&  TopVisibleVoxels,
	               TMap< int32, TArray< FIntPoint > >&  BottomVisibleVoxels,
	               TMap< FIntVector, TArray< int32 > >& SideVisibleVoxels );

private:
	void GenerateRegions( TMap< int32, TArray< FIntPoint > >& VisibleVoxelCoordinates,
	                      bool                                IsTop,
	                      TArray< FVector >&                  OutVertices,
	                      TArray< int32 >&                    OutTriangles,
	                      TArray< FVector >&                  OutNormals,
	                      TArray< FVector2D >&                OutUVs ) const;
	void GenerateRegions( TMap< FIntVector, TArray< int32 > >& VisibleVoxelCoordinates,
	                      TArray< FVector >&                   OutVertices,
	                      TArray< int32 >&                     OutTriangles,
	                      TArray< FVector >&                   OutNormals,
	                      TArray< FVector2D >&                 OutUVs ) const;

	void GeneratePolygon( int32                PolygonHeight,
	                      TArray< FIntPoint >& Region,
	                      bool                 IsTop,
	                      TArray< FVector >&   OutVertices,
	                      TArray< int32 >&     OutTriangles,
	                      TArray< FVector >&   OutNormals,
	                      TArray< FVector2D >& OutUVs ) const;
	void GeneratePolygon( const FIntVector&    PolygonCoordinate,
	                      TArray< int32 >&     Region,
	                      TArray< FVector >&   OutVertices,
	                      TArray< int32 >&     OutTriangles,
	                      TArray< FVector >&   OutNormals,
	                      TArray< FVector2D >& OutUVs ) const;

	float Signed2DPolygonArea( const TArray< FVector >& Polygon ) const;
};
