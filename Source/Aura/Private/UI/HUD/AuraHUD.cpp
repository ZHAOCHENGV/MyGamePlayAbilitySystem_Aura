// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"

UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	//如果控件控制器为空，则创建并初始化它
	if (OverlayWidgetController == nullptr)
	{
		// 创建新的控件控制器对象
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this,OverlayWidgetControllerClass);
		// 设置控件控制器的参数
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		return OverlayWidgetController;
	}
	// 如果已存在控件控制器，则直接返回
	return OverlayWidgetController;
}

void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	//判断OverlayWidgetClass和OverlayWidgetControllerClass是否被初始化，如果没有编辑器崩溃并且打印日志
	checkf(OverlayWidgetClass,TEXT("OverlayWidgetClass未初始化，请填写BP_AuraHUD"));
	checkf(OverlayWidgetControllerClass,TEXT("OverlayWidgetControllerClass未初始化,请填写BP_AuraHUD"));
	// 创建 UI 控件
	UUserWidget * Widget = CreateWidget<UUserWidget>(GetWorld(),OverlayWidgetClass);
	// 转换控件为特定类型
	OverlayWidget = Cast<UAuraUserWidget>(Widget);
	//初始化控件控制器的参数
	const FWidgetControllerParams WidgetControllerParams(PC,PS,ASC,AS);
	//转换控件为特定类型
	UOverlayWidgetController * WidgetController = GetOverlayWidgetController(WidgetControllerParams);
	//初始化控件控制器参数
	OverlayWidget->SetWidgetController(WidgetController);
	//将控件添加到视口
	Widget->AddToViewport();
}


