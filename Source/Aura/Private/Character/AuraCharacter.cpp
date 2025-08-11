// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/AuraAbilitySystemComponent.h"
#include "GAS/Data/LevelUpInfo.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "NiagaraComponent.h"
#include "Camera/CameraComponent.h"
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

int32 AAuraCharacter::GetPlayerLevel_Implementation()
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	//执行玩家状态中的获取等级函数
	return  AuraPlayerState->GetPlayerLevel();
}


void AAuraCharacter::InitAbilityActorInfo()
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	//获取玩家状态中的ASC组件，并且设置ASC组件的拥有者是玩家状态和代理者是此actor
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState,this);
	//获取类型为UAuraAbilitySystemComponent技能组件，并且初始化技能属性集
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	//初始化自身的AbilitySystemComponent和AttributeSet
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();
	
	//获取控制器是否有效
	if (AAuraPlayerController * AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		//获取HUD是否有效
		if (AAuraHUD * AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			//有效则初始化重叠HUD
			AuraHUD->InitOverlay(AuraPlayerController,AuraPlayerState,AbilitySystemComponent,AttributeSet);
		}
	}
	//应用游戏效果，来初始化默认属性
	InitializeDefaultAttributes();

	
}



