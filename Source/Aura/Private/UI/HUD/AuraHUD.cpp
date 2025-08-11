// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/SpellMenuWidgetController.h"

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
		return OverlayWidgetController;
	}
	// 如果已存在控件控制器，则直接返回
	return OverlayWidgetController;
}

UAttributeMenuWidgetController* AAuraHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	//如果控件控制器为空，则创建并初始化它
	if (AttributeMenuWidgetController == nullptr)
	{
		// 创建新的控件控制器对象
		AttributeMenuWidgetController = NewObject<UAttributeMenuWidgetController>(this,AttributeMenuWidgetControllerClass);
		// 设置控件控制器的参数
		AttributeMenuWidgetController->SetWidgetControllerParams(WCParams);
		// 将回调函数绑定到依赖的属性值变化事件
		// 绑定后，当属性值（如 Health 或 MaxHealth）发生变化时，回调函数会被触发，更新 UI 或执行其他逻辑
		AttributeMenuWidgetController->BindCallbacksToDependencies();
		
	}
	// 如果已存在控件控制器，则直接返回
	return AttributeMenuWidgetController;
}

/**
 * 获取或创建法术菜单控件控制器（SpellMenuWidgetController）
 *
 * 功能说明：
 * 本函数用于获取当前的法术菜单控件控制器实例。如果该控制器尚未创建，则会基于指定的类进行实例化，
 * 并初始化其参数和依赖回调。此方法确保控件控制器在同一个HUD对象中只会被创建一次（单例模式），
 * 后续调用会直接返回已存在的实例，避免重复创建。
 *
 * 详细说明：
 * - 首先判断 SpellMenuWidgetController 是否为 nullptr（即是否尚未创建）。
 * - 若未创建，则调用 NewObject，在当前 HUD（this）上生成 Widget Controller 实例。
 *   NewObject 是虚幻引擎用于动态创建 UObject 实例的推荐方式，并自动进行生命周期管理。
 * - 调用 SetWidgetControllerParams 方法，将外部传入的参数结构体（WCParams）赋值给控件控制器，
 *   通常这些参数包括玩家控制器、能力系统组件、属性等依赖对象。
 * - 调用 BindCallbacksToDependencies，将控件控制器中的回调函数与依赖的事件或数据源进行绑定，
 *   确保控件能根据游戏状态动态响应变化。
 * - 最终返回 SpellMenuWidgetController，无论是新建的还是已存在的实例。
 */
USpellMenuWidgetController* AAuraHUD::GetSpellMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	// 如果控件控制器实例尚未创建
	if (SpellMenuWidgetController == nullptr)
	{
		// 基于指定的类在当前HUD上创建控件控制器实例
		SpellMenuWidgetController = NewObject<USpellMenuWidgetController>(this,SpellMenuWidgetControllerClass);
		// 设置控件控制器所需的参数（如依赖对象指针等）
		SpellMenuWidgetController->SetWidgetControllerParams(WCParams);
		// 绑定控件控制器内部回调到其依赖的数据源或事件上
		SpellMenuWidgetController->BindCallbacksToDependencies();
	}
	// 返回控件控制器实例
	return SpellMenuWidgetController;
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


