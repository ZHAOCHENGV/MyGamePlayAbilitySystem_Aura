// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "AuraGamePlayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/AuraAbilitySystemComponent.h"
#include "GAS/Data/LevelUpInfo.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "NiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "Debuff/DebuffNiagaraComponent.h"
#include "GameFramework/SpringArmComponent.h"

AAuraCharacter::AAuraCharacter()
{

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bDoCollisionTest = false;

	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>("TopDownCameraComponent");
	TopDownCameraComponent->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;
	
	
	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LevelUpNiagaraComponent"));
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());
	LevelUpNiagaraComponent->bAutoActivate = false;
	//开启运动方向旋转
	GetCharacterMovement()->bOrientRotationToMovement = true;
	//设置旋转速度
	GetCharacterMovement()->RotationRate = FRotator(0, 400.f, 0);
	//约束到平面
	GetCharacterMovement()->bConstrainToPlane = true;
	//在开始时对齐平面
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	
	//使用控制器旋转Pitch关闭
	bUseControllerRotationPitch = false;
	//使用控制器旋转Roll关闭
	bUseControllerRotationRoll = false;
	//使用控制器旋转Yaw关闭
	bUseControllerRotationYaw = false;
	//设置元素师职业
	CharacterClass = ECharacterClass::Elementtalist;
}

//重写 PossessedBy ，在一个角色（Pawn）被新的控制器（Controller）接管时调用。
//在服务端（Server）中，当一个新的控制器（如玩家的 PlayerController）接管了这个角色时，需要初始化角色的能力系统，以确保以下内容：
//能力系统组件绑定正确的拥有者（Owner）和代理者（Avatar）。
//角色可以正确激活和使用能力。
//总结：确保在服务端初始化能力系统。
void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//为服务器初始化能力信息
	InitAbilityActorInfo();
	//为服务器添加角色能力组件
	AddCharacterAbilities();
	
}

//重写 OnRep_PlayerState ，在网络同步时，PlayerState 属性被复制（Replicated）到客户端时调用。
//在客户端（Client）上，PlayerState 是通过网络复制同步的。而 AbilitySystemComponent 通常绑定在 PlayerState 上，因此在客户端接收到 PlayerState 后需要重新初始化能力系统，保证以下内容：
//客户端有正确的能力系统信息。
//客户端可以正确显示角色的能力和属性（比如 UI 中的生命值、法力值等）。
//总结：确保在客户端同步 PlayerState 后，重新初始化能力系统。
void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}

void AAuraCharacter::AddToXP_Implementation(int32 InXP)
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	AuraPlayerState->AddToXP(InXP);
}

void AAuraCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	AuraPlayerState->AddToLevel(InPlayerLevel);
	if (UAuraAbilitySystemComponent* AUarASC = Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		//根据玩家等级设置技能状态
		AUarASC->UpdateAbilityStatuses(AuraPlayerState->GetPlayerLevel());
	}
}

void AAuraCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints)
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	AuraPlayerState->AddToAttributePoints(InAttributePoints);
}

void AAuraCharacter::AddToSpellPoints_Implementation(int32 InSpellPoints)
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	AuraPlayerState->AddToSpellPoints(InSpellPoints);
}

int32 AAuraCharacter::GetAttributePointsReward_Implementation(int32 Level) const
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	return AuraPlayerState->LevelUpInfo->LevelUpInformation[Level].AttributePointAward;
	
}

int32 AAuraCharacter::GetSpellPointsReward_Implementation(int32 Level) const
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	return AuraPlayerState->LevelUpInfo->LevelUpInformation[Level].SpellPointAward;
}

void AAuraCharacter::LeveUp_Implementation()
{
	MulticastLevelUpParticles();
}



/**
 * 在所有客户端播放角色升级特效（Multicast RPC 实现体）
 *
 * 功能说明：
 * 该方法通过多播RPC（Multicast RPC）机制，使所有客户端都能同步触发角色升级粒子特效。
 * 其核心流程是：首先确保升级用的Niagara粒子组件有效，然后计算该组件朝向主摄像机的旋转，
 * 最后设置组件朝向并激活粒子效果，保证特效在所有客户端上视觉一致且更加突出。
 *
 * 详细说明：
 * - MulticastLevelUpParticles_Implementation 是虚幻引擎的网络RPC函数体，负责在所有客户端上执行。
 * - 首先检查 LevelUpNiagaraComponent 是否有效，避免空指针异常。
 * - 获取顶视角摄像机（TopDownCameraComponent）的位置，和特效组件当前所在的位置。
 * - 通过两者位置差计算旋转量，使特效始终朝向摄像机，提高视觉表现力。
 * - 调用 SetWorldRotation 设置粒子组件的朝向，然后调用 Activate 激活特效（参数true表示重置状态后再激活）。
 */
