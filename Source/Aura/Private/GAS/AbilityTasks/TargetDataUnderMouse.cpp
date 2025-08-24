// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AbilityTasks/TargetDataUnderMouse.h"
#include "AbilitySystemComponent.h"
#include "Aura/Aura.h"


UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	// 调用基础类 UAbilityTask 的模板方法 NewAbilityTask
	// 创建并返回一个实例化的 TargetDataUnderMouse 任务对象
	UTargetDataUnderMouse * MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	// 检查当前能力是否由本地玩家控制
	const bool bIsLocallyControlled  = Ability->GetCurrentActorInfo()->IsLocallyControlledPlayer();
	if (bIsLocallyControlled)
	{
		// 如果是本地玩家控制，则直接发送鼠标数据
		SendMouseCursorData();
	}
	else// 是远程玩家控制，等待服务器同步数据
	{
		

		// 获取当前能力的规格句柄
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		// 获取当前能力的激活预测键
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		// 为目标数据的同步事件添加回调函数
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this,&UTargetDataUnderMouse::OnTargetDataReplicatedCallback);
		// 检查是否已经同步了目标数据
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
		// 如果没有目标数据，则标记为等待远程玩家数据
		if(!bCalledDelegate)
		{
			//标记为等待远程玩家数据
			SetWaitingOnRemotePlayerData();
		}
	}
	

}
 
void UTargetDataUnderMouse::SendMouseCursorData()
{
	// 创建预测窗口，确保客户端和服务器同步
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	
	
	// 获取当前能力的玩家控制器（PlayerController）
	APlayerController * PlayerController = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	// 获取鼠标光标下的碰撞检测信息
	PlayerController->GetHitResultUnderCursor(ECC_Target,false,CursorHit);
	// 定义目标数据句柄
	FGameplayAbilityTargetDataHandle DataHandle;
	// 创建单目标命中数据对象
	FGameplayAbilityTargetData_SingleTargetHit * Data = new FGameplayAbilityTargetData_SingleTargetHit();
	// 将检测到的命中结果保存到目标数据对象
	Data->HitResult = CursorHit;
	// 将目标数据对象添加到数据句柄中
	DataHandle.Add(Data);
	// 将目标数据（这里即客户端数据）同步到服务器
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),						// 当前能力的规格句柄
		GetActivationPredictionKey(),				// 当前能力的激活预测键，用于客户端与服务器的同步。是客户端生成的用于同步能力操作的唯一标识
		DataHandle,									// 要同步的目标数据句柄
		FGameplayTag(),								// 额外的标签信息（此处为空）
		AbilitySystemComponent->ScopedPredictionKey // 是一个范围 (Scoped) 内的全局预测键，用于客户端与服务器之间的一系列同步操作。
	);
	// 如果需要广播事件，则广播目标数据
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle,FGameplayTag Activation)
{
	// 消耗客户端的预测数据，不需要再存储数据，确保不会再次使用同一数据
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(),GetActivationPredictionKey());
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		// 如果需要广播事件，则广播目标数据
		ValidData.Broadcast(DataHandle);
	}
}
