// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AuraAbilitySystemLibrary.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	// 获取第一个玩家控制器（PlayerController），这是虚幻引擎中的每个玩家的控制器。
	APlayerController * PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (PC) // 如果玩家控制器有效
	{
		// 尝试将控制器的 HUD 转换为 AAuraHUD 类型的 HUD，AAuraHUD 是自定义的 HUD 类
		if (AAuraHUD * AuraHUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			// 获取玩家的状态（PlayerState）并转换为自定义的 AAuraPlayerState 类型
			AAuraPlayerState * PS = PC->GetPlayerState<AAuraPlayerState>();
			// 获取玩家的能力系统组件（AbilitySystemComponent），用于处理角色的能力
			UAbilitySystemComponent * ASC = PS->GetAbilitySystemComponent();
			// 获取玩家的属性集（AttributeSet），用于存储玩家的各种属性（如生命、魔法等）
			UAttributeSet * AS = PS->GetAttributeSet();
			// 创建 Widget 控制器的参数对象，包含玩家控制器、玩家状态、能力系统组件和属性集
			const FWidgetControllerParams WidgetControllerParams(PC,PS,ASC,AS);
			// 获取并返回 AuraHUD 中的 OverlayWidgetController，该控制器管理 UI 显示
			return AuraHUD->GetOverlayWidgetController(WidgetControllerParams);
		}
	}
	// 如果没有找到对应的控制器或 HUD，返回 nullptr（无效指针）
	return nullptr;
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(
	const UObject* WorldContextObject)
{
	// 获取第一个玩家控制器（PlayerController），这是虚幻引擎中的每个玩家的控制器。
	APlayerController * PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (PC) // 如果玩家控制器有效
	{
		// 尝试将控制器的 HUD 转换为 AAuraHUD 类型的 HUD，AAuraHUD 是自定义的 HUD 类
		if (AAuraHUD * AuraHUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			// 获取玩家的状态（PlayerState）并转换为自定义的 AAuraPlayerState 类型
			AAuraPlayerState * PS = PC->GetPlayerState<AAuraPlayerState>();
			// 获取玩家的能力系统组件（AbilitySystemComponent），用于处理角色的能力
			UAbilitySystemComponent * ASC = PS->GetAbilitySystemComponent();
			// 获取玩家的属性集（AttributeSet），用于存储玩家的各种属性（如生命、魔法等）
			UAttributeSet * AS = PS->GetAttributeSet();
			// 创建 Widget 控制器的参数对象，包含玩家控制器、玩家状态、能力系统组件和属性集
			const FWidgetControllerParams WidgetControllerParams(PC,PS,ASC,AS);
			// 获取并返回 AuraHUD 中的 OverlayWidgetController，该控制器管理 UI 显示
			return AuraHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
		}
}
