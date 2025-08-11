// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AttributeMenuWidgetController.generated.h"

class UAttributeInfo;
struct FAuraAttributeInfo;
struct FGameplayTag;
//定义动态广播委托  const FAuraAttributeInfo &是委托参数类型
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttrobuteInfoSigmature, const FAuraAttributeInfo &, Info);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAttributeMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	//重载函数 将回调函数绑定到依赖项
	virtual void BindCallbacksToDependencies() override;
	//重载函数 广播初始值
	virtual void BroadcastInitialValues() override;

	//声明属性信息委托变量
	//BlueprintAssignable：表示委托可以在蓝图中被绑定和调用。它使得该委托能够在蓝图中编辑和分配（绑定事件处理程序）。
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FAttrobuteInfoSigmature AttributeInfoDelegate;

	//声明属性点已更改委托变量
	//BlueprintAssignable：表示委托可以在蓝图中被绑定和调用。它使得该委托能够在蓝图中编辑和分配（绑定事件处理程序）。
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnPlayerStatChangedSignature AttributePointsChangedDelegate;

	UFUNCTION(BlueprintCallable)
	void UpgradeAttribute(const FGameplayTag& AttributeTag);
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAttributeInfo> AttributeInfo;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag GameplayTag;

private:
	void BroadcastAttributeInfo(const FGameplayTag & AttributeTag , const FGameplayAttribute & Attribute)const;
};
