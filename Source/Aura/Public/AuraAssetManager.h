// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AuraAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraAssetManager : public UAssetManager
{
	GENERATED_BODY()

	public:
	//获取资产管理器
	static UAuraAssetManager& Get();
	private:
	//开始初始加载
	virtual void StartInitialLoading() override;
	
};
