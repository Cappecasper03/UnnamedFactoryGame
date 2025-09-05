// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialExpression.h"

#include "MaterialExpressionWorldToUV.generated.h"

/**
 * 
 */
UCLASS()
class UNNAMEDFACTORYGAME_API UMaterialExpressionWorldToUV : public UMaterialExpression
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual int32 Compile( FMaterialCompiler* Compiler, int32 OutputIndex ) override;

	virtual FText GetCreationName() const override { return FText::FromString( "World Location To UV" ); }
	virtual void  GetCaption( TArray< FString >& OutCaptions ) const override { OutCaptions.Add( "World To UV" ); }
	virtual void  GetExpressionToolTip( TArray< FString >& OutToolTip ) override { OutToolTip.Add( "Converts world location to uv" ); }
#endif

	UPROPERTY( meta = ( RequiredInput = true ) )
	FExpressionInput WorldLocation;
};
