// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadSlot::InitializeSlot()
{
	SetWidgetSwitcherIndex.Broadcast(1);
}


/**
 * @brief 设置存档槽位的名称，并通过 MVVM 系统通知任何绑定的 UI 控件进行更新。
 *
 * @param InLoadSlotName 要为存档槽位设置的新名称。
 *
 * @par 功能说明
 * 这是一个 Setter 函数，但它并非简单的变量赋值。它使用了 Unreal Engine 的 MVVM 框架核心宏 `UE_MVVM_SET_PROPERTY_VALUE`。
 * 这个宏是实现“数据驱动 UI”的关键，它会自动处理检查值是否变化、更新值以及广播通知这三个步骤。
 *
 * @par 详细流程
 * 1.  **比较值**：宏内部首先会比较当前的 `LoadSlotName` 成员变量值与传入的 `InLoadSlotName` 值。
 * 2.  **条件性赋值**：仅当两个值不相同时，它才会将 `InLoadSlotName` 的值赋给 `LoadSlotName`。
 * 3.  **广播通知**：赋值完成后，宏会立即通过 MVVM 的 Field-Notify（字段通知）系统广播一个“属性已变更”的通知。
 * 4.  **UI 更新**：任何在 UMG (Unreal Motion Graphics) 编辑器中与 `LoadSlotName` 属性绑定的控件（例如一个 TextBlock），都会监听到这个通知，并自动获取新值来更新自己的显示内容。
 *
 * @par 注意事项
 * - 为了让这个宏正常工作，`LoadSlotName` 成员变量必须在头文件 (.h) 中被正确声明，需要使用 `UPROPERTY` 宏，并且最关键的是要有 `FieldNotify` 关键字。例如：`UPROPERTY(BlueprintReadWrite, FieldNotify, meta=(AllowPrivateAccess = "true"))`。
 * - 绝对不要在其他地方使用 `LoadSlotName = "SomeValue";` 这样的直接赋值方式，因为那会绕过通知系统，导致 UI 不会更新。所有对该属性的修改都应通过这个 Setter 函数。
 */
void UMVVM_LoadSlot::SetLoadSlotName(FString InLoadSlotName)
{

	// (为什么这么做): 这是 MVVM 模式的核心。我们不直接操作 UI，而是改变数据（ViewModel 的属性），
	// 然后由系统去通知 UI “嘿，你所关心的数据变了，快更新一下你的样子”。这实现了数据与表现的解耦。
	// 这个宏会先检查 InLoadSlotName 是否与当前的 LoadSlotName 相同，如果不同，才会赋值并广播通知。
	// 这样做可以避免不必要的 UI 刷新和潜在的无限循环。
	UE_MVVM_SET_PROPERTY_VALUE(LoadSlotName, InLoadSlotName);
}
