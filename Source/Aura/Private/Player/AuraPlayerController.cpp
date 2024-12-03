// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


AAuraPlayerController::AAuraPlayerController()
{
	//网络复制
	bReplicates = true;
	
}

void AAuraPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CursorTrace();
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	//检查 AuraContext 是否非空。如果为空，程序会在这里中断运行。
	check(AuraContext);
	//获取本地玩家的增强输入子系统
	UEnhancedInputLocalPlayerSubsystem * Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		//当前玩家添加输入映射上下文,0是最高优先级
		Subsystem->AddMappingContext(AuraContext, 0);
	}
	

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

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	//将基础输入组件InputComponent转换为增强输入
	UEnhancedInputComponent * EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	//绑定输入操作的函数
	//ETriggerEvent::Triggered： 触发事件::触发   
	EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Triggered,this,&AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const struct FInputActionValue& InputActionValue)
{
	//从InputActionValue 提取输入的2D向量
	const FVector2D InputAxisVector2D = InputActionValue.Get<FVector2D>();
	//获取当前控制器旋转
	const FRotator Rotation = GetControlRotation();
	//提取控制器Yaw的方向
	const FRotator YawRotation(0.f,Rotation.Yaw,0.f);

	//将旋转角度（YawRotation）转换为旋转矩阵，通过GetUnitAxis，提取矩阵的 X 轴（前方向量）
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	//将旋转角度（YawRotation）转换为旋转矩阵，通过GetUnitAxis，提取矩阵的 Y 轴（右方向量）
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	//获取当前控制的 Pawn
	if(APawn * ControlledPawn = GetPawn<APawn>())
	{
		//添加移动输入
		ControlledPawn->AddMovementInput(ForwardDirection,InputAxisVector2D.Y);
		ControlledPawn->AddMovementInput(RightDirection,InputAxisVector2D.X);
		
	}

	
}

//鼠标下检测跟踪
void AAuraPlayerController::CursorTrace()
{
	//创建光标击中参数
	FHitResult CuresorHit;
	//获取光标击中的结果
	GetHitResultUnderCursor(ECC_Visibility,false,CuresorHit);
	//如果未击中，则返回
	if(!CuresorHit.bBlockingHit)return;
	LastActor = ThisActor;
	//获取继承接口的Actor  
	ThisActor =CuresorHit.GetActor();

	//新版光标追踪，更简洁明了
	if(LastActor != ThisActor)
	{
		if (LastActor)
		{
			LastActor->UnHigHlightActor();
			
		}
		if (ThisActor)
		{
			ThisActor->HigHlihtActor();
		}
		
	}
	/*		旧版光标追踪
	*       光标开始跟踪。有几种情况：
		A.如果LastActor为空，ThisActor为空
			-不采取任何操作。
		B.如果LastActor是空的，ThisActor是有效的
			-突出显示ThisActor。
		C.如果LastActor是有效的，而ThisActor是空的
			-取消突出LastActor。
		D.如果两个Actor都是有效的，但是LastActor不等于ThisActor
			-取消最后一个Actor并突出这个Actor。
		E.如果两个Actor都有效并且它们是同一Actor
			-不采取任何行动。
	
	if(LastActor == nullptr)
	{
		if(ThisActor != nullptr)
		{
			//B.当前Actor突出显示
			ThisActor->HigHlihtActor();

		}
		else
		{
			//A.Last 和 This 都为空。则不做任何事情
		}

		
	}
	else //lastActor 有效
	{
		
		if(ThisActor == nullptr)
		{
			//C.取消突出显示LastActor
			LastActor->UnHigHlightActor();
		}
		else//ThisActor不为空
		{
			
			if(LastActor != ThisActor)
			{
				//D.取消lastActor,
				LastActor->UnHigHlightActor();
				//突出显示ThisActor。
				ThisActor->HigHlihtActor();
			}
			else//ThisActor==LastActor
			{
				//E.不做任何操作
				
			}
		}
	}
	* 
	 */

}
