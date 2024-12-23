// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AttributeMenuWidgetController.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAttributeMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	//重载函数 将回调函数绑定到依赖项
	virtual void BindCallbacksToDependencies() override;
	//重载函数 广播初始值
	virtual void BroadcastInitialValues() override;
	
};
