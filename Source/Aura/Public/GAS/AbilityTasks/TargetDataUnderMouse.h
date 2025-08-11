// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

// 声明一个动态多播代理（事件），它将携带一个 FVector 类型参数
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataSignature, const FGameplayAbilityTargetDataHandle&, DataHandle);

/**
 * 
 */
UCLASS()
class AURA_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()
public:
	/**
	 * 静态工厂方法，用于创建目标数据任务
	 * DisplayName = "TargetDataUnderMouse",在蓝图中显示为 "TargetDataUnderMouse"，而不是原函数名。
	 * HidePin = "OwningAbility",隐藏蓝图节点上的 OwningAbility 输入引脚，简化蓝图接口。
	 * DefaultToSelf = "OwningAbility", 默认将 OwningAbility 设置为蓝图自身（通常是当前的能力实例）。
	 * BlueprintInternalUseOnly = "true" 声明仅供内部使用,该函数不会出现在普通蓝图类的节点菜单中。
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "TargetDataUnderMouse",HidePin = "OwningAbility",DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility);

	// 蓝图可分配的事件，通知有效的数据已经获取
	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataSignature ValidData;

private:
	// 重写 AbilityTask 的激活方法，在任务启动时调用
	virtual void Activate() override;
	//发送鼠标光标击中的数据
	void SendMouseCursorData();
	//当目标数据从服务器回调时调用
	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag Activation);
	
};
