// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/LoadScreenHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/ViewModel/MVVM_LoadScreen.h"
#include "UI/Widget/LoadScreenWidget.h"


/**
 * @brief 在 HUD Actor 进入游戏世界时被调用，负责创建加载界面的 ViewModel 和 Widget。
 *
 * @par 功能说明
 * 这是加载流程的起点。该函数在游戏开始播放（或关卡加载完成）时执行。
 * 它的核心职责是：
 * 1.  创建并初始化加载界面的主 ViewModel (`UMVVM_LoadScreen`)。
 * 2.  创建并显示加载界面的主 UI Widget (`ULoadScreenWidget`)。
 * 3.  将 ViewModel 与 Widget 关联起来（隐式或显式）。
 * 4.  触发从磁盘加载现有存档数据的操作。
 *
 * @par 详细流程
 * 1.  **调用父类实现**: `Super::BeginPlay()` 确保了父类 `AHUD` 的初始化逻辑能被正确执行，这是 Unreal Engine 中覆盖函数的标准做法。
 * 2.  **创建 ViewModel**: 使用 `NewObject` 创建 `UMVVM_LoadScreen` 的实例。`this` (即 `ALoadScreenHUD` 实例) 作为其 Outer，保证了它们的生命周期同步。
 * 3.  **初始化槽位**: 调用 ViewModel 的 `InitializeLoadSlots()` 函数，在内存中创建所有存档槽位的数据表示（子 ViewModel）。
 * 4.  **创建 Widget**: 使用 `CreateWidget` 创建 `ULoadScreenWidget` 的实例。这是创建 UMG (Unreal Motion Graphics) 控件的标准方法。
 * 5.  **显示 Widget**: 调用 `AddToViewport()` 将创建的 Widget 添加到玩家的屏幕上，使其可见。
 * 6.  **初始化 Widget (蓝图层)**: `BlueprintInitializeWidget()` 是一个自定义或框架内的函数调用，很可能是为了确保 Widget 内部，特别是其 ViewModel 的绑定逻辑能够被执行。
 * 7.  **加载存档数据**: 调用 ViewModel 的 `LoadData()` 函数，命令其从磁盘读取所有存档槽位的数据，并通过 MVVM 的通知机制更新刚刚创建的 UI。
 *
 * @par 注意事项
 * - `LoadScreenViewModelClass` 和 `LoadScreenWidgetClass` 是 `TSubclassOf` 类型的 `UPROPERTY`，必须在蓝图中被正确指定，否则 `NewObject` 或 `CreateWidget` 会失败。
 * - 代码的执行顺序非常重要：必须先创建 ViewModel，再创建 Widget，然后将它们关联（通过 `BlueprintInitializeWidget` 或直接设置），最后才加载数据。这样可以确保 UI 准备好接收数据更新的通知。
 */
void ALoadScreenHUD::BeginPlay()
{// 步骤 1/4: 调用父类的 BeginPlay 函数。这是一个必须的步骤，以确保引擎的底层初始化能够完成。
	Super::BeginPlay();

	// 步骤 2/4: 创建并初始化 ViewModel
	// 使用 NewObject 创建 ViewModel 实例。LoadScreenViewModelClass 是一个 UPROPERTY，允许在蓝图中指定要创建的 ViewModel 的具体类。
	LoadScreenViewModel = NewObject<UMVVM_LoadScreen>(this,LoadScreenViewModelClass);
	LoadScreenViewModel->InitializeLoadSlots();// 调用 ViewModel 的函数来创建并初始化内部的存档槽位数据。

	// 步骤 3/4: 创建并显示 UI Widget
	// 使用 CreateWidget 创建 UMG 控件。LoadScreenWidgetClass 同样是在蓝图中指定的 UMG 控件类。
	LoadScreenWidget = CreateWidget<ULoadScreenWidget>(GetWorld(), LoadScreenWidgetClass);
	LoadScreenWidget->AddToViewport();// 将创建的 Widget 添加到屏幕视口，使其对玩家可见。
	
	// (为什么这么做): 这一步很关键。通常 UMG 的 ViewModel 绑定是在其内部的 Initialize 事件中完成的。
	// 如果 ULoadScreenWidget 类中覆盖了 Initialize 函数，并把 SetViewModel 的逻辑放在里面，那么调用这个函数就是为了确保 ViewModel 被设置到 Widget 中。
	LoadScreenWidget->BlueprintInitializeWidget();

	// 步骤 4/4: 加载数据并更新 UI
	// 在 Widget 和 ViewModel 都准备就绪后，调用 LoadData()。
	// 该函数会从磁盘读取存档，并通过 MVVM 的 FieldNotify 机制自动更新 Widget 上的显示。
	LoadScreenViewModel->LoadData();
	
}
