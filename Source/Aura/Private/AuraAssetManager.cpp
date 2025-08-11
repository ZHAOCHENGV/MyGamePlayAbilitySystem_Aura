// Fill out your copyright notice in the Description page of Project Settings.


#include "AuraAssetManager.h"

#include "AbilitySystemGlobals.h"
#include "AuraGamePlayTags.h"

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine);
	//类型转换为Aura资产管理器
	return * Cast<UAuraAssetManager>(GEngine->AssetManager);
}

void UAuraAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	//调用初始化本地游戏标签
	FAuraGamePlayTags::InitializeNativeGameplayTags();
	//使用目标数据(TargetData)必须初始化全局数据
	UAbilitySystemGlobals::Get().InitGlobalData();
}
