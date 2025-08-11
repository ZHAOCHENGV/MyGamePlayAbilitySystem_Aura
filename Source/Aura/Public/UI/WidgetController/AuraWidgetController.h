// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraWidgetController.generated.h"


class UAuraAttributeSet;
class UAuraAbilitySystemComponent;
class AAuraPlayerState;
class AAuraPlayerController;
class UAbilityInfo;
//声明委托  在玩家更改数据时
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChangedSignature,int32, NewValue);

// 声明动态多播委托（支持蓝图绑定）
// 参数：FAuraAbilityInfo结构体（包含技能信息）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FAuraAbilityInfo&, Info);


class UAttributeSet;
class UAbilitySystemComponent;

//BlueprintType 蓝图可使用结构体
//定义FWidgetControllerParams类型的结构体
USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()
	//默认构造函数
	FWidgetControllerParams(){}
	//带参数的构造函数，
	//:PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS)初始化参数
	FWidgetControllerParams(APlayerController * PC, APlayerState * PS, UAbilitySystemComponent * ASC, UAttributeSet * AS)
	:PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS){}

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
	

};


/**
 * 
 */
UCLASS()
class AURA_API UAuraWidgetController : public UObject
{
	GENERATED_BODY()

public:
	//BlueprintCallable蓝图可调用，设置控件控制器参数
	//设置控件控制器参数
	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FWidgetControllerParams & WCParams);

	//广播初始值
	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues();
	//将回调函数绑定到依赖项
	virtual void BindCallbacksToDependencies();

	// 暴露给蓝图的委托
	UPROPERTY(BlueprintAssignable, Category ="GAS|Messages")
	FAbilityInfoSignature AbilityInfoDelegate;

	//广播能力信息
	void BroadcastAbilityInfo();

	
	
protected:
	//消息数据表
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Widget Data")
	TObjectPtr<UAbilityInfo> AbilityInfo;
	
	UPROPERTY(BlueprintReadOnly , Category  = "WidgetController")
	TObjectPtr<APlayerController> PlayerController;
	UPROPERTY(BlueprintReadOnly , Category  = "WidgetController")
	TObjectPtr<APlayerState> PlayerState;
	UPROPERTY(BlueprintReadOnly , Category  = "WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(BlueprintReadOnly , Category  = "WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly , Category  = "WidgetController")
	TObjectPtr<AAuraPlayerController> AuraPlayerController;
	UPROPERTY(BlueprintReadOnly , Category  = "WidgetController")
	TObjectPtr<AAuraPlayerState> AuraPlayerState;
	UPROPERTY(BlueprintReadOnly , Category  = "WidgetController")
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;
	UPROPERTY(BlueprintReadOnly , Category  = "WidgetController")
	TObjectPtr<UAuraAttributeSet> AuraAttributeSet;

	//获取Aura玩家控制器
	AAuraPlayerController* GetAuraPC();
	//获取Aura玩家状态
	AAuraPlayerState* GetAuraPS();
	//获取Aura能力组件
	UAuraAbilitySystemComponent* GetAuraASC();
	//获取Aura属性
	UAuraAttributeSet* GetAuraAS();
};
