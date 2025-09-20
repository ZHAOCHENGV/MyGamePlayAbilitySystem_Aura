// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AuraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGamePlayTags.h"
#include "Aura/AuraLogChannels.h"
#include "Game/LoadScreenSaveGame.h"
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


/**
 * @brief 从一个存档对象 (SaveData) 中读取技能数据，并在当前 ASC 上重新授予这些技能。
 * @param SaveData 包含已保存技能信息的 ULoadScreenSaveGame 对象。
 *
 * @par 功能说明
 * 该函数负责在游戏加载时恢复玩家角色的所有技能。它会遍历存档数据中的每一个技能记录，
 * 根据记录的信息（技能类型、等级、槽位、状态等）重新构建一个 FGameplayAbilitySpec (技能规格)，
 * 并通过 `GiveAbility` 系列函数将其授予给当前的 ASC (Ability System Component)。
 * 这个函数特别处理了主动技能和被动技能的不同授予方式。
 *
 * @par 详细流程
 * 1.  **遍历存档技能**: 循环遍历 `SaveData` 中存储的 `SavedAbilities` 数组。
 * 2.  **构建 AbilitySpec**:
 *     - 从存档数据中获取技能的类 (`UGameplayAbility`) 和等级。
 *     - 使用这些信息创建一个 `FGameplayAbilitySpec`。Spec 是一个技能在 ASC 内部的“实例”，包含了该技能的所有运行时数据。
 * 3.  **恢复动态标签**: 将存档中记录的技能槽位（Slot）和状态（Status）作为动态标签（Dynamic Tags）添加回 Spec 中。
 * 4.  **区分技能类型**:
 *     - **主动技能 (Offensive)**: 直接调用 `GiveAbility()` 授予技能。
 *     - **被动技能 (Passive)**:
 *       - 如果存档时该技能是“已装备”状态，则调用 `GiveAbilityAndActivateOnce()`。这个函数会授予技能并立即尝试激活它一次，这对于被动技能的初始化至关重要。同时，通过一个多播 RPC (`MulticastActivatePassiveEffect`) 通知所有客户端激活该技能关联的视觉特效。
 *       - 如果是其他状态（如“已解锁但未装备”），则只调用 `GiveAbility()`，不立即激活。
 * 5.  **标记完成**: 所有技能都授予完毕后，将 `bStartupAbilitiesGiven` 标志设为 `true`。这是一个非常重要的状态锁，用于通知系统其他部分（如特效组件）ASC 已经准备就绪。
 * 6.  **广播委托**: 调用 `AbilitiesGivenDelegate.Broadcast()`，这是一个自定义委托，用于通知任何监听者（例如 UI）初始技能已经授予完毕，可以进行刷新了。
 */

