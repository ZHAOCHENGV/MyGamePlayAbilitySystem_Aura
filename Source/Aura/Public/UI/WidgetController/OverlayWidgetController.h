// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "OverlayWidgetController.generated.h"

//声明动态委托
//FOnHealthChangedSignature FOnMaxHealthChangedSignature 两个动态代理类型,分别表示在健康值或最大健康值变化时的事件通知
//float NewHealth 表示当前的健康值（Health）
//float NewMaxHealth：表示当前的最大健康值（MaxHealth）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature,float,NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealtChangedSignature,float,NewMaxHealth);


/**
 * 
 */
//BlueprintType 作用：允许该类的实例可以作为蓝图中的变量
//Blueprintable 作用：允许从该类派生蓝图（即可以在蓝图编辑器中创建子类）
UCLASS(BlueprintType, Blueprintable)
class AURA_API UOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	//广播初始值
	virtual void BroadcastInitialValues() override;

	//BlueprintAssignable,仅用于多播委托，标记这些属性可以在蓝图中绑定事件。当健康值或最大健康值改变时，蓝图中绑定的函数会被调用。
	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnHealthChangedSignature OnHealthChanged;
	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnMaxHealtChangedSignature OnMaxHealthChanged;
	
};
