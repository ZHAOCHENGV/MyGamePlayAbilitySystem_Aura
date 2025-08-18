// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AuraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGamePlayTags.h"
#include "Aura/AuraLogChannels.h"
#include "GAS/AuraAbilitySystemLibrary.h"
#include "GAS/Ability/AuraGameplayAbility.h"
#include "GAS/Data/AbilityInfo.h"
#include "Interation/PlayerInterface.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	// 将 OnGameplayEffectAppliedDelegateToSelf 事件委托绑定到当前对象（this）和 EffectApplied() 函数。
	// 这意味着当 Gameplay Effect 被应用于该组件时，会调用 EffectApplied() 函数。
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this,&UAuraAbilitySystemComponent::ClientEffectApplied);

	const FAuraGamePlayTags & GamePlayTags = FAuraGamePlayTags::Get();
	//GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Red, FString::Printf(TEXT("标签：%s"),*GamePlayTags.Attributes_Secondary_Armor.ToString()));
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartUpAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartUpAbilities)
	{
		// 创建一个游戏能力的规格实例，指定能力类型和等级
		// 参数 1 表示技能的等级，默认赋值为 1
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass,1);
		// 使用 Cast 尝试将 AbilitySpec.Ability 转换为 UAuraGameplayAbility 类型
		// 这是为了检查技能是否属于自定义的 UAuraGameplayAbility 类
		if (const UAuraGameplayAbility * AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			// 如果技能是 UAuraGameplayAbility 类型，获取其启动时绑定的输入标签
			// 并将这个标签添加到技能的动态能力标签（DynamicAbilityTags）中
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag);
			//添加技能状态为已装备
			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGamePlayTags::Get().Abilities_Status_Equipped);
			// 将创建的技能规范授予玩家
			GiveAbility(AbilitySpec);
		}

	}
	// 设置能力初始化完成标志
	bStartupAbilitiesGiven = true;
	// 广播（触发）多播委托，将当前对象作为参数传递
	// 这会通知所有绑定了该委托的回调函数
	AbilitiesGivenDelegate.Broadcast();
}

/**
 * 为角色添加并激活被动技能
 * @param StartUpPassiveAbilities 被动技能类数组（需继承自UGameplayAbility）
 * 
 * 功能说明：
 * 1. 遍历传入的被动技能类数组
 * 2. 为每个技能创建规格说明
 * 3. 赋予角色能力并立即激活
 * 
 * 注意：该函数通常在角色初始化阶段调用
 */
void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartUpPassiveAbilities)
{
	// 遍历所有被动技能类
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartUpPassiveAbilities)
	{
		// 创建技能规格说明
		// 参数说明：
		// - AbilityClass: 技能类
		// - 1: 技能等级（可根据设计需求调整）
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		// 赋予能力并激活（单次激活模式）
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

/**
 * @brief 处理“输入标签按下”（Pressed）：为匹配该输入的 GA 标记 InputPressed，必要时同步网络事件
 * @param InputTag 按下的输入标签（如 InputTag.LMB / InputTag.Skill.Q 等）
 * @details
 *  - 流程：校验标签 → 遍历可激活 GA → 若 GA 的动态标签精确包含该 InputTag，则调用 AbilitySpecInputPressed；
 *    若 GA 目前未激活，则广播一个通用复制事件（当前代码为 InputReleased，见注意事项）。
 *  - 注意：此实现使用了已弃用字段 DynamicAbilityTags，应切换到 GetDynamicSpecSourceTags()。
 */
void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	// 检查输入标签是否有效，如果无效直接返回
	if(!InputTag.IsValid())return;
	FScopedAbilityListLock ActiveScopeLoc(*this);
	// 遍历所有可激活的技能
	// GetActivatableAbilities 获取当前组件中的所有可激活技能。
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 检查当前技能的动态标签中是否有精确匹配的输入标签
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			// 标记该技能的输入为已按下（影响 GA 的 InputPressed 回调/内部状态）
			AbilitySpecInputPressed(AbilitySpec);

			// 如果技能当前处于激活状态
			if (AbilitySpec.IsActive())
			{
				TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
				const FGameplayAbilityActivationInfo& ActivationInfo = Instances.Last()->GetCurrentActivationInfoRef();
				FPredictionKey OriginalPredictionKey = ActivationInfo.GetActivationPredictionKey();
				// 通用复制事件
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, OriginalPredictionKey);
			
			}
		}
	}
}

