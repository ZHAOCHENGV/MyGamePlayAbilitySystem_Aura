// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGamePlayTags.h"
#include "GameplayTagContainer.h"
#include "EnhancedInputSubsystems.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Actor/MagicCircle.h"
#include "Aura/Aura.h"
#include "Components/SplineComponent.h"
#include "GAS/AuraAbilitySystemComponent.h"
#include "Input/AuraInputComponent.h"
#include "Character/CharacterBase.h"
#include "Components/DecalComponent.h"
#include "Interation/EnemyInterface.h"
#include "Interation/HighlightInterface.h"
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
	UpdateMagicCircleLocation();
}

/**
 * @brief 在玩家控制器上显示“魔法指示圈”（Decal），用于选地/施法预览
 *
 * @param DecalMaterial 可选的材质（传入则覆盖默认材质）
 *
 * 功能说明：
 * - 只在指针 MagicCircle 不存在时生成一次 AMagicCircle（避免重复生成）。
 * - 如传入 Decal 材质，设置到指示圈的 Decal 组件第 0 号槽位。
 *
 * 详细流程：
 * 1) 判断 MagicCircle 是否有效；若无 → Spawn 一个 AMagicCircle；
 * 2) 若传入 DecalMaterial → 设置到 MagicCircle 的 Decal 组件；
 * 3) 后续通过 UpdateMagicCircleLocation 持续更新位置，通过 HideMagicCircle 销毁。
 *
 * 注意事项：
 * - 确保 MagicCircleClass 在蓝图/资产中已设置；否则 SpawnActor 会失败（返回 nullptr）。
 * - 该类多用于“本地表现”，推荐只在**拥有者客户端**调用（避免在服务器/非拥有客户端生成 UI 类 Actor）。
 * - 材质槽位索引为 0，如你的 Decal 有多层材质，请确认索引。
 */
void AAuraPlayerController::ShowMagicCircle(UMaterialInterface* DecalMaterial)
{
	// 若当前还没有生成过指示圈
	if (!IsValid(MagicCircle))                                      // IsValid：非空且未 PendingKill
	{
		MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass); // 生成指示圈 Actor（类来自配置）
		if (DecalMaterial)                                           // 传入了材质就替换默认材质
		{
			MagicCircle->MagicCircleDecal->SetMaterial(0, DecalMaterial); // 设置到 Decal 组件第 0 个槽位
		}
	}
}
/**
 * @brief 隐藏（销毁）魔法指示圈
 *
 * 功能说明：
 * - 若指示圈已存在，则调用 Destroy() 进行销毁。
 *
 * 注意事项：
 * - Destroy 后指针不会自动置空，但 IsValid 将返回 false；如需更严谨，可手动置 nullptr。
 * - 如果是“短时间频繁开关”，建议用 SetActorHiddenInGame(true/false) 替代销毁重建（见文末建议）。
 */
void AAuraPlayerController::HideMagicCircle()
{
	if (IsValid(MagicCircle))                                       // 确认当前存在
	{
		MagicCircle->Destroy();                                     // 销毁 Actor（PendingKill）
	}
}
/**
 * @brief 持续更新魔法指示圈的位置到当前鼠标命中点
 *
 * 功能说明：
 * - 将 AMagicCircle 的 Actor 位置设置为 CursorHit.ImpactPoint（你的鼠标射线命中点）。
 *
 * 注意事项：
 * - 确保 CursorHit 由外部逻辑实时更新（Tick/输入事件中进行 LineTrace）。
 * - 若需要贴地朝向，可在 AMagicCircle 内或此处同时设置旋转，使 Decal 法线对齐地面法线。
 */