void UAuraAbilitySystemComponent::AddCharacterAbilitiesFromSaveData(ULoadScreenSaveGame* SaveData)
{
	// 遍历从存档文件中加载出来的每一个技能数据。
	for (const FSavedAbility& Data : SaveData->SavedAbilities)
	{
		// 步骤 1/4: 重建技能规格 (FGameplayAbilitySpec)
		// 从数据中获取技能的 UClass。
		const TSubclassOf<UGameplayAbility> LoadedAbilityClass = Data.GamePlayAbility;
		// FGameplayAbilitySpec 是技能在 ASC 中的运行时实例。它包含了技能的等级、输入绑定、动态标签等信息。
		// 这里我们用技能类和保存的等级来创建一个新的 Spec。
		FGameplayAbilitySpec LoadedAbilitySpec = FGameplayAbilitySpec(LoadedAbilityClass, Data.AbilityLevel);

		// 步骤 2/4: 恢复动态标签
		// (旧 → 新 API): 在旧版本 UE 中这里可能是 LoadedAbilitySpec.DynamicAbilityTags。
		// 在 UE5.1+ 中，官方推荐使用 GetDynamicSpecSourceTags() 来访问和修改这些标签。不过 DynamicAbilityTags 依然可用。
		// 动态标签是附加到这个特定 Spec 上的 Tag，用于描述它的状态，比如它被装备在哪个槽位。
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilitySlot);
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilityStatus);

		// 步骤 3/4: 根据技能类型分别处理
		// 检查技能的类型Tag是否为“进攻型”（即主动技能）。
		if (Data.AbilityType == FAuraGamePlayTags::Get().Abilities_Type_Offensive)
		{
			// GiveAbility 是授予技能的标准函数。它将 Spec 添加到 ASC 中，但通常不会自动激活它。
			GiveAbility(LoadedAbilitySpec);
		}
		// 检查技能类型是否为“被动型”。
		else if (Data.AbilityType == FAuraGamePlayTags::Get().Abilities_Type_Passive)
		{
			// 对于被动技能，需要进一步检查它在保存时是否是“已装备”状态。
			// (为什么用 MatchesTagExact): HasTagExact 通常用于检查一个容器是否精确包含某个 Tag。
			// MatchesTagExact 用于比较两个 Tag 是否完全相等。这里两者效果类似，但 MatchesTagExact 意图更明确。
			if (Data.AbilityStatus.MatchesTagExact(FAuraGamePlayTags::Get().Abilities_Status_Equipped))
			{
				// GiveAbilityAndActivateOnce 是一个特殊的授予函数，它在授予技能后会立即尝试激活一次。
				// 这对于那些需要“启动”的被动技能（比如一个永久光环）是必需的。
				GiveAbilityAndActivateOnce(LoadedAbilitySpec);
				// (NetMulticast): 这是一个多播 RPC (Remote Procedure Call)。
				// 服务器调用此函数后，它会在服务器自身和所有已连接的客户端上执行。
				// 用于确保所有玩家都能看到这个被动技能激活时的视觉特效（比如光环出现）。
				MulticastActivatePassiveEffect(Data.AbilityTag, true);
			}
			else
			{
				// 如果被动技能未装备，就只授予它，不激活。
				GiveAbility(LoadedAbilitySpec);
			}
			
		}
	}
	// 步骤 4/4: 发送完成信号
	// 这是一个非常重要的标志，用于防止其他系统在 ASC 完全就绪前就尝试查询技能状态。
	bStartupAbilitiesGiven = true;
	// 广播一个委托，通知 UI 或其他系统：“所有技能都已加载完毕，你们可以刷新了！”
	AbilitiesGivenDelegate.Broadcast();
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
		//设置已激活技能为已装备
		AbilitySpec.DynamicAbilityTags.AddTag(FAuraGamePlayTags::Get().Abilities_Status_Equipped);
		// 赋予能力并激活（单次激活模式）
		GiveAbilityAndActivateOnce(AbilitySpec);
	
	}
}

/**
 * @brief 处理“输入标签按下”：匹配到的 GA 标记为按下；未激活则尝试激活，已激活则广播 InputPressed（带正确 PredictionKey）
 * @param InputTag 按下的输入标签（如 InputTag.LMB / InputTag.Skill.Q 等）
 * @details
 *  - 使用新 API：AbilitySpec.GetDynamicSpecSourceTags() 进行标签匹配（代替已弃用 DynamicAbilityTags）
 *  - 遍历时使用 FScopedAbilityListLock，保证列表在迭代中的稳定性
 *  - 通过 UAuraAbilitySystemLibrary::AuraGetPredictionKeyFromSpec_Safe 拿到实例上的 PredictionKey
 */
