// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Interation/EnemyInterface.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "EnemyCharacter.generated.h"

class AEnemyAIController;
class UWidgetComponent;
class UBehaviorTree;
/**
 * 
 */
UCLASS()
class AURA_API AEnemyCharacter : public ACharacterBase,public IEnemyInterface
{
	GENERATED_BODY()
public:

	//控制时
	virtual void PossessedBy(AController* NewController) override;
	AEnemyCharacter();
	/** Enemy 接口函数*/
	//重载接口的 突出演员 事件
	virtual void HigHlihtActor()override ;
	//重载接口的 不突出演员 事件
	virtual void UnHigHlightActor()override;
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;
	/** 结束Enemy 接口函数*/

	/** Combat 接口函数*/
	virtual int32 GetPlayerLevel_Implementation()override;
	virtual void Die() override;
	/** 结束Combat 接口函数*/

	
	//HIT React 标签已更改
	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	//战斗目标
	UPROPERTY(BlueprintReadWrite,Category="Combat")
	TObjectPtr<AActor>CombatTarget;
	
	//是否击中
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;

	//声明周期
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Combat")
	float LifeSpan = 5.f;
protected:
	virtual void BeginPlay() override;
	//初始化 能力Actor信息集
	virtual void InitAbilityActorInfo() override;
	
	//初始化默认属性
	virtual void InitializeDefaultAttributes() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class Defaults")
	int32 Level= 1;

	//血条
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;

	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;

	//行为树
	UPROPERTY(EditAnywhere, Category="AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	//敌人AI控制器
	UPROPERTY()
	TObjectPtr<AEnemyAIController> EnemyAIController;

	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount) override;
	
};