/**
 * @brief 处理“输入标签按住”（Held）：为匹配该输入的 GA 标记 InputPressed；未激活则尝试激活
 * @param InputTag 按住的输入标签
 * @details
 *  - 流程：校验标签 → 遍历可激活 GA → 若动态标签包含该 InputTag：调用 AbilitySpecInputPressed；
 *    若 GA 未激活则 TryActivateAbility。
 *  - 注意：此实现使用了已弃用字段 DynamicAbilityTags，应切换到 GetDynamicSpecSourceTags()。
 */
void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	// 检查输入标签是否有效，如果无效直接返回
	if(!InputTag.IsValid())return;

	// 遍历期间更安全
	FScopedAbilityListLock AbilityScopeLock(*this);
	// 遍历所有可激活的技能
	// GetActivatableAbilities 获取当前组件中的所有可激活技能。
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 检查当前技能的动态标签中是否有精确匹配的输入标签
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// 标记该技能的输入为已按下（保持“持续输入”）
			AbilitySpecInputPressed(AbilitySpec);

			// 如果技能当前未处于激活状态
			if (!AbilitySpec.IsActive())
			{
				// 尝试激活该技能（支持按住触发/引导型 GA）
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}
/**
 * @brief 处理“输入标签松开”事件：把松开状态同步到匹配的 GA，并广播网络复制事件
 * @param InputTag 松开的输入标签（如 InputTag.LMB / InputTag.Skill.Q 等）
 */
void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if(!InputTag.IsValid())return;                            // 1) 无效标签直接返回
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities()) // 2) 遍历所有可激活能力
	{
		// 3) 动态标签中是否“精确包含”该输入标签 且 该 GA 当前已激活？
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			// 4) 标记该技能“输入已释放”（影响 GA 的 InputReleased 回调等）
			AbilitySpecInputReleased(AbilitySpec);             // 本地标记输入释放
			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
			const FGameplayAbilityActivationInfo& ActivationInfo = Instances.Last()->GetCurrentActivationInfoRef();
			FPredictionKey OriginalPredictionKey = ActivationInfo.GetActivationPredictionKey();
			// 5) 广播“输入释放”的通用复制事件（让服务器/其他端同步这个输入变化）
			// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, OriginalPredictionKey);
		
		}
	}
}




/**
 * @brief 遍历当前所有可激活能力，对每个 AbilitySpec 调用外部传入的委托
 * @param Delegate 回调委托：签名接收 const FGameplayAbilitySpec&，返回是否执行成功
 * @details 通过 FScopedAbilityListLock 在迭代期间加锁，避免能力列表被改动导致崩溃
 */
void UAuraAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	// 使用作用域锁保证遍历期间能力列表的稳定性（GAS 内部要求）
	FScopedAbilityListLock ActiveScopeLock(*this);

	// 遍历所有可激活能力
	for (const FGameplayAbilitySpec & AbilitySpec : GetActivatableAbilities())
	{
		// 安全执行委托；失败时打印错误日志（便于发现未绑定或执行失败）
		if(!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogAura,Error,TEXT("在 %hs 未能执行委托"),__FUNCTION__); // 输出错误日志
		}
	}
}

/**
 * @brief 从 AbilitySpec.Ability 自带的 AbilityTags 中，找出归类到 "Abilities" 下的标签
 * @param AbilitySpec 能力规格（包含 Ability 指针与静态 AbilityTags）
 * @return 若找到子类于 "Abilities" 的标签则返回之；否则返回空 Tag
 */
FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability) // 有效性检查（Ability 指针可能为空）
	{
		// 遍历该 GA 静态配置的 AbilityTags
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			// 检查是否在 "Abilities" 分类下（允许层级匹配）
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
			{
				return Tag; // 返回匹配到的能力标签
			}
		}
	}
	return FGameplayTag(); // 默认返回空标签
}