void AAuraCharacter::MulticastLevelUpParticles_Implementation()
{
	// 确保升级Niagara特效组件有效，防止空指针
	if (IsValid(LevelUpNiagaraComponent))
	{
		// 获取主摄像机的世界空间位置
		const FVector CameraLocation = TopDownCameraComponent->GetComponentLocation();
		// 获取粒子特效组件（Niagara）的世界空间位置
		const FVector NiagaraLocation = LevelUpNiagaraComponent->GetComponentLocation();
		// 计算从粒子特效指向摄像机的旋转，使特效朝向摄像机
		const FRotator ToCameraRotation = (CameraLocation - NiagaraLocation).Rotation();
		// 设置粒子特效组件的朝向
		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
		// 激活粒子特效（参数true表示每次激活都重置特效状态）
		LevelUpNiagaraComponent->Activate(true);
	}
}

int32 AAuraCharacter::GetXP_Implementation() const
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	return AuraPlayerState->GetXp();
}

int32 AAuraCharacter::FindLevelForXP_Implementation(int32 InXP)
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	return AuraPlayerState->LevelUpInfo->FindLevelForXp(InXP);
}

int32 AAuraCharacter::GetAttributePoints_Implementation() const
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	return AuraPlayerState->GetAttributePoints();
}

int32 AAuraCharacter::GetSpellPoints_Implementation() const
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	return AuraPlayerState->GetSpellPoints();
}

/**
 * @brief （RPC/事件实现）在角色端请求显示“魔法指示圈”
 *
 * @param DecalMaterial 可选材质（覆盖默认）
 *
 * 功能说明：
 * - 从角色获取其控制器，并转为 AAuraPlayerController，调用控制器的 ShowMagicCircle。
 *
 * 注意事项：
 * - 函数名带 _Implementation：通常对应 BlueprintNativeEvent / RPC 的实现体。
 * - 建议作为**Client/OwnerOnly**调用，避免在服务器/非拥有客户端生成 UI 表现。
 */
void AAuraCharacter::ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial)
{
	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController())) // 拿到我自己的控制器
	{
		AuraPlayerController->ShowMagicCircle(DecalMaterial);       // 让控制器去生成/显示
	}
}
/**
 * @brief （RPC/事件实现）在角色端请求隐藏“魔法指示圈”
 *
 * 功能说明：
 * - 获取控制器并调用 HideMagicCircle 销毁指示圈 Actor。
 */
void AAuraCharacter::HideMagicCircle_Implementation()
{
	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController())) // 拿到我自己的控制器
	{
		AuraPlayerController->HideMagicCircle();                    // 让控制器去隐藏（销毁）
	}
}

int32 AAuraCharacter::GetPlayerLevel_Implementation()
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	//执行玩家状态中的获取等级函数
	return  AuraPlayerState->GetPlayerLevel();
}


/**
 * @brief 眩晕标志（bIsStunned）在客户端被复制更新时的回调：添加/移除输入阻断类 Tag，并切换眩晕特效组件
 *
 * @details
 * 功能说明：
 * - 依赖 UPROPERTY(ReplicatedUsing=OnRep_Stunned) 的复制机制；当 bIsStunned 在服务端改变后，客户端会调用本函数。
 * - 眩晕时向 ASC 添加一组“阻断”LooseGameplayTags；解除时移除这些 Tag。同时激活/停用眩晕特效组件。
 *
 * 详细流程：
 * 1) 取得本角色的 ASC（强转为 UAuraAbilitySystemComponent 以使用自定义接口）；
 * 2) 组装一套用于阻断输入/点击检测的标签容器；
 * 3) 若 bIsStunned 为真 → AddLooseGameplayTags + 激活特效；否则 → RemoveLooseGameplayTags + 停用特效。
 *
 * 注意事项：
 * - 确保 bIsStunned 声明为 UPROPERTY(ReplicatedUsing=OnRep_Stunned)，且角色 bReplicates=true；
 * - Add/Remove 的 Tag 集合要和游戏逻辑保持对称，否则会出现“残留阻断”的问题；
 * - StunDebuffComponent 需在构造或 BeginPlay 中创建并保持有效（UPROPERTY() 持有），避免 GC。
 */
