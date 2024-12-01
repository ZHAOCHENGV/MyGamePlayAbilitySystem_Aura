// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "AuraCharacter.generated.h"

class AAuraHUD;
/**
 * 
 */
UCLASS()
class AURA_API AAuraCharacter : public ACharacterBase
{
	GENERATED_BODY()
public:
	AAuraCharacter();
	//重写 PossessedBy ，在一个角色（Pawn）被新的控制器（Controller）接管时调用。
	virtual void PossessedBy(AController* NewController) override;
	//重写 OnRep_PlayerState ，在网络同步时，PlayerState 属性被复制（Replicated）到客户端时调用。
	virtual void OnRep_PlayerState() override;

private:
	//设置拥有者Owner Actor和Avater actor 
	void InitAbilityActorInfo();

	
};
