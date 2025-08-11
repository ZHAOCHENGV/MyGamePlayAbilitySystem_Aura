// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AuraInputConfig.h"
#include "EnhancedInputComponent.h"
#include "AuraInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	// 定义一个模板函数，用于绑定输入操作与对应的委托函数
	// 通过模板支持不同类型的对象和函数
	// UserClass：要绑定的类类型，通常是拥有回调函数的类。
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityActions(
		const UAuraInputConfig* InputConfig,// 输入配置，定义了所有输入动作和标签
		UserClass* Object, // 用户定义的类实例，将用于绑定函数
		PressedFuncType PressedFunc,// 按下操作的回调函数
		ReleasedFuncType ReleasedFunc, // 松开操作的回调函数
		HeldFuncType HeldFunc// 持续操作的回调函数
		);  
	
};

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void UAuraInputComponent::BindAbilityActions(const UAuraInputConfig* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	// 检查输入配置是否有效，如果无效直接崩溃（调试时使用）
	check(InputConfig);
	// 遍历输入配置中的所有输入动作及其绑定的标签
	for (const FAuraInputAction& Action : InputConfig->AbilityInputActions)
	{
		// 检查当前输入动作是否有效，以及对应的输入标签是否有效
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			// 如果提供了 PressedFunc（按下操作的函数），则绑定到对应的触发事件
			if (PressedFunc)
			{
				BindAction(
					Action.InputAction,// 输入动作（UInputAction）
					ETriggerEvent::Started,// 触发事件类型（按下开始）
					Object,// 函数绑定的对象实例
					PressedFunc, // 按下操作的回调函数
					Action.InputTag// 输入标签，传递给回调函数
					);
			}
			// 如果提供了 ReleasedFunc（松开操作的函数），则绑定到对应的触发事件
			if (ReleasedFunc)
			{
				BindAction(
					Action.InputAction,
					ETriggerEvent::Completed,// 触发事件类型（按键完成/松开）
					Object,
					ReleasedFunc,// 松开操作的回调函数
					Action.InputTag
					);
			}
			// 如果提供了 HeldFunc（持续按住操作的函数），则绑定到对应的触发事件
			if (HeldFunc)
			{
				BindAction(
					Action.InputAction,
					ETriggerEvent::Triggered,// 触发事件类型（持续触发）
					Object,
					HeldFunc,// 持续操作的回调函数
					Action.InputTag
					);
			}
		}
	}
}

