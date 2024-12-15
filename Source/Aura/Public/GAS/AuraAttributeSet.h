// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "AuraAttributeSet.generated.h"

//这段宏定义通过组合多个功能性宏，快速为指定类和属性生成常见的访问器函数。

//GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) // 生成 Getter 方法以获取属性对象
//GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)           // 生成 Getter 方法以获取属性的当前值
//GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)               // 生成 Setter 方法以设置属性的值
//GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)                 // 生成初始化方法以设置初始值
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)           



USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()
	
	FEffectProperties() {} //析构函数

	//效果上下文
	FGameplayEffectContextHandle EffectContextHandle;
	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	UPROPERTY()
	AController* SourceController = nullptr;

	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	UPROPERTY()
	AController* TargetController = nullptr;

	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
	
};


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

	//属性更改函数，属性正式更改之前会调用这个函数，因此可以在此函数中限制值得数值
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	//Gameplay Effect执行后处理函数,该函数主要用于在Gameplay Effect（游戏效果）执行完成后，做一些特定的后处理操作
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	/*
	 * 主要属性
	 */
	//创建力量值
	// 声明一个只读的属性，支持蓝图和网络同步，标记为“Primary Attributes”分类
	// ReplicatedUsing=OnRep_Health 表示该变量在客户端更新时会调用 OnRep_Strength 函数
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_Strength,Category = "Primary Attributes")
	FGameplayAttributeData Strength;
	// 使用宏为 Strength 属性生成访问器函数
	// ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Strength) 会生成如下两个函数：
	// 1. GetStrength(): 用于获取 Strength 的值
	// 2. SetStrength(): 用于设置 Strength 的值
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,Strength);
	//创建智力
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_Intelligence,Category = "Primary Attributes")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,Intelligence);
	//创建复原力
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_Resilience,Category = "Primary Attributes")
	FGameplayAttributeData Resilience;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,Resilience);
	//活力
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_Vigor,Category = "Primary Attributes")
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet,Vigor);


	

	/*
	 * 重要属性
	 */
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




	//Rep_Health 方法会在 Health 属性在网络上复制并被修改时调用。
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
	
	//Rep_MaxHealth 方法会在 MaxHealth 属性在网络上复制并被修改时调用。
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;
	
	
	//Rep_Health 方法会在 Mana 属性在网络上复制并被修改时调用。
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;
	
	//Rep_Health 方法会在 MaxMana 属性在网络上复制并被修改时调用。
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;


	
	//在客户端力量值更新时会调用 OnRep_Strength 函数
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;
	
	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;
	
	UFUNCTION()
	void OnRep_Resilience(const FGameplayAttributeData& OldResilience) const;
	
	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;

private:
	
	//设置效果属性
	void SetEffectProperties(const struct FGameplayEffectModCallbackData& Data,FEffectProperties & Props)const;
};
