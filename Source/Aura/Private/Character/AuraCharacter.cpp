// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"

AAuraCharacter::AAuraCharacter()
{
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
}

//重写 PossessedBy ，在一个角色（Pawn）被新的控制器（Controller）接管时调用。
//在服务端（Server）中，当一个新的控制器（如玩家的 PlayerController）接管了这个角色时，需要初始化角色的能力系统，以确保以下内容：
//能力系统组件绑定正确的拥有者（Owner）和代理者（Avatar）。
//角色可以正确激活和使用能力。
//总结：确保在服务端初始化能力系统。
void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();
	
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

void AAuraCharacter::InitAbilityActorInfo()
{
	//获取玩家状态
	AAuraPlayerState * AuraPlayerState =  GetPlayerState<AAuraPlayerState>();
	//检查玩家状态
	check(AuraPlayerState);
	//获取玩家状态中的ASC组件，并且设置ASC组件的拥有者是玩家状态和代理者是此actor
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState,this);
	//初始化自身的AbilitySystemComponent和AttributeSet
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();
	
	//获取控制器
	if (AAuraPlayerController * AuraPlayerController = Cast<AAuraPlayerController>(GetController());)
	{
		if (AAuraHUD * AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			AuraHUD->InitOverlay(AuraPlayerController,AuraPlayerState,AbilitySystemComponent,AttributeSet);
		}
	}
	
	

	
}