void AAuraCharacter::OnRep_Stunned()
{
	// 1) 从 AbilitySystemComponent 强转为自定义 ASC 类型，便于调用扩展 API
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
	{
		// 2) 取全局 GameplayTags 单例（集中管理的标签集合）
		const FAuraGamePlayTags& GamePlayTags = FAuraGamePlayTags::Get();

		// 3) 组装“阻断输入/检测”的标签容器（一次性批量添加/移除）
		FGameplayTagContainer BlockedTag;
		BlockedTag.AddTag(GamePlayTags.Player_Block_CursorTrace);   // 阻止光标射线检测
		BlockedTag.AddTag(GamePlayTags.Player_Block_InputHeld);     // 阻止“按住”输入
		BlockedTag.AddTag(GamePlayTags.Player_Block_InputPressed);  // 阻止“按下”输入
		BlockedTag.AddTag(GamePlayTags.Player_Block_InputReleased); // 阻止“松开”输入

		// 4) 根据眩晕状态切换
		if (bIsStunned)
		{
			AuraASC->AddLooseGameplayTags(BlockedTag); // 加阻断标签（Loose：不走GE生命周期）
			StunDebuffComponent->Activate();           // 激活眩晕特效（如 Niagara/音效）
		}
		else
		{
			AuraASC->RemoveLooseGameplayTags(BlockedTag); // 移除阻断标签
			StunDebuffComponent->Deactivate();            // 停用眩晕特效
		}
	}
}

/**
 * @brief 灼烧标志（bIsBurned）在客户端被复制更新时的回调：切换灼烧特效组件
 *
 * @details
 * 功能说明：
 * - 依赖 UPROPERTY(ReplicatedUsing=OnRep_Burned) 的复制机制；当 bIsBurned 在服务端改变后，客户端会调用本函数。
 * - 根据当前状态激活/停用对应的 Debuff 视觉/音效组件。
 *
 * 注意事项：
 * - BurnDebuffComponent 需为 UPROPERTY() 成员并在初始化时创建，避免空指针/被 GC。
 */
void AAuraCharacter::OnRep_Burned()
{
	// 若处于灼烧状态 → 激活特效；否则关闭
	if (bIsBurned)
	{
		BurnDebuffComponent->Activate();
	}
	else
	{
		BurnDebuffComponent->Deactivate();
	}
}

/**
 * @brief 初始化该角色的 Ability Actor Info，并绑定 HUD/标签事件，应用默认属性
 *
 * @details
 * 功能说明：
 * - 在“玩家状态拥有 ASC、角色作为 Avatar”的架构下：
 *   1) 从 PlayerState 取 ASC 并调用 InitAbilityActorInfo(Owner=PlayerState, Avatar=this)；
 *   2) 初始化 Aura 自定义 ASC 的扩展信息（AbilityActorInfoSet）；
 *   3) 将本角色缓存的 AbilitySystemComponent/AttributeSet 指向 PlayerState 持有的对象；
 *   4) 广播 ASC 已注册（供 UI 或其他系统监听）；
 *   5) 注册眩晕标签的新增/移除事件回调；
 *   6) 若是本地玩家，初始化 HUD Overlay（传入 PC/PS/ASC/AS）；
 *   7) 应用默认属性（初始 GE）。
 *
 * 注意事项：
 * - 该函数通常在 Pawn/Character 的 Possessed 或 BeginPlay 阶段调用（确保 PlayerState/Controller 均已有效）；
 * - 如果是 AI/非玩家角色，PlayerState 可能不同（或没有 HUD），分支判断要健壮；
 * - InitializeDefaultAttributes 内部应确保只在服务器应用一次（属性复制给客户端）。
 */
void AAuraCharacter::InitAbilityActorInfo()
{
	// 1) 获取并校验 PlayerState（多人架构中 ASC 常挂在 PS 上）
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>(); // 取得玩家状态
	check(AuraPlayerState);                                                 // 若无 PS，直接断言（开发期暴露问题）

	// 2) 使用 PS 上的 ASC：设置 Owner=PS，Avatar=this（GAS 标准二元绑定）
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);

	// 3) Aura 自定义 ASC 的扩展初始化（可在此缓存指针、绑定事件等）
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();

	// 4) 本角色缓存 ASC / AttributeSet，统一从 PS 获取（保持与 GAS 绑定一致）
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();

	// 5) 广播“ASC 已注册”（WidgetController/组件可监听此事件完成自身初始化）
	OnAscRegistered.Broadcast(AbilitySystemComponent);

	// 6) 注册“眩晕 Debuff 标签”的新增/移除事件（客户端也能收到，以在本地响应 UI/表现）
	AbilitySystemComponent->RegisterGameplayTagEvent(
		FAuraGamePlayTags::Get().Debuff_Stun, 
		EGameplayTagEventType::NewOrRemoved
	).AddUObject(this, &AAuraCharacter::StunTagChanged);

	// 7) 若控制器为玩家控制器，则初始化 HUD Overlay（绑定 PC/PS/ASC/AS）
	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	// 8) 应用初始属性（一般在服务器执行一次，依赖 GE 复制到客户端）
	InitializeDefaultAttributes();
}


