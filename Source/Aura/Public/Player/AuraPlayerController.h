// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interation/EnemyInterface.h"
#include "GameplayTagContainer.h"
#include "AuraPlayerController.generated.h"


class UNiagaraSystem;
class UDamageTextComponent;
class USplineComponent;
class UAuraAbilitySystemComponent;
class UAuraInputConfig;
class AAuraHUD;
class UInputMappingContext;
class UInputAction;

//struct定义FInputActionValue这是一个结构体
struct FInputActionValue;

/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AAuraPlayerController();
	virtual void Tick(float DeltaTime) override;
	/*
	 *	显示伤害数值
	 *  UFUNCTION(Client, Reliable) 声明 网络函数（RPC）
	 *  Client ：表示这个函数是 客户端 RPC,即该函数在 服务器端被调用，然后会通过网络调用到 客户端 执行
	 *  Reliable ：表示该函数是 可靠的。也就是说，无论网络状况如何，确保这个函数在客户端执行时不会丢失或丢包。
	 */
	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit);
	
protected:
	virtual void BeginPlay() override;
	//设置输入组件
	virtual void SetupInputComponent() override;


private:
	//输入映射上下文
	UPROPERTY(EditAnywhere,Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	//移动输入
	UPROPERTY(EditAnywhere,Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	//移动事件,传入默认输入操作（InputActionValue）
	void Move(const  FInputActionValue & InputActionValue);

	//Shift输入
	UPROPERTY(EditAnywhere,Category = "Input")
	TObjectPtr<UInputAction> ShiftAction;

	//按下shift
	void ShiftPressed(){bShiftKeyDown = true;};
	//松开shift
	void ShiftReleased(){bShiftKeyDown = false;};
	bool bShiftKeyDown = false;
	
	//检查鼠标下演员
	void CursorTrace();
 

	TScriptInterface<IEnemyInterface>LastActor;
	TScriptInterface<IEnemyInterface>ThisActor;
	//创建光标击中参数
	FHitResult CursorHit;

	
	UPROPERTY(EditDefaultsOnly,Category = "Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	//技能输入 已按下
	void AbilityInputTagPressed(FGameplayTag InputTag);
	//技能输入 已释放
	void AbilityInputTagReleased(FGameplayTag InputTag);
	//技能输入 持续长按
	void AbilityInputTagHeld(FGameplayTag InputTag);

	//能力组件
	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	//获取能力组件
	UAuraAbilitySystemComponent * GetASC();

	//缓存目标地点
	FVector CachedDestination = FVector::ZeroVector;
	//按压时长
	float FollowTime = 0.0f;
	//短按时长
	float ShortPressThreshold = 0.5f;
	//自动行走
	bool bAutoRunning = false;
	//瞄准目标
	bool bTargeting = false;

	//自动接受到达半径
	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;

	//样条线
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;

	//自动移动
	void AutoRun();

	//伤害组件
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;

	//鼠标点击Niagara
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> ClickNiagaraSystem;
};
