// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "AuraAttributeSet.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	//构造函数
	UAuraAttributeSet();

	//这是一个虚拟函数，用于定义哪些属性需要在网络中进行复制（replication）。
	//通过重写这个函数，开发者可以指定哪些属性在网络同步时需要被管理。
	// 是 Unreal Engine 用于存储和管理需要复制的属性的数组。
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//属性更改函数，可以在此函数中限制值得数值
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	//创建健康值 ,ReplicatedUsing = OnRep_Health 指定当该属性在网络上被复制时，它会调用 OnRep_Health 方法来通知任何相关的对象或系统。
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_Health,Category = "Vital Attributes")
	FGameplayAttributeData Health;
	//属性器访问。
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,Health);
	
	//创建最大健康值 ,ReplicatedUsing = OnRep_MaxHealth,指定当该属性在网络上被复制时，它会调用 OnRep_Health 方法来通知任何相关的对象或系统。
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_MaxHealth,Category = "Vital Attributes")
	FGameplayAttributeData MaxHealth;
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,MaxHealth);

	//Rep_Health 方法会在 Health 属性在网络上复制并被修改时调用。
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
	
	//Rep_Health 方法会在 MaxHealth 属性在网络上复制并被修改时调用。
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;


	//创建魔法值 ,ReplicatedUsing = OnRep_Health 指定当该属性在网络上被复制时，它会调用 OnRep_Health 方法来通知任何相关的对象或系统。
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_Mana,Category = "Vital Attributes")
	FGameplayAttributeData Mana;
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,Mana);
	//创建最大魔法值 ,ReplicatedUsing = OnRep_MaxHealth,指定当该属性在网络上被复制时，它会调用 OnRep_Health 方法来通知任何相关的对象或系统。
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_MaxMana,Category = "Vital Attributes")
	FGameplayAttributeData MaxMana;
	//这个宏会自动创建初始化，get,set，等方法
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,MaxMana);


	
	//Rep_Health 方法会在 Mana 属性在网络上复制并被修改时调用。
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;
	
	//Rep_Health 方法会在 MaxMana 属性在网络上复制并被修改时调用。
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;
	
};
