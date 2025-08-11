// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "AudioMixerBlueprintLibrary.h"
#include "AuraGamePlayTags.h"
#include "GAS/AuraAbilitySystemComponent.h"
#include "GAS/Data/AbilityInfo.h"
#include "Player/AuraPlayerState.h"

void USpellMenuWidgetController::BroadcastInitialValues()
{
	//初始化能力
	BroadcastAbilityInfo();
	SpellPointsChanged.Broadcast(GetAuraPS()->GetSpellPoints());
}

/**
 * 绑定控件依赖的数据变化回调（技能状态与技能点数）
 *
 * 功能说明：
 * 该方法将控件内部回调与关键依赖事件进行绑定，实现数据驱动的UI响应机制。
 * 包括：技能状态变更事件和技能点数变更事件。当技能或技能点发生变化时，
 * 自动触发控件逻辑，刷新按钮使能状态和技能信息，保持UI与数据实时同步。
 *
 * 详细说明：
 * - 首先获取Aura能力系统组件（Ability System Component），
 *   绑定AbilityStatusChanged事件，使用Lambda表达式捕获控件实例（this）。
 *   - 若当前选中技能与变更事件技能一致，则更新SelectedAbility的状态，
 *     并调用ShouldEnableButtons判定按钮可用性，通过SpellGlobeSelectedDelegate广播按钮状态，驱动UI刷新。
 *   - 若技能数据资产（AbilityInfo）有效，则查找对应技能信息并更新其状态标签，
 *     通过AbilityInfoDelegate广播最新技能信息，驱动技能球的UI刷新。
 * - 获取Aura玩家状态（AuraPlayerState），
 *   绑定OnSpellPointsChangedDelegate事件，使用Lambda回调。
 *   - 当技能点数变化时，通过SpellPointsChanged委托广播新技能点数，并更新当前技能点。
 *   - 根据最新技能状态与技能点数，判定按钮可用性，然后通过SpellGlobeSelectedDelegate广播，驱动UI同步。
 * - 本方法通过事件驱动、委托广播，实现了技能和技能点变化时UI的自动响应与解耦，提升了界面交互体验。
 */
void USpellMenuWidgetController::BindCallbacksToDependencies()
{
    // 获取Aura能力系统组件，绑定技能状态变更事件
    GetAuraASC()->AbilityStatusChanged.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 NewLevel)
    {
        // 如果当前选中技能与事件变更技能一致，刷新按钮状态并广播按钮使能状态
        if (SelectedAbility.AbilityTag.MatchesTagExact(AbilityTag))
        {
            SelectedAbility.Status = StatusTag;
            bool bEnableSpendPoints = false;
            bool bEnableEquip = false;
            ShouldEnableButtons(StatusTag, CurrentSpellPoints, bEnableSpendPoints, bEnableEquip);
        	
			FString Description;
        	FString NextLevelDescription;
        	GetAuraASC()->GetDescriptionsByAbilityTag(AbilityTag,Description,NextLevelDescription);
            SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
        }

        // 如果技能数据资产有效，更新技能信息并广播给UI
        if (AbilityInfo)
        {
            // 查找目标技能的配置信息
            FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
            // 更新技能状态标签
            Info.StatusTag = StatusTag;
            // 广播技能信息变化，驱动技能球UI刷新
            AbilityInfoDelegate.Broadcast(Info);
        }
    });
	//绑定委托
	GetAuraASC()->AbilityEquipped.AddUObject(this, &USpellMenuWidgetController::OnAbilityEquipped);
	
    // 获取Aura玩家状态，绑定技能点数变化事件
    GetAuraPS()->OnSpellPointsChangedDelegate.AddLambda([this](int32 SpellPoint)
    {
        // 广播技能点变化，刷新UI
        SpellPointsChanged.Broadcast(SpellPoint);
        // 更新当前技能点数
        CurrentSpellPoints = SpellPoint;

        // 根据最新技能状态与技能点数，刷新按钮使能状态并广播
        bool bEnableSpendPoints = false;
        bool bEnableEquip = false;
        ShouldEnableButtons(SelectedAbility.Status, CurrentSpellPoints, bEnableSpendPoints, bEnableEquip);
    	FString Description;
		FString NextLevelDescription;
		GetAuraASC()->GetDescriptionsByAbilityTag(SelectedAbility.AbilityTag,Description,NextLevelDescription);
		SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);

    });
}


/**
 * 处理法术球（技能图标）选择事件
 * 
 * @param AbilityTag 被选择的技能标签
 * 
 * 功能说明：
 * 1. 处理装备选择模式下的特殊逻辑
 * 2. 更新当前选中的技能信息
 * 3. 计算按钮可用状态
 * 4. 广播UI更新事件
 * 
 * 调用场景：
 * - 玩家在技能菜单中选择一个技能图标
 * - 切换技能选择时
 */
