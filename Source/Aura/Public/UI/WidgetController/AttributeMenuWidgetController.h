// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AttributeMenuWidgetController.generated.h"

/**
 * 
 */
// BlueprintType : 使用这个宏声明的类可以在蓝图中创建对象、实例化和操作。这意味着该类的实例可以在蓝图中作为类型进行声明和使用。
// Blueprintable : 它允许你将该类作为蓝图基类，或者在蓝图中创建该类的对象。
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAttributeMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	//重载函数 将回调函数绑定到依赖项
	virtual void BindCallbacksToDependencies() override;
	//重载函数 广播初始值
	virtual void BroadcastInitialValues() override;
	
};