void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	// 1) 标签无效则不处理
	if(!InputTag.IsValid())return;
	// 2) 遍历期间加锁，防止列表在激活/授予时被修改
	FScopedAbilityListLock ActiveScopeLoc(*this);// 作用域锁
	// 3) 遍历当前所有可激活的 GA（AbilitySpec）
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())// 遍历所有可激活能力
	{
		// 4) 用“动态源标签”精确匹配输入标签（HasTagExact：要求完全一致）
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))// 匹配到绑定该输入的 GA
		{
			// 5) 本地标记“输入按下”（影响 GA 内部的 InputPressed 状态/回调）
			AbilitySpecInputPressed(AbilitySpec);

			// 6) 若 GA 已激活 → 广播“输入按下”复制事件（供任务/服务器路由）
			if (AbilitySpec.IsActive())
			{
				// 6.1 从“实例”的 CurrentActivationInfo 中拿 PredictionKey（内部已做安全回退）
				const FPredictionKey OriginalPredictionKey = UAuraAbilitySystemLibrary::AuraGetPredictionKeyFromSpec_Safe(AbilitySpec);
				// 6.2 广播 InputPressed（客户端→服务器/其他端），保证 AbilityTask_WaitInputPress 能收到
				InvokeReplicatedEvent(
					EAbilityGenericReplicatedEvent::InputPressed,  // 事件类型：按下
					AbilitySpec.Handle,                            // 该 GA 的句柄
					OriginalPredictionKey                          // 正确的预测键
					);
			
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
 * @brief 处理“输入标签松开”：匹配到且已激活的 GA 标记为松开，并广播 InputReleased（带正确 PredictionKey）
 * @param InputTag 松开的输入标签
 * @details
 *  - 保证与 Pressed 对称：先本地标记 Released，再广播 InputReleased
 *  - 使用新 API：AbilitySpec.GetDynamicSpecSourceTags()；并通过安全函数获取 PredictionKey
 *  - WaitInputRelease 的 OnReleased 依赖正确的 PredictionKey 路由，否则不会触发
 */
void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	// 1) 标签无效则不处理
	if(!InputTag.IsValid())return;                            // 1) 无效标签直接返回
	// 2) 遍历期间加锁，防止容器被改动
	FScopedAbilityListLock ActiveScopeLoc(*this); // 作用域锁
	// 3) 遍历当前所有可激活的 GA
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities()) // 2) 遍历所有可激活能力
	{
		// 4) 匹配到该输入标签，且 GA 当前处于激活中（只对激活中的 GA 广播松开）
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			// 5) 本地标记“输入已释放”（影响 GA 内部 InputReleased 状态/回调）
			AbilitySpecInputReleased(AbilitySpec);// 标记松开
			// 6) 从“实例”的 CurrentActivationInfo 获取预测键（若无实例则内部回退旧字段）
			const  FPredictionKey OriginalPredictionKey = UAuraAbilitySystemLibrary::AuraGetPredictionKeyFromSpec_Safe(AbilitySpec);
			// 7) 广播“输入释放”复制事件（ASC→GA→AbilityTask）；WaitInputRelease 的 OnReleased 才会触发
			InvokeReplicatedEvent(
				EAbilityGenericReplicatedEvent::InputReleased, // 事件类型：松开
				AbilitySpec.Handle,                            // 该 GA 的句柄
				OriginalPredictionKey                         // 正确的预测键
				);
		
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

/**
 * @brief 通过“能力标签”查询该能力的“状态标签”（如：已装备/已解锁等）
 *
 * @param AbilityTag 能力的标识标签（例如 Abilities.Fireball）
 * @return FGameplayTag 若找到对应的 Spec，则返回其状态标签；否则返回“空标签”（无效）
 *
 * 功能说明：
 * - 先用 AbilityTag 定位到对应的 FGameplayAbilitySpec；
 * - 再从该 Spec 中提取“状态标签”（项目自定义语义，如 Equipped/Unlocked/CoolingDown 等）。
 *
 * 注意事项：
 * - 若 AbilityTag 无效或未匹配到任何 Spec，则返回空标签（默认构造的 FGameplayTag）；
 * - GetSpecFromAbilityTag 内部应做好加锁/遍历（建议使用 FScopedAbilityListLock）；
 * - 若项目中一个 AbilityTag 可能对应多条 Spec，需在 GetSpecFromAbilityTag 内定义清晰的选择规则（如取第一条）。
 */
FGameplayTag UAuraAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	// 步骤 1：尝试用 AbilityTag 找到对应的能力规格（Spec）
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag)) // 找到则进入
	{
		// 步骤 2：从 Spec 中解析并返回“状态标签”
		return GetStatusFromSpec(*Spec); // 例如返回 Abilities_Status_Equipped / Abilities_Status_Unlocked 等
	}
	// 步骤 3：未找到对应 Spec，返回“空标签”
	return FGameplayTag(); // 无效标签，调用端可用 IsValid() 判断
}

