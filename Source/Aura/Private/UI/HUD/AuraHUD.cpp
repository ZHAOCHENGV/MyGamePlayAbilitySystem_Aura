// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AuraHUD.h"

#include "../../../../../../../../../../../Program Files/Epic Games/UE_5.4/Engine/Plugins/Animation/ACLPlugin/Source/ThirdParty/acl/external/rtm/includes/rtm/types.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"
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
		// 将回调函数绑定到依赖的属性值变化事件
		// 绑定后，当属性值（如 Health 或 MaxHealth）发生变化时，回调函数会被触发，更新 UI 或执行其他逻辑
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	// 如果已存在控件控制器，则直接返回
	return OverlayWidgetController;
}

UAttributeMenuWidgetController* AAuraHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	// 如果没有创建过控件对象，则创建一个新的
	if (AttributeMenuWidgetController == nullptr)
	{
		// 创建一个新的 UAttributeMenuWidgetController 对象
		AttributeMenuWidgetController = NewObject<UAttributeMenuWidgetController>(this,AttributeMenuWidgetControllerClass);
		// 设置控件的参数
		AttributeMenuWidgetController->SetWidgetControllerParams(WCParams);
		// 绑定控件的回调依赖关系
		AttributeMenuWidgetController->BindCallbacksToDependencies();
	}
	// 返回控件对象
	return AttributeMenuWidgetController;
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
	//广播初始值
	WidgetController->BroadcastInitialValues();
	//将控件添加到视口
	Widget->AddToViewport();
}


