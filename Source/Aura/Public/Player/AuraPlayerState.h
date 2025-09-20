// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AuraPlayerState.generated.h"

class ULevelUpInfo;
// 声明带有一个int32参数的多播委托（用于属性变更通知）
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLevelChanged, int32 /*StatValue*/, bool /*bLevelUp*/)

class UAbilitySystemComponent;
class UAttributeSet;
/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerState : public APlayerState,public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AAuraPlayerState();
	//// 声明 GetLifetimeReplicatedProps 函数，用于指定需要在网络上同步的属性
	// GetLifetimeReplicatedProps 这是一个用于 注册网络同步属性 的虚拟函数
	//你在服务器端修改某个属性时，这个函数会被用来指定哪些属性应该同步到客户端
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//重写IAbilitySystemInterface接口中的 事件
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//获取GAS属性集,直接返回AttributeSet
	UAttributeSet * GetAttributeSet()const{return AttributeSet;}

	//升级信息
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelUpInfo> LevelUpInfo;

	
	// XP变更委托
	FOnPlayerStatChanged OnXPChangedDelegate;
	// 等级变更委托
	FOnLevelChanged OnLevelChangedDelegate;
	//属性点变更委托
	FOnPlayerStatChanged OnAttributePointsChangedDelegate;
	//法术点变更委托
	FOnPlayerStatChanged OnSpellPointsChangedDelegate;

	
	//FORCEINLINE强制要求编译器将函数内联,NLINE编辑器自动判断是否将函数内联
	//这里的 FORCEINLINE 强制编译器将 GetPlayerLevel 函数 内联，无论函数的复杂度如何。它的作用和常规的 inline 类似，但是比普通 inline 更加强制性。
	//获取角色等级 
	FORCEINLINE int32 GetPlayerLevel()const{return Level;}

	// 获取当前经验值（内联函数优化性能）
	FORCEINLINE int32 GetXp()const{return Xp;}

	//获取属性点（FORCEINLINE内联函数优化性能）
	FORCEINLINE int32 GetAttributePoints()const{return AttributePoints;}
	//获取法术点（FORCEINLINE内联函数优化性能）
	FORCEINLINE int32 GetSpellPoints()const{return SpellPoints;}
	
	// 增加经验值（网络同步方法）
	void AddToXP(int32 InXP);
	// 提升等级（网络同步方法）
	void AddToLevel(int32 InLevel);
	//增加属性点
	void AddToAttributePoints(int32 InAttributePoints);
	//增加法术点
	void AddToSpellPoints(int32 InSpellPoints);
	// 直接设置经验值（网络同步方法）
	void SetXP(int32 InXP);
	// 直接设置等级（网络同步方法）
	void SetLevel(int32 InLevel);
	void SetAttributePoints(int32 InAttributePoints);
	void SetSpellPoints(int32 InSpellPoints);

	
protected:	
	//构建GAS组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	//构建GAS属性
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	virtual void BeginPlay() override;

private:
	// 创建角色等级变量，默认为1
	// 该变量在游戏中会随着角色升级而改变，且支持网络同步
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Level)
	int32 Level = 1;

	// 同步经验值（使用RepNotify）
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Xp)
	int32 Xp = 0;

	//属性点
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_AttributePoints)
	int32 AttributePoints = 0;

	//法术点
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_SpellPoints)
	int32 SpellPoints = 0;
	
	// 网络复制时，会调用这个函数，处理等级变化后的逻辑
	UFUNCTION()
	void OnRep_Level(int32 OldLevel);

	// 经验值同步回调（客户端执行）
	UFUNCTION()
	void OnRep_Xp(int32 OldXp);

	//属性点网络复制
	UFUNCTION()
	void OnRep_AttributePoints(int32 OldAttributePoints);

	//法术点网络复制
	UFUNCTION()
	void OnRep_SpellPoints(int32 OldSpellPoints);

};
