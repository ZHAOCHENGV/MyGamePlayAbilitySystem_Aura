// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGamePlayTags.h"
#include "GameplayTagContainer.h"
#include "EnhancedInputSubsystems.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Components/SplineComponent.h"
#include "GAS/AuraAbilitySystemComponent.h"
#include "Input/AuraInputComponent.h"
#include "Character/CharacterBase.h"
#include "UI/Widget/DamageTextComponent.h"


AAuraPlayerController::AAuraPlayerController()
{
	//网络复制
	bReplicates = true;
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CursorTrace();
	AutoRun();
}

void AAuraPlayerController::AutoRun()
{
	// 如果 bAutoRunning 为 false，直接返回，不执行自动奔跑逻辑
	if (!bAutoRunning) return;
	// 获取当前控制的 Pawn（角色）
	if (APawn * ControlledPawn = GetPawn())
	{
		// 1. 在样条曲线上找到最接近角色当前位置的点
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(),ESplineCoordinateSpace::World);
		// 2. 从样条曲线上找到该点的移动方向（切线方向）
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline,ESplineCoordinateSpace::World);
		// 3. 对 Pawn 添加朝着该方向的移动输入
		ControlledPawn->AddMovementInput(Direction);
		// 4. 计算当前位置与目标点的距离
		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		// 5. 如果距离小于设定的接近半径，停止自动奔跑
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
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
	// 将基础输入组件（InputComponent）转换为增强输入组件（UAuraInputComponent）
	// CastChecked 是一种类型转换方法，它会检查转换是否成功，如果失败会触发断言（崩溃并报告错误）。
	UAuraInputComponent * AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	// 绑定移动输入操作
	// MoveAction 是一个增强输入动作（UInputAction），代表玩家的移动操作。
	// ETriggerEvent::Triggered 表示“触发事件”，每帧都会触发对应的回调函数。
	// 当 MoveAction 被触发时，调用 AAuraPlayerController 的 Move 函数来处理移动逻辑。
	AuraInputComponent->BindAction(MoveAction,ETriggerEvent::Triggered,this,&AAuraPlayerController::Move);
	AuraInputComponent->BindAction(ShiftAction,ETriggerEvent::Started,this,&AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction,ETriggerEvent::Completed,this,&AAuraPlayerController::ShiftReleased);
	// 绑定能力相关的输入操作
	// BindAbilityAction 是一个模板函数，用于绑定按下、松开、持续触发的能力输入回调。
	// InputConfig 是一个配置类（UAuraInputConfig），定义了所有能力输入操作及其对应的标签。
	// AbilityInputTagPressed、AbilityInputTagReleased、AbilityInputTagHeld 分别是按下、松开和持续触发时调用的回调函数。
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);

}

void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	// 检查目标角色是否有效，且伤害文本组件类是否有效,和是否是本地控制器（避免在服务器上显示）
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		// NewObject动态创建一个新的伤害文本组件
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		// 因组件是动态创建的，所以需要手动注册组件，使其在游戏中有效
		DamageText->RegisterComponent();
		// 将文本组件附加到目标角色的根组件上（以角色为基准）
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
		// 分离组件，确保它的世界位置保持不变
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		// 设置显示的伤害文本，传入的参数是伤害值
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
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

	//获取光标击中的结果
	GetHitResultUnderCursor(ECC_Visibility,false,CursorHit);
	//如果未击中，则返回
	if(!CursorHit.bBlockingHit)return;
	LastActor = ThisActor;
	//获取继承接口的Actor  
	ThisActor =CursorHit.GetActor();

	//光标追踪
	if(LastActor != ThisActor)
	{
		if (LastActor)LastActor->UnHigHlightActor();
		if (ThisActor)ThisActor->HigHlihtActor();
		
	}
	

}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	//判断输入标签是否完全匹配左键标签（LMB = 鼠标左键）
	if (InputTag.MatchesTagExact(FAuraGamePlayTags::Get().InputTag_LMB))
	{
		// 如果当前有选中的目标对象，则开启目标锁定状态，否则关闭目标锁定
		bTargeting = ThisActor ? true : false;
		// 无论如何都取消自动奔跑状态
		bAutoRunning = false;
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	//判断输入标签是否完全不匹配左键标签（LMB = 鼠标左键）
	if (!InputTag.MatchesTagExact(FAuraGamePlayTags::Get().InputTag_LMB))
	{
		// 如果能力系统组件 (ASC) 存在，则调用它的 AbilityInputTagReleased 方法
		if(GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
		return;// 直接返回，处理完左键逻辑
	}

	// 通过能力系统组件处理释放的输入标签
	if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
	
	// 如果当前不是处于目标锁定模式并且没有按下shift
	if(!bTargeting && !bAutoRunning) 
	{
		
		// 获取当前玩家控制的角色
		const APawn * ControlledPawn = GetPawn();
		// 如果按住时间短于短按阈值，并且角色存在
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			// 使用导航系统同步查找从角色当前位置到缓存目标位置的路径
			if (UNavigationPath * NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this,ControlledPawn->GetActorLocation(),CachedDestination))
			{
				// 清除当前样条曲线上的所有点
				Spline->ClearSplinePoints();
				// 遍历路径点，添加到样条曲线并绘制调试用的黄色球体
				for (const FVector& PointLoc :NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc,ESplineCoordinateSpace::World);
				}
				
				if (NavPath->PathPoints.Num() > 0)
				{
					//将路径最后一个点赋值给目标地点
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
					// 启用自动奔跑状态
					bAutoRunning = true;
				}
				
			}
	
		}
	
		// 重置按住时间
		FollowTime = 0.f;
		// 退出目标锁定模式
		bTargeting = false;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	//判断输入标签是否完全不匹配左键标签（LMB = 鼠标左键）
	if (!InputTag.MatchesTagExact(FAuraGamePlayTags::Get().InputTag_LMB))
	{
		// 如果能力系统组件 (ASC) 存在，则调用它的 AbilityInputTagHeld 方法
		if(GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;// 直接返回，处理完左键逻辑
	}
	// 如果当前处于目标锁定状态或按下shift状态下
	if (bTargeting||bShiftKeyDown)
	{
		// 通过 ASC 处理按住的输入标签
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
	}
	else//否则
	{
		// 增加按住的时间，用于计算长按行为
		FollowTime += GetWorld()->GetDeltaSeconds();
		if (CursorHit.bBlockingHit)
		{
			// 缓存命中点的坐标
			CachedDestination = CursorHit.ImpactPoint;
		}
		// 获取当前玩家控制的角色
		if (APawn * ControlledPawn = GetPawn())
		{
			// 计算从角色位置到目标位置的方向向量
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			// 对角色施加移动输入，朝向目标点移动
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	// 如果能力组件为空
	if (AuraAbilitySystemComponent == nullptr)
	{
		 // 通过蓝图函数库获取被控者的能力组件
		 AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}