/**
 * @brief 通过“能力标签”查询该能力的“槽位/输入标签”（如：绑定到 LMB/Q/E 等）
 *
 * @param AbilityTag 能力的标识标签（例如 Abilities.Fireball）
 * @return FGameplayTag 若找到对应的 Spec，则返回其输入/槽位标签；否则返回“空标签”（无效）
 *
 * 功能说明：
 * - 先用 AbilityTag 定位对应的 FGameplayAbilitySpec；
 * - 再从该 Spec 的“动态标签集合”中取出“输入/槽位标签”（项目中常作为绑定键位/槽位的标识）。
 *
 * 注意事项：
 * - 旧代码可能从 Spec.DynamicAbilityTags 中取；UE 新版推荐使用 Spec.GetDynamicSpecSourceTags()；
 * - 若能力未绑定到任何槽位，或 AbilityTag 无效，则返回空标签；
 * - 槽位语义与“输入标签”在部分项目中等价（例如 Abilities.Slot.LMB / InputTag.LMB），注意统一。
 */
FGameplayTag UAuraAbilitySystemComponent::GetSlotFromAbilityTag(const FGameplayTag& AbilityTag)
{
	// 步骤 1：尝试用 AbilityTag 找到对应的能力规格（Spec）
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag)) // 找到则进入
	{
		// 步骤 2：从 Spec 中解析并返回“输入/槽位标签”
		return GetInputTagFromSpec(*Spec); // 例如返回 Abilities.Slot.Q / InputTag.LMB 等
	}
	// 步骤 3：未找到对应 Spec，返回“空标签”
	return FGameplayTag(); // 无效标签，调用端可用 IsValid() 判断
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
	 * @brief 多播 RPC：通知所有端（含服务器本地）某“被动效果”应当启用/停用
	 *
	 * @param AbilityTag 目标被动能力的标签（与监听方做精确匹配）
	 * @param bActivate  true=启用，false=停用
	 *
	 * 功能说明：
	 * - 服务器调用本函数后，UE 网络层会把该调用以 **NetMulticast** 的方式发送到所有已连接客户端，
	 *   并在各端执行 `_Implementation`，里面通过委托把事件广播给已绑定的系统（如 Niagara 组件或被动 GA）。
	 *
	 * 详细流程：
	 * 1) **服务器**调用 `MulticastActivatePassiveEffect(...)`；
	 * 2) 引擎网络层按 **Unreliable**（不保证送达/顺序）的语义把 RPC 发往各客户端；
	 * 3) 各端（含服务器本地）进入 `_Implementation`；
	 * 4) 通过 `ActivatePassiveEffect.Broadcast(AbilityTag, bActivate)` 通知所有监听者；
	 * 5) 监听者（如 `UPassiveNiagaraComponent` / `UAuraPassiveAbility`）据 `AbilityTag` 与 `bActivate` 做启停。
	 *
	 * 注意事项：
	 * - **仅服务器**应调用该 RPC（NetMulticast 的发送端必须是服务器），可在调用点加 `ensure(HasAuthority())`；
	 * - **Unreliable**：可能丢包/乱序/晚到，**不适合**承载“必须到达”的关键状态；建议事件应“幂等可重放”，或用额外的**可复制状态**兜底；
	 * - **晚加入的客户端**收不到历史多播，若需要初始同步，考虑使用 `OnRep` 的布尔状态（如 `bPassiveActive`）或 `Reliable`（但注意带宽/拥塞）；
	 * - 在监听端（组件/GA）要**先完成委托绑定**，再触发多播，否则本次事件可能被错过。
	 */
