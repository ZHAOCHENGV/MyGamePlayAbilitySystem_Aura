// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interation/EnemyInterface.h"
#include "AuraPlayerController.generated.h"


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

	//检查鼠标下演员
	void CursorTrace();
 

	TScriptInterface<IEnemyInterface>LastActor;
	TScriptInterface<IEnemyInterface>ThisActor;

};
