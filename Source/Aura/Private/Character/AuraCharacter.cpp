// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

AAuraCharacter::AAuraCharacter()
{
	//开启运动方向旋转
	GetCharacterMovement()->bOrientRotationToMovement = true;
	//设置旋转速度
	GetCharacterMovement()->RotationRate = FRotator(0, 400.f, 0);
	//约束到平面
	GetCharacterMovement()->bConstrainToPlane = true;
	//在开始时对齐平面
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	
	//使用控制器旋转Pitch关闭
	bUseControllerRotationPitch = false;
	//使用控制器旋转Roll关闭
	bUseControllerRotationRoll = false;
	//使用控制器旋转Yaw关闭
	bUseControllerRotationYaw = false;
}


