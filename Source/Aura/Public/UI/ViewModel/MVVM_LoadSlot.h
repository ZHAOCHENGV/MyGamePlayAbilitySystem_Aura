// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "MVVM_LoadSlot.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitcherIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnableSelectSlotButton, bool, bEnable);
/**
 * 
 */
UCLASS()
class AURA_API UMVVM_LoadSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FSetWidgetSwitcherIndex SetWidgetSwitcherIndex;

	UPROPERTY(BlueprintAssignable)
	FEnableSelectSlotButton EnableSelectSlotButton;
	
	//插槽状态
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SlotStatus;

	//初始化槽位
	void InitializeSlot();

	UPROPERTY()
	int32 SlotIndex;

	//设置加载槽名称
	void SetLoadSlotName(FString InLoadSlotName);
	//设置玩家名称
	void SetPlayerName(FString InPlayerName);
	//设置地图名称
	void SetMapName(FString InMapName);
	//获取加载槽名称
	FString GetLoadSlotName() const {return LoadSlotName;};
	//获取玩家名称
	FString GetPlayerName() const {return PlayerName;};
	//获取地图名称
	FString GetMapName() const {return MapName;};

private:
	/**
	 * @brief 声明一个用于存储存档槽位名称的字符串变量。
	 *
	 * @par 功能说明
	 * 这是一个在 C++ 头文件中声明的成员变量，通过 `UPROPERTY` 宏向 Unreal Engine 的反射系统注册。
	 * 这次注册赋予了该变量一系列强大的特性，使其能够被编辑器、蓝图和 MVVM 系统深度集成和访问。
	 * 每个修饰符都为这个变量开启了一项特定的功能。
	 *
	 * @par 修饰符详解
	 * - **EditAnywhere**: 允许在编辑器中查看和修改此属性，无论是在蓝图默认值编辑器还是在世界大纲视图的实例细节面板中。
	 * - **BlueprintReadWrite**: 将此属性暴露给蓝图，使得蓝图节点可以自由地读取（Get）和写入（Set）它的值。
	 * - **FieldNotify**: 这是 MVVM (Model-View-ViewModel) 框架的关键。它告知 MVVM 系统，这个属性值的变化是可以被“通知”的。当值改变时，任何绑定到它的 UI 控件都可以自动更新。它本身只是一个标记，需要配合 Setter 函数中的通知逻辑（如 `UE_MVVM_SET_PROPERTY_VALUE`）才能工作。
	 * - **Setter**: 指定一个自定义的 C++ Setter 函数。当蓝图尝试写入这个属性时，引擎不会直接修改变量的值，而是会调用你指定的函数（默认是 `SetLoadSlotName`）。这给了你完全的控制权，可以在赋值前后执行额外的逻辑，比如数据验证或广播通知。
	 * - **Getter**: 指定一个自定义的 C++ Getter 函数。当蓝图尝试读取这个属性时，引擎会调用你指定的函数（默认是 `GetLoadSlotName`）来获取值。这允许你在返回值之前进行一些计算或格式化。
	 * - **meta=(AllowPrivateAccess="true")**: 这是一个元数据（meta）标记。它的作用是“解锁” `private` 访问权限。通常，为了良好的封装性，成员变量会声明在 `private:` 区域。如果没有这个标记，`BlueprintReadWrite` 将无法访问私有变量并导致编译错误。这个标记告诉引擎：“尽管这个变量在 C++ 中是私有的，但我授权蓝图通过它的 Getter 和 Setter 来访问它。”
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta= (AllowPrivateAccess="true"));
	FString LoadSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta= (AllowPrivateAccess="true"));
	FString PlayerName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta= (AllowPrivateAccess="true"));
	FString MapName;
};