/**
 * @brief 从 AbilitySpec 的“动态标签”里找一个属于 "InputTag" 分类的标签（代表这个 GA 当前绑定的输入）
 * @param AbilitySpec 能力规格（动态标签通常在运行时绑定/解绑）
 * @return 若找到 "InputTag" 下的标签则返回之；否则返回空 Tag
 * @note 这里使用了已弃用的 DynamicAbilityTags；需改为 GetDynamicSpecSourceTags()（见后文“修复警告”）
 */
FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	// 遍历动态能力标签（Deprecated：DynamicAbilityTags）
	for (FGameplayTag Tag : AbilitySpec.DynamicAbilityTags)
	{
		// 检查是否属于 "InputTag" 分类（父子层级匹配）
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag; // 返回匹配到的输入标签
		}
	}
	return FGameplayTag(); // 默认返回空标签
}


/**
 * 从技能规格（AbilitySpec）中提取状态标签
 * 
 * @param AbilitySpec 要检查的技能规格数据
 * @return 找到的状态标签（如Abilities.Status.Locked），未找到则返回空标签
 * 
 * 功能说明：
 * 1. 遍历技能的所有动态标签
 * 2. 筛选属于"Abilities.Status"类别的标签
 * 3. 返回第一个匹配的状态标签
 * 
 * 典型应用场景：
 * - 判断技能是否解锁（Abilities.Status.Unlocked）
 * - 检查技能冷却状态（Abilities.Status.Cooldown）
 */
FGameplayTag UAuraAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	// 遍历技能的所有动态标签
	for (FGameplayTag StatusTag : AbilitySpec.DynamicAbilityTags)
	{
		// 检查标签是否属于"Abilities.Status"分类
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			// 返回找到的状态标签
			return StatusTag;
		}
	}
	// 未找到匹配标签时返回空标签
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusFromSpec(*Spec);
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*Spec);
	}
	return FGameplayTag();
}


/**
 * [客户端] 发起属性升级请求
 * @param AttributeTag - 要升级的属性标签（如：Attributes.Strength）
 * 
 * 流程说明：
 * 1. 验证玩家接口实现
 * 2. 检查可用属性点
 * 3. 通过RPC调用服务器端逻辑
 */
void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	//判断Actor是否继续了接口
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		//获取当前属性点数是否大于0
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor())>0)
		{
			//如果大于0则 触发服务器端逻辑更新属性
			ServerUpgradeAttribute(AttributeTag);
		}
	}

	
}

/**
 * [服务器] 执行属性升级逻辑
 * @param AttributeTag - 属性标签（需通过网络验证）
 */
void UAuraAbilitySystemComponent::ServerUpgradeAttribute(const FGameplayTag& AttributeTag)
{
	// 构造游戏事件数据
	FGameplayEventData Payload;
	// 事件类型标签
	Payload.EventTag = AttributeTag;
	// 升级幅度（+1级）
	Payload.EventMagnitude = 1.f;

	// 发送事件触发对应的GameplayAbility
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(),AttributeTag,Payload);
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		//升级完， 扣除属性点（仅在服务端执行）
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

/**
 * 根据技能标签查找对应的技能规格（AbilitySpec）
 * 
 * @param AbilityTag 要查找的技能标签
 * @return 找到的技能规格指针，未找到返回nullptr
 * 
 * 功能说明：
 * 1. 使用作用域锁确保遍历过程中技能列表不会被修改
 * 2. 遍历所有可激活技能
 * 3. 检查每个技能的标签是否匹配目标标签
 */
FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	// 创建作用域锁：防止遍历过程中技能列表被修改
	// 重要：确保线程安全和数据一致性
	FScopedAbilityListLock ActiveScopeLock(*this);
	// 遍历所有可激活技能
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 遍历当前技能的所有标签
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			// 检查标签是否匹配（支持父子标签匹配）
			if (Tag.MatchesTag(AbilityTag))
			{
				// 返回找到的技能规格
				return &AbilitySpec;
			}
		}
	}
	// 未找到匹配技能
	return nullptr;
}

/**
 * 根据玩家等级更新技能状态
 * 
 * @param Level 当前玩家等级
 * 
 * 功能流程：
 * 1. 获取技能配置数据
 * 2. 遍历所有技能配置
 * 3. 检查技能是否满足解锁条件
 * 4. 为符合条件的技能创建新规格并添加状态标签
 */
