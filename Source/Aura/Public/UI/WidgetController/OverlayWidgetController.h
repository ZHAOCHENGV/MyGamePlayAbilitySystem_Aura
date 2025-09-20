// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "OverlayWidgetController.generated.h"

class UAuraAbilitySystemComponent;
struct FAuraAbilityInfo;
class UAuraUserWidget;



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


//声明动态委托
//FOnHealthChangedSignature FOnAttributeChangedSignature 两个动态代理类型,分别表示在健康值或最大健康值变化时的事件通知
//float NewValue：表示新值
//float 类型的，可以共用这一个委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature,float,NewValue);



//声明消息组件行动态多播
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageWidgetRowSignature,FUIWidgetRow,Row);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelChangedSignature,int32,NewLevel,bool,bLevelUp);


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
	FOnAttributeChangedSignature OnHealthChanged;
	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxHealthChanged;
	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnAttributeChangedSignature OnManaChanged;
	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxManaChanged;

	// 暴露给蓝图的委托
	UPROPERTY(BlueprintAssignable, Category ="GAS|Messages")
	FMessageWidgetRowSignature MessageWidgetRowSignature;



	//经验值百分比委托
	UPROPERTY(BlueprintAssignable, Category ="GAS|XP")
	FOnAttributeChangedSignature OnXPPercentChangedDelegate;

	//在玩家级别更改时代理
	UPROPERTY(BlueprintAssignable, Category ="GAS|Level")
	FOnLevelChangedSignature OnPlayerLevelChangedDelegate;

	
protected:

	
	//消息数据表
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Widget Data")
	TObjectPtr<UDataTable> MessageWidgetDataTable;

	

	
	

	// template<typename T> 模板函数，允许使用任意类型 T 的数据表行结构体
	//根据标签获取数据表中的行信息
	template<typename T>
	T* GetDataTableRowByTag(UDataTable *DataTable,const FGameplayTag &Tag);
	
	//经验值更改函数
	void OnXPChanged(int32 NewXP);

	//装备能力
	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,const FGameplayTag& Slot, const FGameplayTag& PreviousSlot);
};

template <typename T>
T* UOverlayWidgetController::GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag)
{
	// 从指定的数据表中查找行：
	// - T：表示数据表行的类型，通常是继承自 FTableRowBase 的结构体。
	// - Tag.GetTagName()：根据标签获取对应的行名。
	// - TEXT("")：上下文字符串，用于标记调试来源，当前留空。
	return DataTable->FindRow<T>(Tag.GetTagName(),TEXT(""));
}