void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
    // === 1. 装备选择模式处理 ===
    if (bWaitingForEquipSelection)
    {
        // 获取新选择技能的类型（如主动/被动）
        const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType;
        
        // 广播停止等待装备选择事件（通知UI更新）
        StopWaitForEquipDelegate.Broadcast(SelectedAbilityType);
        
        // 结束装备选择模式
        bWaitingForEquipSelection = false;

        // 获取新选择技能的状态
        const FGameplayTag SelectedStatus = GetAuraASC()->GetStatusFromAbilityTag(AbilityTag);
        
        // 如果新选择技能是已装备状态，获取其输入槽位
        if (SelectedStatus.MatchesTagExact(FAuraGamePlayTags::Get().Abilities_Status_Equipped))
        {
            // 存储选中技能的输入槽位（用于后续装备逻辑）
            SelectedSlot = GetAuraASC()->GetInputFromAbilityTag(SelectedAbility.AbilityTag);
        }
    }

    // === 2. 获取全局标签和玩家数据 ===
    // 获取游戏标签单例（包含常用标签如Abilities_Status_Locked等）
    const FAuraGamePlayTags GamePlayTags = FAuraGamePlayTags::Get();
    
    // 获取玩家当前拥有的技能点数
    const int32 SpellPoints = GetAuraPS()->GetSpellPoints();
    
    // 初始化技能状态标签
    FGameplayTag AbilityStatus;

    // === 3. 验证技能标签有效性 ===
    // 检查标签是否有效（非空）
    const bool bTagValid = AbilityTag.IsValid();
    
    // 检查是否是"无技能"特殊标签
    const bool bTagNone = AbilityTag.MatchesTag(GamePlayTags.Abilities_None);
    
    // 尝试获取技能规格（AbilitySpec）
    FGameplayAbilitySpec* AbilitySpec = GetAuraASC()->GetSpecFromAbilityTag(AbilityTag);
    
    // 检查是否成功获取技能规格
    const bool bSpecValid = AbilitySpec != nullptr;

    // === 4. 确定技能状态 ===
    // 如果标签无效、是"无技能"标签或没有对应的技能规格
    if (!bTagValid || bTagNone || !bSpecValid)
    {
        // 设置为锁定状态
        AbilityStatus = GamePlayTags.Abilities_Status_Locked;
    }
    else
    {
        // 从技能规格中获取实际状态（如已解锁/已装备）
        AbilityStatus = GetAuraASC()->GetStatusFromSpec(*AbilitySpec);
    }
    
    // === 5. 更新选中技能信息 ===
    // 存储选中的技能标签和状态
    SelectedAbility.AbilityTag = AbilityTag;
    SelectedAbility.Status = AbilityStatus;
    
    // === 6. 计算按钮可用状态 ===
    bool bEnableSpendPoints = false; // 消耗技能点按钮状态
    bool bEnableEquip = false;       // 装备技能按钮状态
    
    // 根据技能状态和技能点数计算按钮状态
    ShouldEnableButtons(AbilityStatus, SpellPoints, bEnableSpendPoints, bEnableEquip);
    
    // === 7. 获取技能描述信息 ===
    FString Description;
    FString NextLevelDescription;
    
    // 从能力系统获取技能描述（当前等级和下一等级）
    GetAuraASC()->GetDescriptionsByAbilityTag(AbilityTag, Description, NextLevelDescription);
    
    // === 8. 广播UI更新事件 ===
    // 参数说明：
    //   bEnableSpendPoints: 是否启用"升级技能"按钮
    //   bEnableEquip: 是否启用"装备技能"按钮
    //   Description: 当前技能描述
    //   NextLevelDescription: 下一等级技能描述
    SpellGlobeSelectedDelegate.Broadcast(
        bEnableSpendPoints, 
        bEnableEquip, 
        Description, 
        NextLevelDescription
    );
}

/**
 * 处理“消耗技能点”按钮被点击的逻辑
 *
 * 功能说明：
 * 当玩家在法术菜单点击“消耗技能点”按钮时，调用此方法向服务器请求对应技能的升级操作。
 * 该方法负责发起升级请求，由服务器进行实际的数据校验与处理（如技能点扣除、技能解锁或升级等）。
 *
 * 详细说明：
 * - 首先检查能力系统组件（AuraASC）是否有效，确保角色能力数据可用。
 * - 若有效，则调用能力系统组件的ServerSpendSpellPoint方法，将当前选中技能的标签（SelectedAbility.AbilityTag）传递给服务器端处理。
 * - 该操作为客户端到服务器的RPC调用，实际的技能升级与数据同步均由服务器权威执行，防止作弊和保持多人游戏数据一致性。
 * - 客户端只负责发起请求，升级结果会通过服务器回传，驱动UI自动刷新。
 */
