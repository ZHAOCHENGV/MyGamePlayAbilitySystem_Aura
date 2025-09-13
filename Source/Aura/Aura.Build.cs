// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Aura : ModuleRules
{
	public Aura(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput","GameplayAbilities","NavigationSystem"});

		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayTags","GameplayTasks","NavigationSystem","Niagara","AIModule" });

		// 取消注释（如果使用的是Slate UI）
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// 如果您正在使用联机功能，请取消注释
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");
		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
