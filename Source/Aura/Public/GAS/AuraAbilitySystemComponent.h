// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

//声明多播委托
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags,const FGameplayTagContainer & /*资产标签AssetTags*/)

// 声明一个带有一个参数的多播委托（允许绑定多个回调函数）
// 参数类型：UAuraAbilitySystemComponent*
// 命名规范：通常以F开头的委托类型
DECLARE_MULTICAST_DELEGATE(FAbilitiesGiven);

// 声明一个带有一个参数的单播委托（只能绑定单个回调函数）
// 参数类型：const FGameplayAbilitySpec&
DECLARE_DELEGATE_OneParam(FForEachAbility,const FGameplayAbilitySpec&);

DECLARE_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusChanged, const FGameplayTag& /*技能标签*/ , const FGameplayTag& /*技能状态标签*/, int32 /*技能等级*/)

DECLARE_MULTICAST_DELEGATE_FourParams(FAbilityEquipped,const FGameplayTag& /*技能标签*/, const FGameplayTag& /*技能状态*/, const FGameplayTag& /*技能插槽*/, const FGameplayTag& /*上一技能插槽*/)

/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	//技能 Actor 信息集
	void AbilityActorInfoSet();

	//定义委托实列
	FEffectAssetTags EffectAbilityTags;

	// 实例化多播委托对象
	FAbilitiesGiven AbilitiesGivenDelegate;

	//委托：能力状态已更改
	FAbilityStatusChanged AbilityStatusChanged;

	//装备能力委托
	FAbilityEquipped AbilityEquipped;
	
	//是否能力初始化完成?
	bool bStartupAbilitiesGiven = false;
	
	//添加角色能技能
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>> & StartUpAbilities);
	//添加角色被动技能
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>> & StartUpPassiveAbilities);

	//按住按键 Tag
	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	//释放按键 Tag
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	//遍历所有可激活能力的实现
	void ForEachAbility(const FForEachAbility & Delegate);
	// 从能力规格中获取能力标签
	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	// 从能力规格中获取输入标签
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	//根据游戏效果获取技能状态
	static FGameplayTag GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	//从技能标签获取状态
	FGameplayTag GetStatusFromAbilityTag(const FGameplayTag& AbilityTag);
	//从能力标签获取输入标签
	FGameplayTag GetInputFromAbilityTag(const FGameplayTag& AbilityTag);
	//根据技能标签查找对应的技能规格
	FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);
	//更新属性
	void UpgradeAttribute(const FGameplayTag& AttributeTag);
	//在服务器上更新属性
	void ServerUpgradeAttribute(const FGameplayTag& AttributeTag);
	//根据玩家等级更新技能状态
	void UpdateAbilityStatuses(int32 Level);

	//在服务器执行，消耗技能点
	UFUNCTION(Server, Reliable)
	void ServerSpendSpellPoint(const FGameplayTag& AbilityTag);
	
	//服务器装备能力
	UFUNCTION(Server, Reliable)
	void ServerEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Slot);
	//客户端装备能力
	void ClientEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot);
	//根据技能标签获取技能描述信息
	bool GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextDescription);
	//清除插槽
	void ClearSlot(FGameplayAbilitySpec* Spec);
	//插槽的清除能力
	void ClearAbilitiesOfSlot(const FGameplayTag& Slot);
	//判断能力是否有槽位
	static bool AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot);
protected:

	virtual void OnRep_ActivateAbilities() override;

	
	//应用的效果
	//Client:是一个网络调用标志，它指示这个函数是从服务器调用并在客户端上执行的.用于在客户端执行与某些逻辑或显示效果相关的功能，例如播放动画、显示特效、更新 UI 等。
	//Reliable:确保网络调用不会丢失。如果网络连接较差或出现了数据包丢失的情况，使用 Reliable 的调用会被重新传输，直到客户端确认接收为止。
	UFUNCTION(Client,Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);

	//RPC:客户端更新能力状态
	UFUNCTION(Client,Reliable)
	void ClientUpdateAbilityStatus(const FGameplayTag& AbilityTag , const FGameplayTag& StatusTag , int32 AbilityLevel);
};
