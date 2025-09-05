// Fill out your copyright notice in the Description page of Project Settings.

#include "MaterialExpressionWorldToUV.h"

#include "MaterialCompiler.h"
#include "UnnamedFactoryGame/World/Generation/HexagonVoxel.h"

#if WITH_EDITOR
int32 UMaterialExpressionWorldToUV::Compile( FMaterialCompiler* Compiler, const int32 OutputIndex )
{
	// Convert world location to voxel coordinate
	const int32 WorldLocationID = WorldLocation.Compile( Compiler );

	const int32 XWorldLocationID = Compiler->ComponentMask( WorldLocationID, true, false, false, false );
	const int32 YWorldLocationID = Compiler->ComponentMask( WorldLocationID, false, true, false, false );

	const int32 HexagonRadiusID = Compiler->Constant( HexagonRadius );

	const int32 TwoThirdsID     = Compiler->Constant( 2.f / 3.f );
	const int32 NegativeThirdID = Compiler->Constant( -1.f / 3.f );
	const int32 Root3Divided3ID = Compiler->Constant( Root3 / 3 );
	const int32 NegativeOneID   = Compiler->Constant( -1 );
	const int32 ZeroID          = Compiler->Constant( .00001f );

	const int32 QFloatID = Compiler->Div( Compiler->Mul( TwoThirdsID, XWorldLocationID ), HexagonRadiusID );
	const int32 RFloatID = Compiler->Div( Compiler->Add( Compiler->Mul( NegativeThirdID, XWorldLocationID ), Compiler->Mul( Root3Divided3ID, YWorldLocationID ) ),
	                                      HexagonRadiusID );
	const int32 SFloatID = Compiler->Sub( Compiler->Mul( QFloatID, NegativeOneID ), RFloatID );

	const int32 QRoundedID = Compiler->Round( QFloatID );
	const int32 RRoundedID = Compiler->Round( RFloatID );
	const int32 SRoundedID = Compiler->Round( SFloatID );

	const int32 QDiffID = Compiler->Abs( Compiler->Sub( QRoundedID, QFloatID ) );
	const int32 RDiffID = Compiler->Abs( Compiler->Sub( RRoundedID, RFloatID ) );
	const int32 SDiffID = Compiler->Abs( Compiler->Sub( SRoundedID, SFloatID ) );

	const int32 NewQCoordinateID = Compiler->Sub( Compiler->Mul( RRoundedID, NegativeOneID ), SRoundedID );
	int32       NewRCoordinateID = Compiler->Sub( Compiler->Mul( QRoundedID, NegativeOneID ), SRoundedID );
	NewRCoordinateID             = Compiler->If( RDiffID, SDiffID, NewRCoordinateID, RRoundedID, RRoundedID, ZeroID );

	const int32 RAndSMaxID = Compiler->Max( RDiffID, SDiffID );

	const int32 QCoordinateID = Compiler->If( QDiffID, RAndSMaxID, NewQCoordinateID, QRoundedID, QRoundedID, ZeroID );
	const int32 RCoordinateID = Compiler->If( QDiffID, RAndSMaxID, RRoundedID, NewRCoordinateID, NewRCoordinateID, ZeroID );

	// Convert voxel coordinate to world location
	const int32 Root3ID         = Compiler->Constant( Root3 );
	const int32 Root3Divided2ID = Compiler->Constant( Root3Divided2 );
	const int32 OneAndHalfID    = Compiler->Constant( 1.5f );

	const int32 XWorldID = Compiler->Mul( HexagonRadiusID, Compiler->Mul( OneAndHalfID, QCoordinateID ) );
	const int32 YWorldID = Compiler->Mul( HexagonRadiusID, Compiler->Add( Compiler->Mul( Root3ID, RCoordinateID ), Compiler->Mul( Root3Divided2ID, QCoordinateID ) ) );

	// Convert to local location
	const int32 XPixelDiffID = Compiler->Sub( XWorldID, XWorldLocationID );
	const int32 YPixelDiffID = Compiler->Sub( YWorldID, YWorldLocationID );

	// Convert to uv
	const int32 DoubleHexagonRadiusID = Compiler->Constant( HexagonRadius * 2 );
	const int32 OneHalfID             = Compiler->Constant( .5f );

	const int32 Xuv = Compiler->Add( Compiler->Div( XPixelDiffID, DoubleHexagonRadiusID ), OneHalfID );
	const int32 Yuv = Compiler->Add( Compiler->Div( YPixelDiffID, DoubleHexagonRadiusID ), OneHalfID );

	return Compiler->AppendVector( Xuv, Yuv );
}
#endif