void USpellMenuWidgetController::SpendPointButtonPressed()
{
	// 检查能力系统组件是否有效
	if (GetAuraASC())
	{
		// 发起RPC请求，由服务器处理技能点消耗与技能升级
		GetAuraASC()->ServerSpendSpellPoint(SelectedAbility.AbilityTag);
	}
}


/**
 * 取消法术球选择状态（重置选择状态）
 * 
 * 功能说明：
 * 1. 重置当前选中的技能标签为"无"
 * 2. 设置技能状态为"锁定"
 * 3. 广播取消选择事件通知UI更新
 * 
 * 调用场景：
 * - 玩家关闭技能菜单时
 * - 切换技能选择时
 * - 取消技能装备操作时
 */
void USpellMenuWidgetController::GlobeDeselect()
{
	//当等待技能装备中时
	if (bWaitingForEquipSelection)
	{
		//获取当前取消选中的技能类型
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityType;
		//广播停止等待装备技能选择事件
		StopWaitForEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}

	
	// 重置选中的技能标签为"无"（特殊空标签）
	// 使用全局标签系统中的Abilities_None标签
	SelectedAbility.AbilityTag = FAuraGamePlayTags::Get().Abilities_None;
    
	// 设置技能状态为"锁定"
	// 表示当前没有选中的技能或技能不可用
	SelectedAbility.Status = FAuraGamePlayTags::Get().Abilities_Status_Locked;
    
	// 广播法术球取消选择事件
	// 参数说明：
	//   bool bSpellSelected: 是否有技能被选中 (false)
	//   bool bCanEquip: 当前选中技能是否可装备 (false)
	//   FString Name: 技能名称 (空字符串)
	//   FString Description: 技能描述 (空字符串)
	SpellGlobeSelectedDelegate.Broadcast(
		false,   // 无技能被选中
		false,   // 不可装备
		FString(), // 清空技能名称
		FString()  // 清空技能描述
	);
}

/**
 * 处理装备按钮按下事件
 * 
 * 功能说明：
 * 1. 获取当前选中技能的类型
 * 2. 广播等待装备选择事件
 * 3. 设置等待选择状态标志
 * 
 * 调用场景：
 * - 玩家在技能菜单中选择某个技能后按下装备按钮
 * - 触发装备槽位选择流程
 */
void USpellMenuWidgetController::EquipButtonPressed()
{
	// 1. 获取当前选中技能的类型（如主动技能、被动技能等）
	// 通过技能信息数据资产查询当前选中技能的类型标签
	const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(
		SelectedAbility.AbilityTag  // 当前选中的技能标签
	).AbilityType;
    
	// 2. 广播等待装备选择事件
	// 参数说明：传递技能类型，UI将根据类型显示对应的装备槽位
	WaitForEquipDelegate.Broadcast(AbilityType);
    
	// 3. 设置等待装备选择状态标志
	bWaitingForEquipSelection = true;
   
}


/**
 * 处理法术菜单中技能行被点击的逻辑
 *
 * @param SlotTag      点击的技能槽标签
 * @param AbilityType  点击的技能类型标签
 *
 * 功能说明：
 * 当玩家在法术菜单中点击某个技能行时，调用此方法处理技能选择逻辑。
 * 该方法首先检查是否在等待装备选择，然后确认选中的技能类型是否与点击的技能类型匹配。
 *
 * 详细说明：
 * - 首先检查bWaitingForEquipSelection标志，若为false则直接返回，表示当前不在等待装备选择阶段。
 * - 然后，通过AbilityInfo查找与SelectedAbility.AbilityTag对应的技能信息，获取该技能的类型（SelectedAbilityType）。
 * - 使用MatchesTagExact方法检查SelectedAbilityType是否与传入的AbilityType匹配。
 *   - 如果不匹配，则返回，表示该技能与点击的类型不符，无法进行后续的装备或操作。
 * - 如果匹配，则可以执行后续的装备逻辑（未在此段代码中展现）。
 */
void USpellMenuWidgetController::SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType)
{
	// 检查是否正在等待装备选择
	if (!bWaitingForEquipSelection) return;

	// 查找当前选中技能的信息，获取其类型
	const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityType;

	// 检查选中技能类型是否与点击的技能类型匹配
	if (!SelectedAbilityType.MatchesTagExact(AbilityType)) return;
	// 如果匹配，则在服务器执行装备能力逻辑
	GetAuraASC()->ServerEquipAbility(SelectedAbility.AbilityTag, SlotTag);
	
}