void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	// 获取技能配置数据资产
	// 注意：依赖GetAvatarActor()获取关联角色
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	// 遍历所有技能配置
	for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		// 跳过无效标签
		if(!Info.AbilityTag.IsValid())continue;
		// 检查等级要求：未达到要求则跳过
		if(Level < Info.LevelRequirement)continue;
		// 检查是否已存在该技能
		if(GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			// 创建新技能规格
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			// 添加"符合条件"状态标签
			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGamePlayTags::Get().Abilities_Status_Eligible);
			// 赋予角色新技能
			GiveAbility(AbilitySpec);
			// 强制能力规格立即复制
			MarkAbilitySpecDirty(AbilitySpec);
			//广播委托
			ClientUpdateAbilityStatus(Info.AbilityTag,FAuraGamePlayTags::Get().Abilities_Status_Eligible,1);
		}
		
	}
}


/**
 * 服务器端处理消耗技能点以升级或解锁技能的逻辑
 *
 * @param AbilityTag 要升级或解锁的技能标签
 *
 * 功能说明：
 * 当客户端请求消耗技能点升级或解锁某个技能时，服务器通过该方法完成实际的数据修改与同步。
 * 包括技能点扣除、技能状态变更、技能等级提升，并将最新状态同步回客户端，实现数据的权威管理。
 *
 * 详细说明：
 * - 首先通过技能标签查询能力系统组件中对应的技能规格（AbilitySpec）。
 * - 若找到了有效的AbilitySpec，并且角色实现了UPlayerInterface接口，则调用AddToSpellPoints方法扣除1点技能点。
 * - 获取全局技能标签集合（GamePlayTags），并根据AbilitySpec获取当前技能状态（Status）。
 * - 如果技能处于“可升级”（Eligible）状态，表示玩家首次升级此技能：移除Eligible标签，添加Unlocked标签，并更新Status为Unlocked。
 * - 如果技能处于“已装备”（Equipped）或“已解锁”（Unlocked）状态，说明技能已经可以直接升级：将技能等级加1。
 * - 无论哪种情况，均通过ClientUpdateAbilityStatus方法，将技能标签、最新状态和等级同步给客户端，确保前端UI及时更新。
 * - 最后调用MarkAbilitySpecDirty标记该技能规格为已修改，确保网络同步和存档生效。
 * - 此函数为服务器RPC的实现体（_Implementation后缀），只会在服务器端运行，防止作弊并确保数据一致性。
 */
void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
    // 查找目标技能规格（AbilitySpec）
    if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
    {
        // 如果角色实现了玩家接口，则扣除1点技能点
        if (GetAvatarActor()->Implements<UPlayerInterface>())
        {
            IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
        }

        // 获取全局技能标签
        const FAuraGamePlayTags GamePlayTags = FAuraGamePlayTags::Get();
        // 获取当前技能的状态标签
        FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);

        // 若技能处于“可升级”状态，首次解锁技能
        if (Status.MatchesTagExact(GamePlayTags.Abilities_Status_Eligible))
        {
            AbilitySpec->DynamicAbilityTags.RemoveTag(GamePlayTags.Abilities_Status_Eligible);
            AbilitySpec->DynamicAbilityTags.AddTag(GamePlayTags.Abilities_Status_Unlocked);
            Status = GamePlayTags.Abilities_Status_Unlocked; // 更新状态为已解锁
        }
        // 若技能已装备或已解锁，直接升级技能等级
        else if (Status.MatchesTagExact(GamePlayTags.Abilities_Status_Equipped) ||
                 Status.MatchesTagExact(GamePlayTags.Abilities_Status_Unlocked))
        {
            AbilitySpec->Level += 1;
        }

        // 通知客户端技能状态和等级的最新信息，保证UI和数据同步
        ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);

        // 标记技能规格为已修改，触发网络同步
        MarkAbilitySpecDirty(*AbilitySpec);
    }
}