void UAuraAbilitySystemComponent::MulticastActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag,bool bActivate)
{
	ActivatePassiveEffect.Broadcast(AbilityTag, bActivate); // 广播项目内委托；监听者据 Tag/状态做启停
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
 * @brief 服务器端为“指定能力标签”绑定到“指定槽位”的实现（Server RPC）
 *
 * @param AbilityTag  目标能力的标识标签（如 Abilities.Fireball）
 * @param Slot        目标槽位/输入标签（如 InputTag.LMB / Abilities.Slot.Q）
 *
 * 功能说明：
 * - 在服务器上根据 AbilityTag 找到对应的 FGameplayAbilitySpec；
 * - 校验该能力当前状态（已解锁/已装备才允许装备到槽位）；
 * - 若目标槽位已被其他能力占用：处理“同能力重复装备”的快速返回；否则清理旧能力槽位，并在必要时“停用旧被动”；
 * - 若本次是该能力**首次**占用槽位且是“被动型能力”，则尝试激活并广播被动启用；
 * - 把能力分配到新槽位、标记 Spec 脏以触发复制，并通知客户端 UI 同步。
 *
 * 详细流程（高层）：
 * 1) GetSpecFromAbilityTag → Spec；取出旧槽位 PrevSlot 与当前状态 Status；
 * 2) 允许状态：Equipped/Unlocked 才继续；
 * 3) 若 Slot 已被占用：
 *    3.1) 如果就是同一个 Ability：直接 Client 通知并 return（无需改动）；
 *    3.2) 如果是别的 Ability：若它是 Passive → 先停用并广播，然后 ClearSlot(旧 Spec)；
 * 4) 若本能力尚未占用任何槽位 且 是 Passive → TryActivateAbility + Multicast 被动启用；
 * 5) AssignSlotToAbility（把本能力放到新槽位）+ MarkAbilitySpecDirty；
 * 6) 最后 ClientEquipAbility 通知客户端状态（UI/提示等）。
 *
 * 注意事项：
 * - 本函数是 “_Implementation” 版本，仅在服务器执行；调用方应通过 Server RPC 入口；
 * - ClearSlot / AssignSlotToAbility 修改了 Spec 的“动态源标签”，务必 MarkAbilitySpecDirty 触发同步；
 * - GetSpecWithSlot 函数名疑似拼写（应为 With）；HasAnySlot / IsPassiveAbility 等函数内部建议使用 GetDynamicSpecSourceTags() 新 API；
 * - 若 Slot 无效或 AbilityTag 未找到 Spec，函数将不做任何处理（可按项目需求补充日志与保护）。
 */
