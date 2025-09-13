// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/MVVM_LoadScreen.h"

#include "UI/ViewModel/MVVM_LoadSlot.h"




/**
 * @brief 初始化加载界面中所有存档槽位的 ViewModel。
 *
 * @par 功能说明
 * 该函数负责在游戏启动或进入加载界面时，创建并设置所有存档槽位（Load Slot）所需的数据模型（ViewModel）。
 * 它使用 NewObject 动态创建 UMVVM_LoadSlot 实例，并将其存储在一个 TMap 中以便后续通过索引访问。
 *
 * @par 详细流程
 * 1.  为第一个槽位 (Index 0) 创建一个 UMVVM_LoadSlot 实例。
 * 2.  设置该实例的内部名称为 "LoadSlot_0"。
 * 3.  将这个实例添加到名为 `LoadSlots` 的 TMap 容器中，键为 0。
 * 4.  为第二个槽位 (Index 1) 重复上述过程。
 * 5.  为第三个槽位 (Index 2) 重复上述过程。
 *
 * @par 注意事项
 * - `NewObject<UMVVM_LoadSlot>(this, ...)` 中的 `this` 参数指定了当前 UMVVM_LoadScreen 对象作为新创建的 UMVVM_LoadSlot 对象的 "Outer"。这意味着它们的生命周期是绑定的，当 UMVVM_LoadScreen 被垃圾回收时，这些槽位对象也会被一并回收，可以有效防止内存泄漏。
 * - 当前实现是硬编码（Hard-coded）创建3个槽位，扩展性较差。
 * - 代码中存在一个明显的复制粘贴错误（见注释）。
 */
void UMVVM_LoadScreen::InitializeLoadSlots()
{

	// --- 第一个槽位 ---
	// (为什么这么做): NewObject 是 Unreal Engine 中动态创建 UObject 派生类实例的标准方法。
	// LoadSlotViewModelClass 应该是一个 TSubclassOf<UMVVM_LoadSlot> 类型的 UPROPERTY，允许在蓝图中指定要创建的具体 ViewModel 类。
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this,LoadSlotViewModelClass);
	LoadSlot_0->SetLoadSlotName("LoadSlot_0"); 
	LoadSlots.Add(0, LoadSlot_0);

	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this,LoadSlotViewModelClass);
	LoadSlot_1->SetLoadSlotName("LoadSlot_1");
	LoadSlots.Add(1, LoadSlot_1);

	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this,LoadSlotViewModelClass);
	LoadSlot_1->SetLoadSlotName("LoadSlot_2");
	LoadSlots.Add(2, LoadSlot_2);
}


/**
 * @brief 根据索引号安全地获取一个存档槽位的 ViewModel。
 * @param Index 要获取的槽位的索引（TMap 中的 Key）。
 * @return 指向对应 UMVVM_LoadSlot 实例的指针。如果找不到，程序会崩溃。
 *
 * @par 功能说明
 * 这是一个 getter 函数，提供通过索引访问已创建的槽位 ViewModel 的能力。
 * 它使用了 `FindChecked`，这是一个高性能但有风险的查找方法。
 *
 * @par 注意事项
 * - `const` 关键字表示此函数不会修改类的任何成员变量，是一个只读操作。
 * - `FindChecked` 假设传入的 `Index` 永远是有效的。如果 `LoadSlots` 中不存在该 `Index`，程序会立即断言失败并崩溃。这适用于调用者能100%保证索引合法的情况，否则应使用更安全的方法。
 */
UMVVM_LoadSlot* UMVVM_LoadScreen::GetLoadSlotViewModelByIndex(int32 Index) const
{
	// FindChecked 是 TMap 的一个成员函数，它会查找指定的 Key。如果找到，返回对应的 Value；
	// 如果找不到，它会触发一个断言（assertion），在开发版本中导致程序崩溃。这有助于及早发现逻辑错误。
	return LoadSlots.FindChecked(Index);
}

/**
 * @brief (占位函数) 处理当玩家在某个空槽位上点击“创建新存档”并输入名字后的逻辑。
 * @param Slot 发生操作的槽位索引。
 * @param EnteredName 玩家输入的新存档名称。
 */
void UMVVM_LoadScreen::NewSlotButtonPressed(int32 Slot, const FString& EnteredName)
{
}

/**
 * @brief 处理当玩家点击某个槽位上的“开始新游戏”按钮时的逻辑。
 * @param Slot 发生操作的槽位索引。
 *
 * @par 功能说明
 * 此函数用于触发对应槽位 UI 的状态变更。它通过广播一个委托来通知视图（View）进行更新。
 *
 * @par 详细流程
 * 1. 使用 `operator[]` 从 `LoadSlots` TMap 中获取指定索引的 `UMVVM_LoadSlot` 实例。
 * 2. 访问该实例的 `SetWidgetSwitcherIndex` 委托。
 * 3. 调用 `Broadcast(1)` 来执行所有绑定到该委托的函数，并传递参数 `1`。
 */
void UMVVM_LoadScreen::NewGameButtonPressed(int32 Slot)
{
	// (为什么这么做): 这里的 SetWidgetSwitcherIndex 明显是一个委托（Delegate），很可能是 FMulticastInlineDelegate。
	// 在 MVVM 模式中，ViewModel 通过广播委托来通知 View（UMG 蓝图）更新自身状态，而无需直接持有 View 的引用，实现了“单向数据流”和解耦。
	// `Broadcast(1)` 很可能是在通知对应的 UMG 控件将其内部的 Widget Switcher 切换到索引为 1 的子控件（例如，从“新游戏”按钮切换到“请输入存档名”的输入框）。
	LoadSlots[Slot]->SetWidgetSwitcherIndex.Broadcast(1);
}


void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 Slot)
{
}