void AAuraPlayerController::UpdateMagicCircleLocation()
{
	if (IsValid(MagicCircle))                                       // 仅当存在时更新位置
	{
		MagicCircle->SetActorLocation(CursorHit.ImpactPoint);       // 把位置对齐到鼠标命中点
	}
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
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGamePlayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
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
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGamePlayTags::Get().Player_Block_CursorTrace))
	{
		UnHighlightActor(ThisActor);
		UnHighlightActor(LastActor);
		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}
	//当魔法阵有效时，设置 ECC_ExcludePlayers，否则设置ECC_Visibility
	const ECollisionChannel TraceChannel = IsValid(MagicCircle) ? ECC_ExcludePlayers : ECC_Visibility;
	//获取光标击中的结果
	GetHitResultUnderCursor(TraceChannel,false,CursorHit);
	//如果未击中，则返回
	if(!CursorHit.bBlockingHit)return;
	LastActor = ThisActor;
	if (IsValid(CursorHit.GetActor()) && CursorHit.GetActor()->Implements<UHighlightInterface>())
	{
		//获取继承接口的Actor  
		ThisActor = CursorHit.GetActor();
	}
	else
	{
		ThisActor = nullptr;
	}

	//光标追踪
	if(LastActor != ThisActor)
	{
		UnHighlightActor(LastActor);
		HighlightActor(ThisActor);
	}
	

}

void AAuraPlayerController::HighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_HighlightActor(InActor);
	}
}

void AAuraPlayerController::UnHighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_UnHighlightActor(InActor);
	}
}

/**
 * @brief 处理“按下”类型的输入标签：左键按下时切换目标锁定并关闭自动奔跑；其他输入透传到 ASC
 * @param InputTag 按下的输入标签（例如 LMB、技能热键等）
 * @details
 *  - 左键（LMB）按下：若有当前选中目标则进入 bTargeting（目标锁定），同时关闭 bAutoRunning。
 *  - 无论何种输入，都会把该 InputTag 转发给 ASC 的 AbilityInputTagPressed 以驱动 GA。
 *  - 仅在“按下瞬间”调用；持续按住逻辑在 AbilityInputTagHeld，松开逻辑在 AbilityInputTagReleased。
 */
void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGamePlayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	// 判断输入是否为“鼠标左键”（完全匹配 LMB Tag）
	if (InputTag.MatchesTagExact(FAuraGamePlayTags::Get().InputTag_LMB)) // LMB 按下
	{
		if (IsValid(ThisActor))
		{
			// 若当前存在 ThisActor（表示有选中目标），则进入目标锁定；否则不锁定
			TargetingStatus = ThisActor->Implements<UEnemyInterface>() ? ETargetingStatus::TargetingEnemy : ETargetingStatus::TargetingNonEnemy;
			// 无论是否锁定，按下 LMB 都会关闭“自动奔跑”
			bAutoRunning = false; // 停止自动寻路/奔跑
		}
		else
		{
			TargetingStatus = ETargetingStatus::NotTargeting;
		}
	
	}
	// 如果 ASC 存在，则把“按下”事件透传给 ASC（用于触发/预测能力）
	if (GetASC())
	{
		// 透传给能力系统
		GetASC()->AbilityInputTagPressed(InputTag);
	} 
}

/**
 * @brief 处理“松开”类型的输入标签：非左键直接透传；左键松开时可能触发一次“点地移动/自动奔跑”
 * @param InputTag 松开的输入标签
 * @details
 *  - 非 LMB：仅把 Released 事件转发给 ASC，立即返回（与移动无关）。
 *  - LMB：释放后若“未在目标锁定且未自动奔跑”，并且按住时间小于阈值 → 走一次点击寻路，绘制路径并启动自动奔跑。
 *  - 释放后统一重置 FollowTime，并退出目标锁定。
 */
