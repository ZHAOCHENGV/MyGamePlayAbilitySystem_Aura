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
	//重写IAbilitySystemInterface接口中的 事件
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//获取GAS属性集,直接返回AttributeSet
	UAttributeSet * GetAttributeSet()const{return AttributeSet;}
protected:	
	//构建GAS组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	//构建GAS属性
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	virtual void BeginPlay() override;

};