void UAuraAbilitySystemComponent::ServerEquipAbility_Implementation(
    const FGameplayTag& AbilityTag,
    const FGameplayTag& Slot)
{
    // 步骤 1：用能力标签查找对应的能力规格（Spec）
    if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
    {
        // 步骤 2：获取全局标签单例（集中管理项目用到的标签）
        const FAuraGamePlayTags& GamePlayTags = FAuraGamePlayTags::Get();
        // 步骤 3：读取该能力当前绑定的“旧槽位”标签（如果有）
        const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec);
        // 步骤 4：读取该能力当前的“状态”标签（如 Equipped/Unlocked）
        const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);
        
        // 步骤 5：允许装备的状态仅为“已装备”或“已解锁”
        const bool bStatusValid = Status == GamePlayTags.Abilities_Status_Equipped || Status == GamePlayTags.Abilities_Status_Unlocked;
        
        // 步骤 6：仅当状态合法时才进行装备逻辑
        if (bStatusValid)
        {
            // 步骤 7：如果目标槽位当前不是空的，需要处理“顶掉/替换”
            if (!SlotIsEmpty(Slot))
            {
                // 步骤 7.1：找到当前占用该槽位的能力 Spec
                FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(Slot);
                if (SpecWithSlot)
                {
                    // 步骤 7.2：如果占位的就是“同一个能力”，那就只是重复装备 → 直接同步到客户端并返回
                    if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
                    {
                        ClientEquipAbility(AbilityTag, GamePlayTags.Abilities_Status_Equipped, Slot, PrevSlot); // 通知客户端：同能力维持已装备状态
                        return; // 无需继续处理
                    }

                    // 步骤 7.3：若被顶掉的是“被动型能力”，先停用被动并广播（避免残留效果）
                    if (IsPassiveAbility(*SpecWithSlot))
                    {
                        MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*SpecWithSlot), false);     // 多播：关闭被动表现
                        DeactivatePassiveAbility.Broadcast((GetAbilityTagFromSpec(*SpecWithSlot)));      // 本地/蓝图事件：被动已停用
                    }

                    // 步骤 7.4：清空旧能力的槽位标签（释放该 Slot）
                    ClearSlot(SpecWithSlot);
                    // 注意：此处可视情况 MarkAbilitySpecDirty(*SpecWithSlot) 以立刻同步旧 Spec 的变更（见建议）
                }
            }

            // 步骤 8：如果“本能力此前没有任何槽位”且它是“被动型”，先尝试激活被动（首次绑定触发）
            if (!AbilityHasAnySlot(*AbilitySpec))
            {
                if (IsPassiveAbility(*AbilitySpec))
                {
                    TryActivateAbility(AbilitySpec->Handle);          // 激活被动 GA（通常是持续效果/被动监听）
                    MulticastActivatePassiveEffect(AbilityTag, true); // 多播：打开被动表现（VFX/SFX/UI）
                }
            	//先移除能力状态
            	AbilitySpec->DynamicAbilityTags.RemoveTag(GetStatusFromSpec(*AbilitySpec));
            	//后设置能力状态为已装备
            	AbilitySpec->DynamicAbilityTags.AddTag(GamePlayTags.Abilities_Status_Equipped);
            }

            // 步骤 9：把本能力分配到新槽位（会写入 Spec 的动态源标签）
            AssignSlotToAbility(*AbilitySpec, Slot);
            // 步骤 10：标记该 Spec“脏”，驱动 GAS 复制/同步
            MarkAbilitySpecDirty(*AbilitySpec);
        }
        
        // 步骤 11：向客户端发送“已装备”通知（用于 UI 刷新、快捷栏高亮等）
        ClientEquipAbility(AbilityTag, GamePlayTags.Abilities_Status_Equipped, Slot, PrevSlot);
    }
}


/**
 * @brief 查询指定“槽位”是否为空（即：当前可激活能力列表中，没有任何 GA 标记了该槽位）
 *
 * @param Slot 槽位的 GameplayTag（例如 Abilities.Slot.Q / Abilities.Slot.LMB）
 * @return bool 为空则返回 true（可用于判定“能否装备到该槽位”）
 *
 * 功能说明：
 * - 遍历本 ASC 的“可激活能力列表”，一旦发现有能力的“动态标签”包含该槽位，即认为槽位被占用。
 * - 使用 FScopedAbilityListLock 保护遍历期的能力列表不被并发修改（GAS 内部约定）。
 *
 * 详细流程：
 * 1) 加锁能力列表；2) 遍历每个 FGameplayAbilitySpec；3) 调用 AbilityHasSlot 判断是否带有 Slot 标签；
 * 4) 若命中则立即返回 false；5) 全部未命中则返回 true。
 *
 * 注意事项：
 * - HasTag 为包含匹配，若希望“完全一致”，可改用 HasTagExact（见下方“建议与优化”）。
 * - 此处使用的是 Spec.DynamicAbilityTags（旧 API 会在新版产生警告），建议迁移到 GetDynamicSpecSourceTags。
 * - 遍历复杂度 O(N)，N 为可激活能力数量；一般足够快，如需高频查询可做缓存映射。
 */
bool UAuraAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot)
{
	// 步骤 1：加锁能力列表，防止遍历过程中被修改（GAS 线程/复制安全约定）
	FScopedAbilityListLock ActiveScopeLock(*this); // 进入作用域即加锁，函数结束自动解锁

	// 步骤 2：遍历当前“可激活能力”集合
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities()) // 逐个检查能力规格（Spec）
	{
		// 步骤 3：判断该能力是否“拥有该槽位”
		if (AbilityHasSlot(AbilitySpec, Slot))     // 命中即说明槽位已被占用
		{
			return false;                          // 立即返回“非空”
		}
	}

	// 步骤 4：遍历完成仍未命中 → 槽位为空
	return true;                                   // 空槽位
}

