// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnnamedFactoryGame/World/Generation/HexagonVoxel.h"

#include "BaseUnit.generated.h"

class USelectableComponent;
class UFloatingPawnMovement;
class UNavigationComponent;

UCLASS()
class UNNAMEDFACTORYGAME_API ABaseUnit : public AActor
{
	GENERATED_BODY()

public:
	ABaseUnit();

	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION( BlueprintCallable )
	void MoveTo( const FVector& Location );

protected:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Unit" )
	TObjectPtr< UStaticMeshComponent > MeshComponent;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Unit" )
	TObjectPtr< USelectableComponent > SelectableComponent;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Unit" )
	TObjectPtr< UFloatingPawnMovement > MovementComponent;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Unit" )
	TObjectPtr< UNavigationComponent > NavigationComponent;

	TArray< FHexagonVoxel > CurrentPath;
};
