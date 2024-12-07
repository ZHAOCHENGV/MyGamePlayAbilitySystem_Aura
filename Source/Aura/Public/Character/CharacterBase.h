// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"


class UAbilitySystemComponent;
class UAttributeSet;

//UCLASS(Abstract) 表示该类是一个 抽象类，不能直接被实例化。
//通常抽象类会提供一些通用逻辑或接口，要求子类实现具体的行为。
//public  IAbilitySystemInterface  接口属于GAS原始存在的接口，不需要自行创建
UCLASS(Abstract)
class AURA_API ACharacterBase : public ACharacter,public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	//构造函数
	ACharacterBase();
	//重写IAbilitySystemInterface接口中的 事件
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//获取GAS属性集,直接返回AttributeSet
	UAttributeSet * GetAttributeSet()const{return AttributeSet;}
protected:
	//游戏开始
	virtual void BeginPlay() override;
	//武器
	UPROPERTY(EditAnywhere,Category="Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	//构建GAS组件
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	//构建GAS属性
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

};
