// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GAS/Data/CharacterClassInfo.h"
#include "GAS/Passive/PassiveNiagaraComponent.h"
#include "Interation/CombatInterface.h"
#include "CharacterBase.generated.h"


class UDebuffNiagaraComponent; 
class UNiagaraSystem;
class UGameplayAbility;
class UGameplayEffect;
class UAbilitySystemComponent;
class UAttributeSet;
class UAnimMontage;

//UCLASS(Abstract) 表示该类是一个 抽象类，不能直接被实例化。
//通常抽象类会提供一些通用逻辑或接口，要求子类实现具体的行为。
//public  IAbilitySystemInterface  接口属于GAS原始存在的接口，不需要自行创建
UCLASS(Abstract)
class AURA_API ACharacterBase : public ACharacter,public IAbilitySystemInterface,public ICombatInterface
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	//构造函数
	ACharacterBase();
	//重写IAbilitySystemInterface接口中的 事件
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//获取GAS属性集,直接返回AttributeSet
	UAttributeSet * GetAttributeSet()const{return AttributeSet;}
	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	//多播函数 Death（客户端和服务器都要执行死亡相关逻辑）
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath();

	/*Combatinterface接口开始*/
	//重新接口获取蒙太奇事件
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;
	
	//实现接口中的函数，获取攻击插槽位置
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) override;
	
	//死亡事件
	virtual void Die() override;

	//是否死亡
	virtual bool IsDead_Implementation() const override;

	//获取Avatar
	virtual AActor* GetAvatar_Implementation() override;

	//获取血液特效
	virtual UNiagaraSystem* GetBloodEffect_Implementation() override;

	//按标签获取标记的蒙太奇
	virtual FTaggedMontage GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag) override;

	//获取仆从数量
	virtual int32 GetMinionCount_Implementation() override;

	//修改仆从数量
	virtual void IncrementMinionCount_Implementation(int32 Amount) override;

	//重载获取角色类函数
	virtual ECharacterClass GetCharacterClass_Implementation() override;

	// ASC 注册委托
	virtual FOnASCRegistered& GetOnASCRegisteredDelegate() override;

	// 死亡 注册委托
	virtual FOnDeathSignature& GetOnDeathSignatureDelegate() override;
	
	//获取武器
	virtual USkeletalMeshComponent* GetWeapon_Implementation() override;

	virtual void SetIsBeingShocked_Implementation(bool bInShock) override;
	
	virtual bool IsBeingShocked_Implementation() const override;

	virtual FOnDamageSignature& GetOnDamageSignature() override;
	
	/*Combatinterface接口结束*/

	//当ASC注册时
	FOnASCRegistered OnAscRegistered;
	FOnDeathSignature OnDeathDelegate;
	FOnDamageSignature OnDamageDelegate;

	//攻击蒙太奇数组
	UPROPERTY(EditAnywhere, Category="Combat")
	TArray<FTaggedMontage> AttackMontages;

	//获取攻击蒙太奇
	virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() override;

	//是否眩晕
	UPROPERTY(ReplicatedUsing=OnRep_Stunned, BlueprintReadOnly)
	bool bIsStunned = false;

	//是否燃烧
	UPROPERTY(ReplicatedUsing=OnRep_Burned, BlueprintReadOnly)
	bool bIsBurned = false;

	//眩晕中
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsBeingShocked = false;
	
	//具有复制功能的晕眩
	UFUNCTION()
	virtual void OnRep_Stunned();
	//具有复制功能的燃烧
	UFUNCTION()
	virtual void OnRep_Burned();
	
	//获取生命周期复制的 props (bIsStunned)
	//注意：如有变量为Replicated，则必须GetLifetimeReplicatedProps
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const ;

	//设置角色职业
	void SetCharacterClass(ECharacterClass InClass) {CharacterClass = InClass;};
protected:
	//游戏开始
	virtual void BeginPlay() override;

	//初始化 能力Actor信息集
	virtual void InitAbilityActorInfo();
	
	//武器
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	//武器插槽
	UPROPERTY(EditAnywhere,Category="Combat")
	FName WeaponTipSocketName;

	//右手插槽
	UPROPERTY(EditAnywhere,Category="Combat")
	FName RightHandSocketName;
	
	//左手插槽
	UPROPERTY(EditAnywhere,Category="Combat")
	FName LeftHandSocketName;
	
	//尾部插槽
	UPROPERTY(EditAnywhere,Category="Combat")
	FName TailSocketName;

	//构建GAS组件
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	//构建GAS属性
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	//创建初始化属性游戏效果
	UPROPERTY(BlueprintReadOnly,EditAnywhere,Category="Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;
	//创建默认次要属性
	UPROPERTY(BlueprintReadOnly,EditAnywhere,Category="Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;
	//创建默认次要属性
	UPROPERTY(BlueprintReadOnly,EditAnywhere,Category="Attributes")
	TSubclassOf<UGameplayEffect> DefaultSignificantAttributes;
	
	//初始化默认属性
	virtual void InitializeDefaultAttributes() const;
	//应用游戏效果到自身
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GamePlayEffectClass,float Level)const;

	//添加角色能力组件
	void AddCharacterAbilities();

	

	/*
	 * 消散
	 */

	void Dissolve();

	//BlueprintImplementableEvent：声明一个函数，C++ 不提供实现，逻辑完全由蓝图来定义。
	//溶解时间轴事件
	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeLine(UMaterialInstanceDynamic* DynamicMaterialInstance);
	//BlueprintImplementableEvent：声明一个函数，C++ 不提供实现，逻辑完全由蓝图来定义。
	//溶解时间轴事件
	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeLine(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

	//死亡变量
	bool bDead = false;

	//血液特效
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* BloodEffect;

	//死亡音效
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* DeathSound;

	/* 仆从 */
	int32 MinionCount = 0;

	//职业
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;

	//燃烧减益组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDebuffNiagaraComponent> BurnDebuffComponent;

	//眩晕减益组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDebuffNiagaraComponent> StunDebuffComponent;

	//移动速度
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Combat")
	float BaseWalkSpeed = 600.f;

	//当眩晕标签已更改时
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
	
private:
	//启动技能数组
	UPROPERTY(EditAnywhere,Category="Attributes")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	//被动技能数组
	UPROPERTY(EditAnywhere,Category="Attributes")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;

	//被击蒙太奇
	UPROPERTY(EditAnywhere,Category="Combat")
	TObjectPtr<UAnimMontage> HitReactMontage ;

	//保护光环Niagara组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> HaloOfProtectionNiagaraComponent;

	//生命吸取Niagara组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> LifeSiphonNiagaraComponent;

	//法力吸取Niagara组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> ManaSiphonNiagaraComponent;

	//效果附加组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> EffectAttachComponent;
};
