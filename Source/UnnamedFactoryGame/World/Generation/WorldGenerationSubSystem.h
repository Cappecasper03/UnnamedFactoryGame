// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Chunk.h"
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "WorldGenerationSubSystem.generated.h"

/**
 * 
 */
UCLASS()
class UNNAMEDFACTORYGAME_API UWorldGenerationSubSystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UWorldGenerationSubSystem();

	static UWorldGenerationSubSystem* Get( const UObject* WorldContextObject ) { return WorldContextObject->GetWorld()->GetSubsystem< UWorldGenerationSubSystem >(); }

	virtual TStatId GetStatId() const override { return TStatId(); }

	virtual void Tick( float DeltaTime ) override;

	AChunk* GetChunk( const FVector& WorldLocation );
	AChunk* GetChunk( const FIntVector& VoxelCoordinate );
	AChunk* GetChunk( const FIntPoint& ChunkCoordinate );

	bool GetVoxel( const FVector& WorldLocation, FHexagonVoxel& OutVoxel );
	bool GetVoxel( const FIntVector& VoxelCoordinate, FHexagonVoxel& OutVoxel );

private:
	bool GenerateChunk( FIntPoint Chunk );

	bool IsWithinDistance( const FIntPoint& Current, const FIntPoint& Other ) const;

	TMap< FIntPoint, TObjectPtr< AChunk > > Chunks;

	UPROPERTY()
	TSubclassOf< AChunk > ChunkClass;

	int GenerationDistance = 8;
};
