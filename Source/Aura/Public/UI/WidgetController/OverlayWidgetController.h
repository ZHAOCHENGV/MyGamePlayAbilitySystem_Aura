// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "OverlayWidgetController.generated.h"

class UAuraUserWidget;
//声明动态委托
//FOnHealthChangedSignature FOnMaxHealthChangedSignature 两个动态代理类型,分别表示在健康值或最大健康值变化时的事件通知
//float NewHealth 表示当前的健康值（Health）
//float NewMaxHealth：表示当前的最大健康值（MaxHealth）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature,float,NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealtChangedSignature,float,NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaChangedSignature,float,NewMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxManaChangedSignature,float,NewMaxMana);

//创建结构体 蓝图可用类型
//FTableRowBase 是虚幻引擎中用于数据表系统的基础结构体。主要原因是为了支持虚幻引擎的数据表（Data Table）功能
USTRUCT(BlueprintType)
struct FUIWidgetRow:public FTableRowBase
{
	GENERATED_BODY()

	//Game标签
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FGameplayTag MessageTag = FGameplayTag();
	//文本
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FText Message = FText();
	//UI组件
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TSubclassOf<UAuraUserWidget> MessageWidget;
	//图片
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	UTexture2D* Image = nullptr;
	
};




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
	// 将回调函数绑定到依赖的属性值变化事件
	// 绑定后，当属性值（如 Health 或 MaxHealth）发生变化时，回调函数会被触发，更新 UI 或执行其他逻辑
	virtual void BindCallbacksToDependencies() override;
	
	//BlueprintAssignable,仅用于多播委托，标记这些属性可以在蓝图中绑定事件。当健康值或最大健康值改变时，蓝图中绑定的函数会被调用。
	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnHealthChangedSignature OnHealthChanged;
	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnMaxHealtChangedSignature OnMaxHealthChanged;
	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnManaChangedSignature OnManaChanged;
	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnMaxManaChangedSignature OnMaxManaChanged;

protected:

	
	//消息小组件数据表
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Widget Data")
	TObjectPtr<UDataTable> MessageWidgetDataTable;

	
	//健康值改变的回调函数
	//OnAttributeChangeData 关于属性变化数据
	void HealthChanged(const FOnAttributeChangeData& Data)const;

	//最大健康值改变的回调函数
	void MaxHealthChanged(const FOnAttributeChangeData& Data)const;

	//魔法值改变的回调函数
	void ManaChanged(const FOnAttributeChangeData& Data)const;

	//最大魔法值改变的回调函数
	void MaxManaChanged(const FOnAttributeChangeData& Data)const;
	
};
