// Fill out your copyright notice in the Description page of Project Settings.

#include "MiningToolComponent.h"

#include "UnnamedFactoryGame/World/Generation/ProceduralHexagonMeshComponent.h"

UMiningToolComponent::UMiningToolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder< UMaterial > MaterialFinder( TEXT( "/Game/Art/Materials/MiningTool_M" ) );
	if( MaterialFinder.Succeeded() )
		Material = MaterialFinder.Object;
}

void UMiningToolComponent::TickComponent( const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if( !IsValid( MeshComponent ) )
		return;

	const FIntVector Coordinate = FHexagonVoxel::WorldToVoxel( InteractableData.ImpactPoint - InteractableData.ImpactNormal * 10 );
	MeshComponent->SetWorldLocation( FHexagonVoxel( Coordinate ).WorldLocation );
}

void UMiningToolComponent::UpdateSize( const int32 SizeChange )
{
	Radius = FMath::Clamp( Radius + SizeChange, MinRadius, MaxRadius );

	TMap< FIntVector, FHexagonVoxel > HexagonVoxels;

	FIntVector    VoxelCoordinate = FIntVector::ZeroValue;
	FHexagonVoxel Voxel( VoxelCoordinate, EVoxelType::Ground );
	HexagonVoxels.Add( VoxelCoordinate, Voxel );

	TQueue< TTuple< int32, FIntVector > > Queue;
	Queue.Enqueue( MakeTuple( 1, VoxelCoordinate ) );

	TTuple< int32, FIntVector > Current;
	while( Queue.Dequeue( Current ) )
	{
		for( const FIntVector& Direction: HexagonDirections )
		{
			VoxelCoordinate = Current.Value + Direction;
			Voxel           = FHexagonVoxel( VoxelCoordinate, EVoxelType::Ground );

			if( Current.Key < Radius )
			{
				HexagonVoxels.Add( VoxelCoordinate, Voxel );
				Queue.Enqueue( MakeTuple( Current.Key + 1, VoxelCoordinate ) );
			}
			else if( Current.Key == Radius && Direction.Z == 0 )
				HexagonVoxels.Add( VoxelCoordinate, Voxel );
		}
	}

	MeshComponent->Generate( HexagonVoxels );
}

void UMiningToolComponent::Activate( const bool bReset )
{
	Super::Activate( bReset );

	AActor* Owner = GetOwner();
	if( !IsValid( Owner ) )
		return;

	MeshComponent = NewObject< UProceduralHexagonMeshComponent >( Owner );
	if( !MeshComponent )
		return;

	MeshComponent->RegisterComponent();

	if( IsValid( Material ) )
		MeshComponent->SetMaterial( 0, Material );

	UpdateSize( 0 );
}

void UMiningToolComponent::Deactivate()
{
	Super::Deactivate();

	if( !MeshComponent )
		return;

	MeshComponent->DestroyComponent();
	MeshComponent = nullptr;
}