/**
 * @brief 判断某个能力规格（Spec）的“动态标签”中，是否包含指定槽位标签
 *
 * @param Spec 能力规格（包含该 GA 的激活句柄、动态标签等）
 * @param Slot 槽位标签（如 Abilities.Slot.XXX）
 * @return bool 若 Spec 的动态标签包含 Slot，则返回 true
 *
 * 功能说明：
 * - 槽位通常不放在 AbilityTags（静态标签）里，而是放在 DynamicAbilityTags（运行时可改）；
 * - 这里直接用 HasTag 做包含匹配（父子层级也会匹配）。
 *
 * 注意事项：
 * - UE5.4+ 建议改用 Spec.GetDynamicSpecSourceTags().HasTag(Slot)（旧字段会产生弃用警告）。
 * - 若你希望“必须精确等于某个槽位标签”，请用 HasTagExact。
 */
bool UAuraAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	return Spec.GetDynamicSpecSourceTags().HasTagExact(Slot); // 完全匹配
}

/**
 * @brief 判断某个能力 Spec 是否“拥有任意槽位/输入标签”
 *
 * @param Spec  要检查的能力规格
 * @return bool 若 Spec 的“动态源标签”中存在任意“槽位/输入”类标签，则返回 true
 *
 * 功能说明：
 * - 这里用父标签 "InputTag" 做约定：凡是属于该父标签命名空间的（如 InputTag.LMB/E/Q…），都视为“一个槽位”。
 * - 通过检查动态源标签中是否“匹配”该父标签来判断是否“已占用任意槽位”。
 *
 * 注意事项：
 * - HasTag(Parent) 的匹配语义依赖引擎版本与容器实现；若要更严格，建议遍历容器并逐个 MatchesTag(Parent) 检查（见下方“建议与优化”）。
 * - 若你的项目用的是 “Abilities.Slot.*” 作为槽位命名，则应把父标签改为对应前缀。
 */
bool UAuraAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& Spec)
{
	// 用父标签“InputTag”作为槽位命名空间（约定所有具体槽位都在其之下）
	return Spec.GetDynamicSpecSourceTags().HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))); // 命中父标签即认为有槽位
}
/**
 * @brief 通过“槽位标签”在当前 ASC 的可激活能力中查找对应的 Spec 指针
 *
 * @param Slot 槽位标签（如 Abilities.Slot.LMB / Abilities.Slot.Q），要求精确匹配
 * @return FGameplayAbilitySpec* 找到则返回该 Spec 的指针；找不到返回 nullptr
 *
 * 功能说明：
 * - 对可激活能力列表做一次线性遍历，查找“动态源标签”中**精确包含** Slot 的能力。
 * - 使用 FScopedAbilityListLock 保护遍历期间的列表一致性（GAS 约定）。
 *
 * 详细流程：
 * 1) 加锁能力列表；2) 遍历每个 Spec；3) 用 GetDynamicSpecSourceTags().HasTagExact(Slot) 判断；
 * 4) 命中立即返回该 Spec；5) 遍历完未命中则返回 nullptr。
 *
 * 注意事项：
 * - 若项目中允许一个槽位绑定多个能力，需要约定“优先返回哪一个”；当前实现返回**第一个命中**的 Spec。
 * - 若高频调用，可考虑维护“Slot → SpecHandle”的缓存表以降到 O(1) 查询（见文末建议）。
 */
//（命名建议：GetSpecWithSlot；当前函数名 GetSpecWithSlot 拼写可能有误）
FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecWithSlot(const FGameplayTag& Slot)
{
	// 步骤 1：加锁能力列表，防止遍历过程中被修改（复制/添加/移除）
	FScopedAbilityListLock ActiveScopeLock(*this); // 作用域结束自动解锁

	// 步骤 2：遍历所有“可激活能力”规格
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities()) // 线性扫描 O(N)
	{
		// 步骤 3：判断该能力的“动态源标签”是否**精确**包含该槽位标签
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(Slot))   // 精确匹配，避免父/子标签误判
		{
			return &AbilitySpec;                                        // 命中：返回指向该 Spec 的指针
		}
	}

	// 步骤 4：未命中任何能力，返回空指针
	return nullptr;                                                     // 未绑定该槽位
}


