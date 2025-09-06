// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#define是 C++ 预处理器指令，用于定义宏。宏是一种符号替换机制。
//把CUSTOM_DEPTH_RED 设置为250
#define CUSTOM_DEPTH_RED 250

//自定义碰撞
#define ECC_Projectile ECollisionChannel::ECC_GameTraceChannel1
#define ECC_Target ECollisionChannel::ECC_GameTraceChannel2
#define ECC_ExcludePlayers ECollisionChannel::ECC_GameTraceChannel3
