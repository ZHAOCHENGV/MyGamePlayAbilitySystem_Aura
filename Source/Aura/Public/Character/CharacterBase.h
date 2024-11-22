// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"
//UCLASS(Abstract) 表示该类是一个 抽象类，不能直接被实例化。
//通常抽象类会提供一些通用逻辑或接口，要求子类实现具体的行为。
UCLASS(Abstract)
class AURA_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	//构造函数
	ACharacterBase();

protected:
	//游戏开始
	virtual void BeginPlay() override;
	//武器
	UPROPERTY(EditAnywhere,Category="Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;
	
	

};
