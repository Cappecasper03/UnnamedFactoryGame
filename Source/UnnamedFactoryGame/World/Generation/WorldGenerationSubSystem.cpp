// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldGenerationSubSystem.h"

#include "Kismet/GameplayStatics.h"
#include "UnnamedFactoryGame/Player/FactoryPlayer.h"

UWorldGenerationSubSystem::UWorldGenerationSubSystem()
{
	const ConstructorHelpers::FClassFinder< AChunk > ClassFinder( TEXT( "/Game/Blueprints/World/Generation/Chunk_BP" ) );
	ChunkClass = ClassFinder.Class;
}

void UWorldGenerationSubSystem::Tick( const float DeltaTime )
{
	Super::Tick( DeltaTime );

	const AFactoryPlayer* Player = AFactoryPlayer::Get( this );
	if( !IsValid( Player ) )
		return;

	const FIntPoint Chunk = AChunk::WorldToChunk( Player->GetActorLocation() );
	for( int Q = -GenerationDistance; Q < GenerationDistance; Q++ )
	{
		for( int R = -GenerationDistance; R < GenerationDistance; R++ )
			GenerateChunk( Chunk + FIntPoint( Q, R ) );
	}
}

AChunk* UWorldGenerationSubSystem::GetChunk( const FVector& WorldLocation )
{
	const FIntPoint Chunk = AChunk::WorldToChunk( WorldLocation );
	return GetChunk( Chunk );
}

AChunk* UWorldGenerationSubSystem::GetChunk( const FIntVector3& VoxelCoordinate )
{
	const FIntPoint Chunk = AChunk::VoxelToChunk( VoxelCoordinate );
	return GetChunk( Chunk );
}

AChunk* UWorldGenerationSubSystem::GetChunk( const FIntPoint& ChunkCoordinate )
{
	TObjectPtr< AChunk >* Chunk = Chunks.Find( ChunkCoordinate );
	if( !Chunk )
		return nullptr;

	return *Chunk;
}

bool UWorldGenerationSubSystem::GetVoxel( const FVector& WorldLocation, FHexagonVoxel& OutVoxel )
{
	const AChunk* Chunk = GetChunk( WorldLocation );
	if( !IsValid( Chunk ) )
		return false;

	if( !Chunk->GetVoxel( AChunk::WorldToVoxel( WorldLocation ), OutVoxel ) )
		return false;

	return true;
}

bool UWorldGenerationSubSystem::GetVoxel( const FIntVector3& VoxelCoordinate, FHexagonVoxel& OutVoxel )
{
	const AChunk* Chunk = GetChunk( VoxelCoordinate );
	if( !IsValid( Chunk ) )
		return false;

	if( !Chunk->GetVoxel( VoxelCoordinate, OutVoxel ) )
		return false;

	return true;
}

void UWorldGenerationSubSystem::GenerateChunk( const FIntPoint Chunk )
{
	const TObjectPtr< AChunk >* ChunkActorPtr = Chunks.Find( Chunk );
	if( ChunkActorPtr )
	{
		( *ChunkActorPtr )->SetVisible();
		return;
	}

	const FVector WorldLocation = AChunk::ChunkToWorld( Chunk );
	AChunk*       ChunkActor    = GetWorld()->SpawnActor< AChunk >( ChunkClass, FTransform( WorldLocation ) );
	ChunkActor->Generate( Chunk );
	ChunkActor->SetVisible();
	Chunks.Add( Chunk, ChunkActor );
}