/**
 * @brief 判断传入的能力 Spec 是否为“被动型”能力（Passive）
 *
 * @param AbilitySpec 待判断的能力规格（包含能力实例/标签等）
 * @return bool 是被动能力则返回 true，否则 false
 *
 * 功能说明：
 * - 通过项目的 AbilityInfo（数据表/数据资产）从“能力标签”映射到“能力类型标签”，
 *   再和全局被动类型标签 Abilities.Type.Passive 做精确比对。
 *
 * 详细流程：
 * 1) 通过库函数 GetAbilityInfo(Avatar) 拿到 AbilityInfo 数据； 
 * 2) 从 Spec 提取该能力的“能力标签”（如 Abilities.Fireball）；
 * 3) 在 AbilityInfo 里查到该标签对应的信息（FAuraAbilityInfo），取出 AbilityType；
 * 4) 与全局标签单例中的 Abilities_Type_Passive 精确匹配，返回判断结果。
 *
 * 注意事项：
 * - 需保证 AbilityInfo 不为 nullptr，且 FindAbilityInfoForTag 能处理“未找到”的情况（返回默认 info）；
 * - GetAbilityTagFromSpec 的实现要明确：当一个 Spec 挂了多个标签时选哪个作为“能力标签”。
 */
bool UAuraAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& AbilitySpec) const
{
	// 步骤 1：通过 Avatar 拿到 AbilityInfo（通常是一个集中配置的数据资产/表）
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor()); // 可能为 nullptr（取决于实现）
	
	// 步骤 2：从 Spec 中提取“能力标签”（项目自定义，通常来自 AbilityTags）
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(AbilitySpec);                           // 用于在表中查询

	// 步骤 3：在 AbilityInfo 中查找该能力的配置信息（若未找到应返回默认/空信息）
	const FAuraAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);                // 查表得到类型等

	// 步骤 4：取出“能力类型”标签，与“被动类型”做精确比对
	const FGameplayTag AbilityType = Info.AbilityType;                                            // 例如 Abilities.Type.Passive
	return AbilityType.MatchesTagExact(FAuraGamePlayTags::Get().Abilities_Type_Passive);          // 精确匹配：是被动则 true
}

/**
 * @brief 将“槽位标签”分配给指定能力 Spec（先清空旧槽位，再写入新槽位）
 *
 * @param Spec  要操作的能力规格（FGameplayAbilitySpec），通常来自 ASC 的可激活能力列表
 * @param Slot  槽位标签（如 Abilities.Slot.LMB / Abilities.Slot.Q），建议为叶子标签并使用精确匹配
 *
 * 详细流程：
 * 1) 调用 ClearSlot(&Spec) 移除该 Spec 现有的所有“槽位/输入”类动态标签；
 * 2) 使用 GetDynamicSpecSourceTags().AddTag(Slot) 把新槽位写入到 Spec 的“动态源标签”中。
 *
 * 注意事项：
 * - 槽位语义基于“动态标签”，便于运行时切换；请确保 ClearSlot 的实现会移除“InputTag.* / Abilities.Slot.*”等旧槽位；
 * - 若此操作可能与能力列表的遍历/复制并发发生，外层应使用 FScopedAbilityListLock 加锁；
 * - Slot 应该是有效标签且来自统一的父标签命名空间（如 InputTag.*），避免含糊匹配。
 */
void UAuraAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	ClearSlot(&Spec);                                       // 先清空旧槽位（防止同一能力绑定多个槽位）
	Spec.GetDynamicSpecSourceTags().AddTag(Slot);          // 将新槽位写入到“动态源标签”容器
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
void UAuraAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
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


