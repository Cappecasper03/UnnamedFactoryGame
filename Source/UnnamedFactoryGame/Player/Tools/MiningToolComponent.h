// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseToolComponent.h"
#include "CoreMinimal.h"

#include "MiningToolComponent.generated.h"

class UProceduralHexagonMeshComponent;

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class UNNAMEDFACTORYGAME_API UMiningToolComponent : public UBaseToolComponent
{
	GENERATED_BODY()

public:
	UMiningToolComponent();

	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	virtual void UpdateSize( int32 SizeChange ) override;

protected:
	virtual void Activate( bool bReset = false ) override;
	virtual void Deactivate() override;

	UPROPERTY()
	TObjectPtr< UProceduralHexagonMeshComponent > MeshComponent;

	UPROPERTY()
	TObjectPtr< UMaterial > Material;

	int32 Radius    = 3;
	int32 MinRadius = 1;
	int32 MaxRadius = 5;
};
