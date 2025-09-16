// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldGenerationSubSystem.h"

#include "Kismet/GameplayStatics.h"
#include "UnnamedFactoryGame/Player/FactoryPlayer.h"

UWorldGenerationSubSystem::UWorldGenerationSubSystem()
{
	static ConstructorHelpers::FClassFinder< AChunk > MaterialFinder( TEXT( "/Game/Blueprints/World/Generation/Chunk_BP" ) );
	if( MaterialFinder.Succeeded() )
		ChunkClass = MaterialFinder.Class;
}

void UWorldGenerationSubSystem::Tick( const float DeltaTime )
{
	Super::Tick( DeltaTime );

	const AFactoryPlayer* Player = AFactoryPlayer::Get( this );
	if( !IsValid( Player ) )
		return;

	int32           ChunksGenerated = 0;
	const FIntPoint Chunk           = AChunk::WorldToChunk( Player->GetActorLocation() );
	for( int Q = -GenerationDistance; Q < GenerationDistance; Q++ )
	{
		for( int R = -GenerationDistance; R < GenerationDistance; R++ )
		{
			FIntPoint Other = Chunk + FIntPoint( Q, R );
			if( IsWithinDistance( Chunk, Other ) && UpdateChunk( Other, ChunksGenerated > 0 ) )
				ChunksGenerated++;
		}
	}
}

AChunk* UWorldGenerationSubSystem::GetChunk( const FVector& WorldLocation )
{
	const FIntPoint Chunk = AChunk::WorldToChunk( WorldLocation );
	return GetChunk( Chunk );
}

AChunk* UWorldGenerationSubSystem::GetChunk( const FIntVector& VoxelCoordinate )
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

bool UWorldGenerationSubSystem::GetVoxel( const FIntVector& VoxelCoordinate, FHexagonVoxel& OutVoxel )
{
	const AChunk* Chunk = GetChunk( VoxelCoordinate );
	if( !IsValid( Chunk ) )
		return false;

	if( !Chunk->GetVoxel( VoxelCoordinate, OutVoxel ) )
		return false;

	return true;
}

bool UWorldGenerationSubSystem::UpdateChunk( const FIntPoint& Chunk, const bool OnlyVisibility )
{
	const TObjectPtr< AChunk >* ChunkActorPtr = Chunks.Find( Chunk );
	if( ChunkActorPtr && IsValid( *ChunkActorPtr ) )
	{
		( *ChunkActorPtr )->SetVisible();
		return false;
	}

	if( OnlyVisibility )
		return false;

	const FVector WorldLocation = AChunk::ChunkToWorld( Chunk );
	AChunk*       ChunkActor    = GetWorld()->SpawnActor< AChunk >( ChunkClass, FTransform( WorldLocation ) );
	ChunkActor->Generate( Chunk );
	ChunkActor->SetVisible();
	Chunks.Add( Chunk, ChunkActor );
	return true;
}
bool UWorldGenerationSubSystem::IsWithinDistance( const FIntPoint& Current, const FIntPoint& Other ) const
{
	const int32 SCurrent = -Current.X - Current.Y;
	const int32 SOther   = -Other.X - Other.Y;
	const int32 Distance = ( FMath::Abs( Current.X - Other.X ) + FMath::Abs( Current.Y - Other.Y ) + FMath::Abs( SCurrent - SOther ) ) / 2;

	return Distance < GenerationDistance;
}