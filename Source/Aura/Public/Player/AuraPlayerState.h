// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AuraPlayerState.generated.h"

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
	//FORCEINLINE强制要求编译器将函数内联,NLINE编辑器自动判断是否将函数内联
	//这里的 FORCEINLINE 强制编译器将 GetPlayerLevel 函数 内联，无论函数的复杂度如何。它的作用和常规的 inline 类似，但是比普通 inline 更加强制性。
	//获取角色等级 
	FORCEINLINE int32 GetPlayerLevel()const{return Level;} 
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

	
	// 网络复制时，会调用这个函数，处理等级变化后的逻辑
	UFUNCTION()
	void OnRep_Level(int32 OldLevel);

	
};