/**
 * 服务器端执行技能装备逻辑（RPC实现）
 * 
 * @param AbilityTag 要装备的技能标签
 * @param Slot 目标装备槽位标签
 * 
 * 功能说明：
 * 1. 验证技能是否可装备（已解锁或已装备状态）
 * 2. 清除目标槽位上的现有技能
 * 3. 更新技能标签和状态
 * 4. 同步状态到客户端
 * 
 * 网络说明：
 * - 使用_Implementation后缀表示这是服务器RPC的实际实现
 * - 仅在服务端执行，客户端调用需通过Server前缀函数
 */
void UAuraAbilitySystemComponent::ServerEquipAbility_Implementation(
    const FGameplayTag& AbilityTag,
    const FGameplayTag& Slot)
{
    // 1. 获取目标技能的规格信息
    if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
    {
        // 获取游戏标签单例（包含状态标签如Equipped/Unlocked）
        const FAuraGamePlayTags GamePlayTags = FAuraGamePlayTags::Get();
        
        // 获取技能当前槽位和状态
        const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec);
        const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);
        
        // 2. 验证技能状态：必须是已解锁或已装备状态
        const bool bStatusValid = Status == GamePlayTags.Abilities_Status_Equipped || 
                                 Status == GamePlayTags.Abilities_Status_Unlocked;
        
        if (bStatusValid)
        {
            // 3. 清除目标槽位上的所有技能（确保槽位唯一性）
            ClearAbilitiesOfSlot(Slot);
            
            // 4. 清除当前技能原有的槽位标签
            ClearSlot(AbilitySpec);
            
            // 5. 添加新槽位标签到技能
            AbilitySpec->DynamicAbilityTags.AddTag(Slot);
            
            // 6. 状态升级：从未装备到已装备
            if (Status.MatchesTagExact(GamePlayTags.Abilities_Status_Unlocked))
            {
                // 移除"未装备"状态标签
                AbilitySpec->DynamicAbilityTags.RemoveTag(GamePlayTags.Abilities_Status_Unlocked);
                
                // 添加"已装备"状态标签
                AbilitySpec->DynamicAbilityTags.AddTag(GamePlayTags.Abilities_Status_Equipped);
            }
            
            // 7. 标记技能规格为脏数据（触发网络同步）
            MarkAbilitySpecDirty(*AbilitySpec);
        }
        
        // 8. 通知客户端装备结果
        ClientEquipAbility(AbilityTag, GamePlayTags.Abilities_Status_Equipped, Slot, PrevSlot);
    }
    // 建议：添加技能不存在的错误处理
}

/**
 * 客户端装备技能的逻辑实现
 * 
 * @param AbilityTag 要装备的技能标签
 * @param Status 装备状态（如已装备或已解锁）
 * @param Slot 当前技能槽位标签
 * @param PreviousSlot 前一个技能槽位标签
 * 
 * 功能说明：
 * 1. 广播技能装备事件，通知所有监听者技能的装备状态变化。
 * 2. 该函数主要用于客户端，确保客户端能够接收到技能装备的更新信息。
 */
void UAuraAbilitySystemComponent::ClientEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	// 1. 广播装备技能事件，允许其他系统或对象响应这一变化
	AbilityEquipped.Broadcast(AbilityTag, Status, Slot, PreviousSlot);
}

/**
 * 根据技能标签获取技能描述信息
 * 
 * @param AbilityTag 要查询的技能标签
 * @param OutDescription [输出] 当前等级的描述文本（已解锁）或锁定描述（未解锁）
 * @param OutNextDescription [输出] 下一等级的描述文本（仅已解锁技能有效）
 * @return 是否成功获取到已解锁技能描述
 * 
 * 功能流程：
 * 1. 尝试获取已解锁的技能规格
 * 2. 如果已解锁：获取当前等级和下一等级描述
 * 3. 如果未解锁：生成锁定状态描述
 * 4. 处理特殊空标签情况
 */
bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(
	const FGameplayTag& AbilityTag, 
	FString& OutDescription, 
	FString& OutNextDescription)
{
	// 尝试查找玩家已拥有的该技能规格
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		// 转换为自定义的Aura技能类
		if (UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability))
		{
			// 获取当前等级的技能描述
			OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);
            
			// 获取下一等级（当前等级+1）的技能描述
			OutNextDescription = AuraAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
            
			return true; // 成功获取已解锁技能描述
		}
	}
    
	// --- 处理未解锁技能的情况 ---
    
	// 获取技能配置数据资产
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
    
	// 处理无效或空标签
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FAuraGamePlayTags::Get().Abilities_None))
	{
		OutDescription = FString(); // 清空描述
	}
	else
	{
		// 获取解锁所需等级
		const int32 LevelRequirement = AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement;
        
		// 生成锁定状态描述（如"需要等级5解锁"）
		OutDescription = UAuraGameplayAbility::GetLockedDescription(LevelRequirement);
	}
    
	// 未解锁技能没有下一级描述
	OutNextDescription = FString();
    
	return false; // 未获取到已解锁技能描述
}
/**
 * 清除技能槽位标签
 * 
 * @param Spec 指向技能规格的指针
 * 
 * 功能说明：
 * 1. 从技能规格中获取当前技能的槽位标签。
 * 2. 从动态技能标签中移除该槽位标签。
 * 3. 标记技能规格为脏数据，以便触发网络同步。
 */
void UAuraAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	// 1. 获取技能规格对应的槽位标签
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
    
	// 2. 移除槽位标签
	Spec->DynamicAbilityTags.RemoveTag(Slot);
    
	// 3. 标记技能规格为脏数据
	MarkAbilitySpecDirty(*Spec);
}

/**
 * 清除指定槽位上的所有技能
 * 
 * @param Slot 需要清除的技能槽位标签
 * 
 * 功能说明：
 * 1. 锁定当前活动技能列表，确保线程安全。
 * 2. 遍历所有可激活的技能规格。
 * 3. 检查每个技能规格是否包含指定槽位标签。
 * 4. 如果包含，调用ClearSlot函数清除该技能的槽位标签。
 */
void UAuraAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& Slot)
{
	// 1. 锁定当前活动技能列表以避免线程冲突
	FScopedAbilityListLock ActiveScopeLock(*this);
    
	// 2. 遍历所有可激活的技能规格
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		// 3. 检查技能规格是否有指定的槽位标签
		if (AbilityHasSlot(&Spec, Slot))
		{
			// 4. 清除该技能的槽位标签
			ClearSlot(&Spec);
		}
	}
}

/**
 * 检查技能规格是否包含指定槽位标签
 * 
 * @param Spec 指向技能规格的指针
 * @param Slot 需要检查的槽位标签
 * @return 如果技能规格包含该槽位标签，则返回true；否则返回false。
 * 
 * 功能说明：
 * 1. 遍历技能规格的动态标签。
 * 2. 如果找到与指定槽位标签完全匹配的标签，返回true。
 * 3. 如果没有匹配的标签，则返回false。
 */
bool UAuraAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot)
{
	// 1. 遍历动态技能标签
	for (FGameplayTag Tag : Spec->DynamicAbilityTags)
	{
		// 2. 检查标签是否与指定槽位标签完全匹配
		if (Tag.MatchesTagExact(Slot))
		{
			return true; // 找到匹配的槽位标签
		}
	}
	return false; // 未找到匹配的槽位标签
}


/**
 * [网络复制] 可激活能力列表更新回调
 * 
 * 功能说明：
 * 当服务端同步初始能力时，标记能力初始化完成
 */
void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();
	// 首次同步时触发委托
	if (!bStartupAbilitiesGiven)
	{
		// 设置完成标志
		bStartupAbilitiesGiven = true;
		// 通知所有订阅者
		AbilitiesGivenDelegate.Broadcast();
	}
}

void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag,const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	//广播委托
	AbilityStatusChanged.Broadcast(AbilityTag,StatusTag,AbilityLevel);
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	// 这里将处理应用于当前组件的游戏效果时的逻辑。
	// 可以通过 EffectSpec 获取效果的详细信息，
	// 通过 ActiveEffectHandle 管理当前活动效果。

	//创建 Gameplay 标签容器
	FGameplayTagContainer TagContainer;
	//获取效果规范的所有标签
	EffectSpec.GetAllAssetTags(TagContainer);

	//广播多播委托
	EffectAbilityTags.Broadcast(TagContainer);
	

}


