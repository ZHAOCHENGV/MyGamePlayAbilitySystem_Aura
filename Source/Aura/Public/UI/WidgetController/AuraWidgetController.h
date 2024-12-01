// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "UObject/NoExportTypes.h"
#include "AuraWidgetController.generated.h"

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

protected:
	UPROPERTY(BlueprintReadOnly , Category  = "WidgetController")
	TObjectPtr<APlayerController> PlayerController;
	UPROPERTY(BlueprintReadOnly , Category  = "WidgetController")
	TObjectPtr<APlayerState> PlayerState;
	UPROPERTY(BlueprintReadOnly , Category  = "WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(BlueprintReadOnly , Category  = "WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;
	
};