/**
 * 响应技能装备事件（法术菜单控制器）
 *
 * @param AbilityTag      被装备技能的标签
 * @param Status          技能当前状态（如已装备、已解锁等）
 * @param Slot            技能新槽位标签
 * @param PreviousSlot    技能原槽位标签
 *
 * 功能说明：
 * 1. 关闭等待装备选择状态（UI解锁交互）。
 * 2. 广播“上一个槽位已清空”消息，通知UI做对应视觉清理。
 * 3. 广播新装备技能的槽位信息，用于UI更新展示。
 * 4. 通知结束等待装备（如关闭高亮、停止动画等）。
 * 5. 通知技能球重新分配，通常用于UI元素重新布局。
 * 6. 取消技能球选中状态，恢复UI初始状态。
 *
 * 虚幻机制说明：
 * - 多播委托（Delegate）：用于事件派发和UI解耦，多个UI/逻辑层可同时监听并响应这些事件。
 * - GameplayTag系统：标签用于标识技能和状态，统一管理便于扩展。
 */
void USpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	// 1. 关闭等待装备选择状态（如按钮解锁、界面可操作）
	bWaitingForEquipSelection = false;

	// 2. 获取全局标签单例
	const FAuraGamePlayTags& GameplayTags = FAuraGamePlayTags::Get();

	// 3. 构造先前槽位的清空信息
	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag  = GameplayTags.Abilities_Status_Unlocked; // 状态重置为未装备
	LastSlotInfo.InputTag   = PreviousSlot;                           // 槽位设为上一个槽
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;            // 技能标签设为None

	// 4. 广播槽位清空，驱动UI刷新（如技能球消失）
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	// 5. 查找并更新当前装备技能的信息
	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status; // 更新技能状态
	Info.InputTag  = Slot;   // 更新槽位标签

	// 6. 广播新装备技能的信息，UI显示新技能
	AbilityInfoDelegate.Broadcast(Info);

	// 7. 通知UI或动画层“装备操作已完成”（如关闭加载动画）
	StopWaitForEquipDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);

	// 8. 通知UI重新分配技能球（通常用于视觉布局）
	SpellGlobeReassignedDelegate.Broadcast(AbilityTag);

	// 9. 取消技能球选中状态，恢复菜单初始交互
	GlobeDeselect();
}



/**
 * 判断技能球操作按钮的可用性（升级按钮/装备按钮）
 *
 *
 *
 * @param AbilityStatus 当前技能的状态标签
 * @param SpellPoints 当前玩家可用的技能点数
 * @param bShouldEnableSpellPointsButton [输出] 是否应启用“消耗技能点”按钮
 * @param bShouldEnableEquipButton [输出] 是否应启用“装备”按钮
 *
 * 功能说明：
 * 根据技能状态和技能点数，判断法术菜单中“消耗技能点”和“装备”按钮是否可用。
 * 常用于在技能球被选中后，动态刷新UI按钮的交互状态，防止无效操作。
 *
 * 详细说明：
 * - 首先默认两个按钮均不可用（false），然后根据技能当前状态标签具体判断。
 * - 如果技能已装备（Equipped），装备按钮可用；若技能点数大于0，则“消耗技能点”按钮也可用。
 * - 如果技能已解锁且可升级（Eligible），只要技能点数大于0，则“消耗技能点”按钮可用，装备按钮始终不可用。
 * - 如果技能已解锁但未装备（Unlocked），装备按钮可用，且只要有技能点同样“消耗技能点”按钮可用。
 * - 其它状态（如锁定、无效）下，两个按钮都不可用。
 * - 该函数通过引用参数返回按钮使能状态，通常会在UI逻辑中配合委托进行按钮状态刷新。
 */
void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints,bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton)
{
	
	// 获取全局游戏标签
	const FAuraGamePlayTags GamePlayTags = FAuraGamePlayTags::Get();

	// 默认两个按钮都不可用
	bShouldEnableEquipButton = false;
	bShouldEnableSpellPointsButton = false;

	// 技能已装备：可装备按钮可用，有技能点则升级按钮也可用
	if (AbilityStatus.MatchesTagExact(GamePlayTags.Abilities_Status_Equipped))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	// 技能可升级（已解锁且未装备）：只要有技能点则升级按钮可用
	else if (AbilityStatus.MatchesTagExact(GamePlayTags.Abilities_Status_Eligible))
	{
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	// 技能已解锁但未装备：装备按钮可用，有技能点则升级按钮也可用
	else if (AbilityStatus.MatchesTagExact(GamePlayTags.Abilities_Status_Unlocked))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	// 其它状态下，两个按钮都不可用（已在初始值中处理）
}