void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGamePlayTags::Get().Player_Block_InputReleased))
	{
		return;
	}
	// 若不是“鼠标左键”，只需要把“松开”事件发给 ASC，然后返回
	if (!InputTag.MatchesTagExact(FAuraGamePlayTags::Get().InputTag_LMB)) // 非 LMB
	{
		// ASC 存在则透传松开事件
		if(GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag); // 通知能力系统
		}
		return; // 结束：非左键不涉及移动逻辑
	}

	// 是 LMB：先把“松开”事件交给 ASC（技能释放/取消等）
	if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag); // 通知能力系统
	
	// 若当前不在目标锁定，且不处于自动奔跑状态 → 可能触发一次“点击寻路”
	if(TargetingStatus != ETargetingStatus::TargetingEnemy && !bAutoRunning) 
	{
		// 取当前玩家控制的 Pawn
		const APawn * ControlledPawn = GetPawn(); // 受控角色
		// 若按住时间小于“短按阈值”，且 Pawn 有效 → 视为一次“点击移动”
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())
			{
				IHighlightInterface::Execute_SetMoveToLocation(ThisActor, CachedDestination);
			}
			else if (GetASC() && !GetASC()->HasMatchingGameplayTag(FAuraGamePlayTags::Get().Player_Block_InputPressed))
			{
				// 在点击位置播放一个 Niagara 效果，作为点击反馈
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination); // 点击特效
			}
			// 使用导航系统同步计算从 Pawn 到 CachedDestination 的路径
			if (UNavigationPath * NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this,ControlledPawn->GetActorLocation(),CachedDestination)) // 查路径
			{
				
				// 清空样条上的旧路径点
				Spline->ClearSplinePoints(); // 清路径
				// 把导航路径的各点写入样条（用于可视化或沿样条移动）
				for (const FVector& PointLoc :NavPath->PathPoints) // 遍历路径点
				{
					Spline->AddSplinePoint(PointLoc,ESplineCoordinateSpace::World); // 加入样条
				}
				
				// 若确实有路径点，更新最终目的地并开启自动奔跑
				if (NavPath->PathPoints.Num() > 0)
				{
					// 以路径最后一个点作为最终落点
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1]; // 更新目标
					// 开启自动奔跑（Tick/其他逻辑据此沿样条前进）
					bAutoRunning = true; // 启动自动奔跑
				}
			}
			
			
		}
	
		// 释放后重置“按住计时”
		FollowTime = 0.f; // 计时清零
		// 释放左键后退出“目标锁定”（与按下时的进入相对）
		TargetingStatus = ETargetingStatus::NotTargeting; // 退出锁定
	}
}

/**
 * @brief 处理“按住”类型的输入标签：左键按住时进行“长按寻路/即时移动”或转发给 ASC
 * @param InputTag 按住的输入标签
 * @details
 *  - 非 LMB：仅把 Held 事件转发到 ASC。
 *  - LMB：
 *    * 若处于目标锁定或按着 Shift：将 Held 事件交给 ASC（例如引导技能/持续释放）。
 *    * 否则：把按住时间累加为 FollowTime；持续更新 CachedDestination=光标命中点；并给 Pawn 施加移动输入（即时跟随光标点）。
 */
void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGamePlayTags::Get().Player_Block_InputHeld))
	{
		return;
	}
	// 非左键：只把“Held”事件交给 ASC
	if (!InputTag.MatchesTagExact(FAuraGamePlayTags::Get().InputTag_LMB)) // 非 LMB
	{
		if(GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag); // 透传给能力系统
		}
		return; // 返回：非 LMB 不做移动逻辑
	}

	// 左键被按住：若当前在目标锁定模式或按下 Shift，则把 Held 交给 ASC（技能持续输入）
	if (TargetingStatus == ETargetingStatus::TargetingEnemy||bShiftKeyDown)
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag); // 交由能力系统处理（如引导型技能）
		}
	}
	else // 否则：进行“长按寻路/即时移动”逻辑
	{
		// 累加按住时长（用于区分短按/长按）
		FollowTime += GetWorld()->GetDeltaSeconds(); // 递增计时
		
		// 若光标命中有效，持续更新目标点（用于即时跟随）
		if (CursorHit.bBlockingHit) // 命中了地面/可行走区域
		{
			CachedDestination = CursorHit.ImpactPoint; // 刷新目标点
		}
		// 获取玩家当前 Pawn
		if (APawn * ControlledPawn = GetPawn())
		{
			// 计算从 Pawn 到目标点的单位方向
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal(); // 方向
			// 给 Pawn 施加移动输入（基于 CharacterMovement）
			ControlledPawn->AddMovementInput(WorldDirection); // 即时向目标点移动
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

