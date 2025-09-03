// Fill out your copyright notice in the Description page of Project Settings.

#include "NavigationComponent.h"

#include "UnnamedFactoryGame/World/Generation/WorldGenerationSubSystem.h"

float FVoxelNode::CalculateFutureCost( const FVoxelNode& Other ) const
{
	const int32 Q = Coordinate.X - Other.Coordinate.X;
	const int32 R = Coordinate.Y - Other.Coordinate.Y;
	const int32 S = ( -Coordinate.X - Coordinate.Y ) - ( -Other.Coordinate.X - Other.Coordinate.Y );

	const float Horizontal = ( FMath::Abs( Q ) + FMath::Abs( R ) + FMath::Abs( S ) ) / 2;
	const float Vertical   = FMath::Abs( Coordinate.Z - Other.Coordinate.Z );
	return Horizontal + Vertical;
}

UNavigationComponent::UNavigationComponent() { PrimaryComponentTick.bCanEverTick = false; }

bool UNavigationComponent::CalculatePath( const FVector& TargetLocation, TArray< FHexagonVoxel >& OutPath ) const
{
	UWorldGenerationSubSystem* WorldGenerationSubSystem = UWorldGenerationSubSystem::Get( this );

	FHexagonVoxel StartVoxel;
	FHexagonVoxel TargetVoxel;
	if( !WorldGenerationSubSystem->GetVoxel( GetOwner()->GetActorLocation(), StartVoxel ) )
		return false;

	if( !WorldGenerationSubSystem->GetVoxel( TargetLocation, TargetVoxel ) )
		return false;

	const FVoxelNode TargetNode{ .Coordinate = TargetVoxel.GridLocation };

	TArray< FVoxelNode >             OpenNodes;
	TSet< FVoxelNode >               ClosedNodes;
	TMap< FIntVector3, FIntVector3 > ParentMap;
	TMap< FIntVector3, float >       CurrentCostMap;

	FVoxelNode CurrentNode{ .Coordinate = StartVoxel.GridLocation };
	OpenNodes.HeapPush( CurrentNode );

	const TArray Directions = {
		FIntVector3( 1, -1, 0 ), FIntVector3( 1, 0, 0 ),  FIntVector3( 0, 1, 0 ), FIntVector3( -1, 1, 0 ),
		FIntVector3( -1, 0, 0 ), FIntVector3( 0, -1, 0 ), FIntVector3( 0, 0, 1 ), FIntVector3( 0, 0, -1 ),
	};

	while( !OpenNodes.IsEmpty() )
	{
		OpenNodes.HeapPop( CurrentNode );

		if( ClosedNodes.Contains( CurrentNode ) )
			continue;

		if( CurrentNode.Coordinate == TargetVoxel.GridLocation )
		{
			OutPath = ReconstructPath( ParentMap, CurrentNode.Coordinate );
			return true;
		}

		ClosedNodes.Add( CurrentNode );

		for( const FIntVector3& Direction: Directions )
		{
			FVoxelNode NeighborNode{ .Coordinate = CurrentNode.Coordinate + Direction };

			if( ClosedNodes.Contains( NeighborNode ) )
				continue;

			FHexagonVoxel Voxel;
			if( !WorldGenerationSubSystem->GetVoxel( NeighborNode.Coordinate, Voxel ) || Voxel.Type != EVoxelType::Air )
				continue;

			if( !WorldGenerationSubSystem->GetVoxel( NeighborNode.Coordinate - FIntVector3( 0, 0, 1 ), Voxel ) )
				continue;

			if( Voxel.Type == EVoxelType::Air )
			{
				if( !WorldGenerationSubSystem->GetVoxel( CurrentNode.Coordinate - FIntVector3( 0, 0, 1 ), Voxel ) || Voxel.Type == EVoxelType::Air )
					continue;

				bool IsValid = false;
				for( int i = 0; i < 6; ++i )
				{
					const FIntVector3& BelowDirection = Directions[ i ];

					if( !WorldGenerationSubSystem->GetVoxel( NeighborNode.Coordinate - FIntVector3( 0, 0, 1 ) + BelowDirection, Voxel ) || Voxel.Type == EVoxelType::Air )
						continue;

					IsValid = true;
					break;
				}

				if( !IsValid )
					continue;
			}

			const float CurrentCost = CurrentCostMap.FindRef( CurrentNode.Coordinate ) + 1;
			if( CurrentCostMap.Contains( NeighborNode.Coordinate ) && CurrentCost > CurrentCostMap.FindRef( NeighborNode.Coordinate ) )
				continue;

			ParentMap.Add( NeighborNode.Coordinate, CurrentNode.Coordinate );
			CurrentCostMap.Add( NeighborNode.Coordinate, CurrentCost );

			NeighborNode.Cost = CurrentCost + NeighborNode.CalculateFutureCost( TargetNode );
			OpenNodes.HeapPush( NeighborNode );
		}
	}

	return false;
}

TArray< FHexagonVoxel > UNavigationComponent::ReconstructPath( const TMap< FIntVector3, FIntVector3 >& ParentMap, FIntVector3& CurrentNode ) const
{
	TArray< FHexagonVoxel > Path;
	Path.Add( FHexagonVoxel( CurrentNode ) );

	while( ParentMap.Contains( CurrentNode ) )
	{
		CurrentNode = ParentMap[ CurrentNode ];
		Path.Add( FHexagonVoxel( CurrentNode ) );
	}

	Algo::Reverse( Path );
	return Path;
}
