// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class UnnamedFactoryGame : ModuleRules
{
	public UnnamedFactoryGame( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange( new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "ProceduralMeshComponent" } );

		PrivateDependencyModuleNames.AddRange( new string[] {} );
	}
}