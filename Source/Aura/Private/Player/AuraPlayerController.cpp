// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"

#include "EnhancedInputSubsystems.h"

AAuraPlayerController::AAuraPlayerController()
{
	//网络复制
	bReplicates = true;
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	//检查 AuraContext 是否非空。如果为空，程序会在这里中断运行。
	check(AuraContext);
	//获取本地玩家的增强输入子系统
	UEnhancedInputLocalPlayerSubsystem * Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	//检查 check(Subsystem)是否非空。如果为空，程序会在这里中断运行。
	check(Subsystem);
	//当前玩家添加输入映射上下文,0是最高优先级
	Subsystem->AddMappingContext(AuraContext,0);

	//鼠标指针显示
	bShowMouseCursor = true;
	//鼠标指针样式为默认
	DefaultMouseCursor = EMouseCursor::Default;

	//设定输入模式为「游戏和 UI 模式」
	FInputModeGameAndUI InputModeData;
	//设置鼠标是否锁定在游戏视口内
	//EMouseLockMode::DoNotLock 表示鼠标不会被限制在窗口内
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	//设置捕获鼠标输入时是否隐藏鼠标指针，false 表示不会隐藏。
	InputModeData.SetHideCursorDuringCapture(false);
	//应用上述输入模式配置
	SetInputMode(InputModeData);
}
