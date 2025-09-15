// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/MVVM_LoadSlot.h"


/**
 * @brief 根据当前槽位的状态（`SlotStatus`）来刷新并同步其对应的 UI 显示。
 *
 * @par 功能说明
 * 该函数是连接槽位数据状态和 UI 视觉表现的桥梁。它被调用时，会读取 `SlotStatus` 这个枚举变量的值，
 * 并通过广播一个委托（Delegate）来通知绑定的 UMG Widget 更新其外观。
 * 这通常用于切换一个 Widget Switcher 中的子控件，以显示“空槽位”、“已有存档”、“存档损坏”等不同的界面。
 *
 * @par 详细流程
 * 1.  **获取状态索引**: 读取 `SlotStatus` 枚举变量的底层整数值。例如，如果 `ESaveSlotStatus` 枚举被定义为 `Vacant=0`, `Taken=1`，那么 `SlotStatus.GetValue()` 就会返回 0 或 1。
 * 2.  **广播通知**: 调用 `SetWidgetSwitcherIndex` 这个多播委托的 `Broadcast` 函数，将上一步获取到的整数索引作为参数传递出去。
 * 3.  **UI 响应**: 任何绑定到此委托的 UMG 控件中的函数都会被执行，接收到这个索引值，并用它来设置 Widget Switcher 的 Active Index，从而切换显示的子控件。
 *
 * @par 注意事项
 * - 此函数的正确工作，强依赖于 C++ 中 `ESaveSlotStatus` 枚举的整数顺序与 UMG 蓝图中 `WidgetSwitcher` 控件的子控件（child widget）顺序完全一致。
 * - `SetWidgetSwitcherIndex` 委托必须在对应的 UMG 控件蓝图（View）中被成功绑定，否则 `Broadcast` 将不会产生任何效果。
 */
void UMVVM_LoadSlot::InitializeSlot()
{
	// (为什么这么做): SlotStatus 是一个 TEnumAsByte<ESaveSlotStatus> 类型的枚举。
	// .GetValue() 会返回该枚举值对应的底层整数值（例如，0, 1, 2...）。
	// 这个整数值将直接用作 Widget Switcher 的索引。
	const int32 WidgetSwitcherIndex = SlotStatus.GetValue();

	// (为什么这么做): 这是典型的 ViewModel -> View (视图) 的单向通知。
	// ViewModel 不知道也不关心 UI 是什么样子，它只负责说：“我的状态索引是 X”。
	// 任何对这个“信号”感兴趣的 View 都可以订阅（Bind）这个委托，并根据接收到的索引 X 来更新自己。
	SetWidgetSwitcherIndex.Broadcast(WidgetSwitcherIndex);
}



void UMVVM_LoadSlot::SetLoadSlotName(FString InLoadSlotName)
{

	// (为什么这么做): 这是 MVVM 模式的核心。我们不直接操作 UI，而是改变数据（ViewModel 的属性），
	// 然后由系统去通知 UI “嘿，你所关心的数据变了，快更新一下你的样子”。这实现了数据与表现的解耦。
	// 这个宏会先检查 InLoadSlotName 是否与当前的 LoadSlotName 相同，如果不同，才会赋值并广播通知。
	// 这样做可以避免不必要的 UI 刷新和潜在的无限循环。
	UE_MVVM_SET_PROPERTY_VALUE(LoadSlotName, InLoadSlotName);
}

void UMVVM_LoadSlot::SetPlayerName(FString InPlayerName)
{
	UE_MVVM_SET_PROPERTY_VALUE(PlayerName, InPlayerName);
}

void UMVVM_LoadSlot::SetMapName(FString InMapName)
{
	UE_MVVM_SET_PROPERTY_VALUE(MapName, InMapName);
}
