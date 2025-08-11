// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/** 
 * 
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	//BlueprintCallable  蓝图可调用函数
	UFUNCTION(BlueprintCallable)
	//设置控件控制器
	void SetWidgetController(UObject* InWidgetController);

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;

	

protected:
	//BlueprintImplementableEvent  蓝图可实现的事件
	UFUNCTION(BlueprintImplementableEvent)
	//控件控制器设置
	void WidgetControllerSet();
	

};
