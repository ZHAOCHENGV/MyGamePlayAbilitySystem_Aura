// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AuraAbilitySystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraAbilityTypes.h"
#include "AuraGamePlayTags.h"
#include "Game/AuraGameModeBase.h"
#include "Interation/CombatInterface.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "GAS/Data/AbilityInfo.h"

/**
 * 构建WidgetController所需的参数集
 * 
 * @param WorldContextObject 世界上下文对象（用于获取游戏实例和全局信息）
 * @param OutWCParams [输出] 构建完成的控件控制器参数（通过结构体返回所有依赖的对象指针）
 * @param OutAuraHUD [输出] 获取到的AuraHUD指针（通过引用返回，便于后续复用HUD对象）
 * @return 是否成功构建参数集（所有关键对象都获取到则返回true，否则返回false）
 * 
 * 功能说明：
 * 本函数用于集中收集并组织构建控件控制器（WidgetController）所需的全部依赖对象。
 * 主要流程包括：
 * 1. 获取主玩家控制器（PlayerController），通常索引0为本地玩家。
 * 2. 从玩家控制器获取当前HUD对象，并尝试转换为自定义的AAuraHUD类型。
 *    若转换失败，则说明当前HUD不是AuraHUD类型，立即返回false。
 * 3. 获取玩家状态（PlayerState），并通过自定义AAuraPlayerState类型访问能力系统组件（ASC）和属性集（AttributeSet）。
 * 4. 将所有获取到的对象指针填充进输出参数结构体OutWCParams，供控件控制器后续初始化使用。
 * 5. 同时通过引用返回AuraHUD对象，避免多次查找。
 * 6. 若任意关键步骤失败（如找不到PlayerController、HUD类型不匹配等），则函数返回false，调用方可据此处理异常流程。
 */
bool UAuraAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject,FWidgetControllerParams& OutWCParams,AAuraHUD*& OutAuraHUD)
{
	// 获取主玩家控制器（索引0）
	APlayerController * PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (PC) 
	{
		// 尝试获取并转换HUD对象（关键步骤）
		// 通过引用传递OutAuraHUD，允许调用方直接获取结果
		OutAuraHUD = Cast<AAuraHUD>(PC->GetHUD());
		
		if (OutAuraHUD)// 验证转换是否成功
		{
			// 获取玩家状态（特定子类）
			AAuraPlayerState * PS = PC->GetPlayerState<AAuraPlayerState>();
			// 获取能力系统组件
			UAbilitySystemComponent * ASC = PS->GetAbilitySystemComponent();
			// 获取属性集
			UAttributeSet * AS = PS->GetAttributeSet();
			// 填充输出结构体
			OutWCParams.AttributeSet = AS;
			OutWCParams.PlayerState = PS;
			OutWCParams.AbilitySystemComponent = ASC;
			OutWCParams.PlayerController = PC;
			return true;// 所有参数成功获取
		}
		return false;// HUD转换失败
	}
	return false;// HUD转换失败
}

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AAuraHUD* AuraHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams,AuraHUD))
	{
		return AuraHUD->GetOverlayWidgetController(WCParams);
	}

	// 如果没有找到对应的控制器或 HUD，返回 nullptr（无效指针）
	return nullptr;
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(
	const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AAuraHUD* AuraHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams,AuraHUD))
	{
		return AuraHUD->GetAttributeMenuWidgetController(WCParams);
	}
	// 如果没有找到对应的控制器或 HUD，返回 nullptr（无效指针）
	return nullptr;
}

USpellMenuWidgetController* UAuraAbilitySystemLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AAuraHUD* AuraHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams,AuraHUD))
	{
		return AuraHUD->GetSpellMenuWidgetController(WCParams);
	}

	// 如果没有找到对应的控制器或 HUD，返回 nullptr（无效指针）
	return nullptr;

}


void UAuraAbilitySystemLibrary::InitializeDefaultAttribute(const UObject* WorldContextObject,ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	// 获取与 Ability System Component（ASC）相关联的 AvatarActor
	AActor * AvatarActor = ASC->GetAvatarActor();

	// 从游戏模式中获取职业信息数据资产
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	// 根据角色职业类型（CharacterClass）获取职业的默认属性信息
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefault(CharacterClass);

	// 主属性：创建一个效果上下文
	FGameplayEffectContextHandle PrimaryAttributesContextHandle = ASC->MakeEffectContext();
	//添加来源对象（AvatarActor）
	PrimaryAttributesContextHandle.AddSourceObject(AvatarActor);
	// 生成主属性的 GameplayEffect 规范实例（GameplayEffectSpec）
	FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributesContextHandle);
	// 将生成的主属性效果应用到 ASC 本身
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());

	// 次属性：创建效果上下文并添加来源对象
	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	SecondaryAttributesContextHandle.AddSourceObject(AvatarActor);
	FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	// 重要属性：创建效果上下文并添加来源对象
	FGameplayEffectContextHandle VitalAttributesContextHandle = ASC->MakeEffectContext();
	VitalAttributesContextHandle.AddSourceObject(AvatarActor);
	FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}

void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC,ECharacterClass CharacterClass)
{
	// 获取角色职业信息
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr)return;
	// 遍历职业的通用技能，逐一赋予这些技能
	for (auto AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		// 创建技能规范，等级为 1
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		// 将技能添加到 AbilitySystemComponent（能力系统组件）中
		ASC->GiveAbility(AbilitySpec);
	}
	// 获取当前职业的默认配置信息（包含初始技能列表等）
	const FCharacterClassDefaultInfo& DefaultInfo = CharacterClassInfo->GetClassDefault(CharacterClass);
	// 遍历该职业的初始技能列表
	for (auto AbilityClass : DefaultInfo.StartupAbilities)
	{
		// 通过技能系统的AvatarActor获取战斗接口（用于读取玩家等级）
		if(ASC->GetAvatarActor()->Implements<UCombatInterface>())
		{
			// 创建技能规格（绑定技能类和玩家当前等级）
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass,ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));
			// 将技能赋予角色的技能系统
			ASC->GiveAbility(AbilitySpec);
		}
	}

	

	
}

UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	// 获取当前游戏模式并检查其是否为有效的 Aura 游戏模式
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if(AuraGameMode == nullptr)return nullptr;
	// 获取角色职业信息
	return AuraGameMode->CharacterClassInfo;
}

UAbilityInfo* UAuraAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	// 获取当前游戏模式并检查其是否为有效的 Aura 游戏模式
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if(AuraGameMode == nullptr)return nullptr;
	// 获取技能信息
	return AuraGameMode->AbilityInfo;
}


bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	// 尝试将传入的 EffectContextHandle 转换为 FAuraGameplayEffectContext 指针
	// 使用 static_cast，因为我们确信这个上下文实际上是 FAuraGameplayEffectContext 类型
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// 如果转换成功，则调用 FAuraGameplayEffectContext 中的 IsBlockedHit() 方法，
		// 返回该上下文是否处于“格挡击中”状态
		return AuraEffectContext->IsBlockedHit();
	}
	// 如果转换失败，则返回 false，表示没有“格挡击中”的状态
	return false;
}

/**
 * @brief 从 EffectContext 读取：Debuff 是否命中成功
 * @param EffectContextHandle 输入：一次 GE 应用的上下文句柄（内部应为 FAuraGameplayEffectContext）
 * @return bool               true=Debuff 判定成功；否则 false
 *
 * 背景知识（GAS）：
 * - GameplayEffectContext：承载一次效果的来源信息（Instigator/EffectCauser/自定义扩展字段等）。
 * - 我们扩展了 FAuraGameplayEffectContext，新增了 Debuff 相关标记/数值，便于在任意位置读取。
 *
 * 详细流程：
 * 1) 从句柄中取出底层 Context 指针；
 * 2) 尝试将其转换为 FAuraGameplayEffectContext；
 * 3) 转换成功则读取标记并返回；失败则返回 false。
 *
 * 注意事项：
 * - 本函数是“只读”访问；未修改 EffectContextHandle。
 * - 若句柄中不是 FAuraGameplayEffectContext，static_cast 结果为不匹配类型，指针判空后直接兜底返回。
 */
bool UAuraAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	// 步骤 1：从句柄拿到底层 Context 指针
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) // 取并尝试转换
	{
		// 步骤 2：读取我们扩展的标记（Debuff 是否命中）
		return AuraEffectContext->IsSuccessfulDebuff(); // 直接返回标记
	}
	// 步骤 3：兜底（取不到扩展 Context）
	return false;
}


/**
 * @brief 从 EffectContext 读取：Debuff 每跳伤害（DoT Tick 伤害）
 * @param EffectContextHandle 输入：上下文句柄
 * @return float              每跳伤害；取不到时返回 0.f
 *
 * 详细流程：同上（转换→读取→兜底）
 */
float UAuraAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	// 步骤 1：尝试拿到我们扩展的 Context
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) // 转换成功才读
	{
		// 步骤 2：读取 DebuffDamage
		return AuraEffectContext->GetDebuffDamage(); // 返回数值
	}
	// 步骤 3：兜底
	return 0.f;
}

/**
 * @brief 从 EffectContext 读取：Debuff 持续时间（秒）
 * @param EffectContextHandle 输入：上下文句柄
 * @return float              Debuff 持续时间；取不到时 0.f
 */
float UAuraAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	// 步骤 1：尝试拿到扩展 Context
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) // 判空+转换
	{
		// 步骤 2：读取持续时间
		return AuraEffectContext->GetDebuffDuration(); // 秒
	}
	// 步骤 3：兜底
	return 0.f;
}

/**
 * @brief 从 EffectContext 读取：Debuff Tick 间隔（秒/次）
 * @param EffectContextHandle 输入：上下文句柄
 * @return float              Tick 间隔；取不到时 0.f
 */
float UAuraAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	// 步骤 1：尝试拿到扩展 Context
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) // 转换检查
	{
		// 步骤 2：读取频率（间隔）
		return AuraEffectContext->GetDebuffFrequency(); // 秒/次
	}
	// 步骤 3：兜底
	return 0.f;
}

/**
 * @brief 从 EffectContext 读取：DamageType（FGameplayTag）
 * @param EffectContextHandle 输入：上下文句柄
 * @return FGameplayTag       有效则返回具体 Tag；取不到时返回空 Tag
 *
 * 背景：
 * - 我们把 DamageType 用 TSharedPtr<FGameplayTag> 存在 Context 里，读取时需要先判 IsValid()。
 */
FGameplayTag UAuraAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	// 步骤 1：拿到扩展 Context
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) // 转换
	{
		// 步骤 2：内部 Tag 指针判有效
		if (AuraEffectContext->GetDamageType().IsValid()) // 需要先判断
		{
			// 步骤 3：解引用返回实际的 FGameplayTag
			return *AuraEffectContext->GetDamageType(); // 解引用得到值
		}
	}
	// 步骤 4：兜底（返回空 Tag）
	return FGameplayTag();
}

FVector UAuraAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetDeathImpulse();
	}
	
	return FVector::ZeroVector;
}

FVector UAuraAbilitySystemLibrary::GetKnockBackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		
		return AuraEffectContext->GetKnockBackForce();
	}

	return FVector::ZeroVector;
}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	// 尝试将传入的 EffectContextHandle 转换为 FAuraGameplayEffectContext 指针
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// 如果转换成功，则返回该上下文中是否记录了暴击状态
		return AuraEffectContext->IsCriticalHit();
	}
	// 如果转换失败，则返回 false
	return false;
}

void UAuraAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	// 尝试将传入的 EffectContextHandle 转换为 FAuraGameplayEffectContext 指针
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// 如果转换成功，则调用 SetIsBlockedHit() 方法设置格挡击中状态
		// 参数 bInIsBlockedHit 为传入的布尔值，用于指示是否为格挡击中
		 AuraEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle,bool bInIsCriticalHit)
{
	// 尝试将传入的 EffectContextHandle 转换为 FAuraGameplayEffectContext 指针
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// 如果转换成功，则调用 SetIsCriticalHit() 方法设置暴击状态
		 AuraEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void UAuraAbilitySystemLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle,bool bInSuccessfulDebuff)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetIsSuccessfulDebuff(bInSuccessfulDebuff);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDamage)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDebuffDamage(InDamage);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle, float InDuration)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDebuffDuration(InDuration);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle, float InFrequency)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDebuffFrequency(InFrequency);
	}
}

void UAuraAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle,const FGameplayTag& InDamageType)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		const TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(InDamageType);
		AuraEffectContext->SetDamageType(DamageType);
	}
}

void UAuraAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle,const FVector& InImpulse)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDeathImpulse(InImpulse);
	}
}

void UAuraAbilitySystemLibrary::SetKnockBackForce(FGameplayEffectContextHandle& EffectContextHandle,const FVector& InForce)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetKnockBackForce(InForce);
	}
}

/**
 * 在指定球形区域内检索所有存活的玩家角色，并输出到数组
 * 
 * @param WorldContextObject    用于获取World上下文的对象（通常传入this）
 * @param OutOverlappingActors  输出参数，存放符合条件的Actor数组
 * @param ActorsToIgnore        需要忽略检测的Actor数组
 * @param Radius                检测范围的半径（单位：厘米）
 * @param SphereLocation        球形检测的中心点世界坐标
 * 
 * @note 通过CombatInterface接口判断角色存活状态，要求目标Actor必须：
 *       1. 实现UCombatInterface接口
 *       2. IsDead接口返回false（存活状态）
 */
void UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject,
	TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius,
	const FVector& SphereLocation)
{
	// 初始化碰撞检测参数
	FCollisionQueryParams SphereParams;
	// 设置需要忽略的Actor
	SphereParams.AddIgnoredActors(ActorsToIgnore);
	// 安全获取World对象
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		//创建检测碰撞数组
		TArray<FOverlapResult> Overlaps;
		// 执行球形区域检测（仅检测动态对象）  
		World->OverlapMultiByObjectType(
			Overlaps,//检测到的会传入Overlaps
			SphereLocation, // 检测中心点
			FQuat::Identity, // 无旋转
			FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects),// 检测所有动态对象
			FCollisionShape::MakeSphere(Radius),// 创建球形检测区域半径
			SphereParams// 碰撞参数（包含忽略列表）
			);
		// 遍历检测结果Overlaps
		for (FOverlapResult& Overlap: Overlaps)
		{
			//判断是否继承接口，并且存活
			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				//添加到输出数组中
				OutOverlappingActors.AddUnique(Overlap.GetActor());
			}
		}


	}
}

/**
 * 根据角色职业和等级获取对应的经验奖励值
 * 
 * @param WorldContextObject 世界上下文对象（用于获取游戏数据资产）
 * @param CharacterClass 角色职业枚举
 * @param CharacterLevel 目标角色等级
 * @return 计算后的经验奖励值（整数）
 * 
 * @note 需要确保CharacterClassInfo数据资产已正确配置
 */
int32 UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject,ECharacterClass CharacterClass, int32 CharacterLevel)
{
	// 获取角色职业配置数据资产
	// 注意：GetCharacterClassInfo应为自定义的辅助函数
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	// 安全检查：确保数据资产有效
	if(CharacterClassInfo == nullptr)return 0;

	// 获取指定职业的默认配置信息
	// 注意：应确保CharacterClass参数在合法范围内
	const FCharacterClassDefaultInfo& Info = CharacterClassInfo->GetClassDefault(CharacterClass);
	// 从曲线表中获取对应等级的经验奖励（浮点数）
	// 假设XPReward是FRichCurve或FCurveTableRowHandle类型
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);

	// 转换为整数
	return static_cast<int32>(XPReward);
}

/**
 * @brief 依据参数集构建并应用一次伤害型 GameplayEffect
 *
 * @param DamageEffectParams 伤害参数集（来源/目标 ASC、GE 类、等级、基础伤害、伤害类型与各类 Debuff 参数）
 * @return FGameplayEffectContextHandle 返回此次应用所用的 Effect 上下文（含施加者、来源对象等），便于受击端或 Cue 读取
 *
 * 功能说明：
 * - 将外部准备好的伤害与 Debuff 数值通过 SetByCaller 注入到 GE Spec，并在目标 ASC 上应用，实现属性变更与网络同步。
 *
 * 详细流程：
 * 1) 获取全局标签与来源 Avatar；
 * 2) 用来源 ASC 创建 EffectContext 并补充 SourceObject；
 * 3) 生成 GE Spec（类、等级、上下文）；
 * 4) 依次注入伤害与 Debuff 的 SetByCaller 数值；
 * 5) 在目标 ASC 上应用该 Spec；
 * 6) 返回 Context 供后续查询来源信息。
 *
 * 注意事项：
 * - 确保 DamageGameplayEffectClass 内读取的 SetByCaller 标签与此处一致；多人环境建议在服务器端调用（Authority）。
 * - 目标 ASC 必须有效；若 SpecHandle 无效或 GEClass 为空，将无法生效（本函数未做防御，可在调用处校验）。
 */
FGameplayEffectContextHandle UAuraAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
	// 获取项目统一的 GameplayTags 映射（单例），便于引用 Debuff_* 等标准标签
	const FAuraGamePlayTags& GameplayTags = FAuraGamePlayTags::Get();

	// 从来源 ASC 取到 AvatarActor（通常是 Pawn/Character）。会进入 Context，方便在执行/表现层追踪“谁造成的伤害”
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

	// 以来源 ASC 创建 EffectContext：携带 Instigator、EffectCauser、命中信息等；Context 会随 Spec 一起复制到客户端
	FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();

	// 将来源对象写入 Context（这里用 Avatar 本体；也可换成武器/技能实例）。Execution Calculation 与 GameplayCue 都能读取到
	EffectContextHandle.AddSourceObject(SourceAvatarActor);
	//设置死亡冲击向量
	SetDeathImpulse(EffectContextHandle,DamageEffectParams.DeathImpulse);
	//设置击退力
	SetKnockBackForce(EffectContextHandle,DamageEffectParams.KnockBackForce);
	// 基于 GE 类、技能等级与 Context 生成“效果规格单”Spec
	// 注意：SpecHandle 内部持有到 FGameplayEffectSpec 的共享指针；生成成功后再写入各类 SetByCaller
	const FGameplayEffectSpecHandle SpecHandle =
		DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(
			DamageEffectParams.DamageGameplayEffectClass,   // 伤害型 GE 类（应在其中读取 SByC）
			DamageEffectParams.AbilityLevel,                // 按此等级计算系数（如曲线/系数表）
			EffectContextHandle                             // 携带来源/命中信息
		); 

	// —— 向 Spec 注入所有“动态参数”（SetByCaller），键为 GameplayTag，值为 float —— 
	// 基础伤害：键为“具体伤害类型”Tag（如 Damage.Fire），便于同一 GE 支持多种伤害通道
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);

	// Debuff 触发概率：Execution/Modifier 中按同一 Tag 读取（数值区间规范由读取端决定：0~1 或 0~100）
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);

	// Debuff 每跳伤害：用于 DoT/周期伤害计算（与 Frequency/Duration 共同决定总伤）
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);

	// Debuff 持续时间（秒）：设定效果总时长或定时器寿命
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);

	// Debuff Tick 频率：通常表示“每隔多少秒触发一次”（读取端需约定语义，避免与 Hz 概念混淆）
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency); 

	// 在目标 ASC 上“自施”该 Spec：返回的 ActiveHandle 可用于移除/查询（此处未保存）
	// 网络：在服务器调用会创建可复制的 ActiveGE，同步到客户端；Context/SetByCaller 数值也随之同步
	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

	// 返回 Context：受击端、Cue 或日志系统可据此获取施加者、武器等来源信息
	return EffectContextHandle; 
}

/**
 * @brief 生成一组“角度均匀分布”的旋转（Rotator）
 * @param Forward   前方方向向量（基准朝向）
 * @param Axis      旋转所围绕的轴（通常是世界Up向量）
 * @param Spread    总扩散角度（度数，左右总角度范围）
 * @param NumRotators 需要生成的旋转数量
 * @return Rotators 结果：等间距分布的旋转数组
 * @details
 *  - 【作用】比如发射扇形子弹，每颗子弹要有不同的旋转方向，但又要平均分布。
 *  - 【流程】
 *    1) 先求出最左边的方向（Forward 绕 Axis 旋转 -Spread/2）；
 *    2) 如果数量>1：按等分角度依次旋转，生成多个方向；
 *    3) 如果数量=1：直接返回 Forward。
 */
TArray<FRotator> UAuraAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators)
{
	TArray<FRotator> Rotators; // 存放最终结果
	
	// 先算“扇形最左边”的方向
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread /2.f, Axis);
	
	if (NumRotators > 1)
	{
		// 每个方向之间的角度间隔
		const float DeltaSpread = Spread / (NumRotators - 1);
		
		// 循环生成多个旋转
		for (int32 i = 0; i < NumRotators; i++)
		{
			// 从最左边开始，逐步加角度，得到新方向
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			
			// 把方向转换成 Rotator（旋转）
			Rotators.Add(Direction.Rotation());
		}
	}
	else
	{
		// 只有一个，直接用原 Forward
		Rotators.Add(Forward.Rotation());
	}
	return Rotators;
}

/**
 * @brief 生成一组“角度均匀分布”的向量（Vector）
 * @param Forward   前方方向向量（基准朝向）
 * @param Axis      旋转所围绕的轴
 * @param Spread    总扩散角度
 * @param NumVectors 需要生成的向量数量
 * @return Vectors 结果：等间距分布的方向向量数组
 * @details
 *  - 和 EvenlySpacedRotators 一样，只是返回的是方向向量，而不是 Rotator。
 *  - 常用于：投射物发射、检测方向、特效扩散等。
 */
TArray<FVector> UAuraAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors)
{
	TArray<FVector> Vectors; // 存放最终结果
	
	// 先算“扇形最左边”的方向
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread /2.f, Axis);
	
	if (NumVectors > 1)
	{
		// 每个方向之间的角度间隔
		const float DeltaSpread = Spread / (NumVectors - 1);
		
		for (int32 i = 0; i < NumVectors; i++)
		{
			// 按角度逐个生成方向向量
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Vectors.Add(Direction);
		}
	}
	else
	{
		// 只有一个方向，就直接 Forward
		Vectors.Add(Forward);
	}
	return Vectors;
}

/**
 * @brief 从 FGameplayAbilitySpec 中“安全地”获取本次激活应使用的 PredictionKey（实例优先，失败回退）
 * @param Spec 目标能力规格（包含授予方式、实例列表、旧 ActivationInfo 等）
 * @return FPredictionKey 返回用于路由 InputPressed/Released 等事件的预测键
 *
 * 功能说明：
 * - GAS 中 Instanced 能力的“正确预测键”保存在“能力实例”的 CurrentActivationInfo 里；
 * - 旧的 `Spec.ActivationInfo` 仅对 NonInstanced 能力有效（且已被弃用），继续使用会导致事件路由不到当前实例；
 * - 本函数按优先级依次尝试：PrimaryInstance → 任一有效实例 → 回退到旧字段，最大化兼容并消除警告。
 *
 * 详细流程：
 * 1) 通过 Spec.GetPrimaryInstance() 直接获取主实例（InstancedPerActor 的常见路径）；
 * 2) 若无主实例，则枚举 Spec.GetAbilityInstances()，找一个含“有效 PredictionKey”的实例；
 * 3) 若仍未找到，回退使用 Spec.ActivationInfo（兼容 NonInstanced 或少见时序）。
 *
 * 注意事项：
 * - 推荐在发送 `InvokeReplicatedEvent(InputPressed/Released, Handle, PredKey)` 前使用本函数取键；
 * - 与 `UAbilityTask_WaitInputPress/Release` 配合时，可在任务创建时设置 `bTestAlreadyPressed/Released=true` 兜底极端时序；
 * - 若你的 GA 是 InstancedPerExecution，可能同时存在多个实例，步骤 2 会选取一个“键有效”的实例以保证事件能被路由。
 */
FPredictionKey UAuraAbilitySystemLibrary::AuraGetPredictionKeyFromSpec_Safe(const FGameplayAbilitySpec& Spec) 
{
	// 步骤 1：优先尝试获取“主实例”（InstancedPerActor 模式通常稳定存在）
	if (UGameplayAbility* Primary = Spec.GetPrimaryInstance()) // 若存在主实例
	{
		return Primary->GetCurrentActivationInfo().GetActivationPredictionKey(); // 直接从实例的 CurrentActivationInfo 取 PredictionKey
	}

	// 步骤 2：若没有主实例，则遍历所有实例，挑一个“键有效”的
	const TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances(); // 获取当前 Spec 的所有实例（InstancedPerExecution 可能有多个）
	for (UGameplayAbility* Inst : Instances) // 逐个检查实例
	{
		if (!Inst) continue; // 判空防护
		const FPredictionKey Key = Inst->GetCurrentActivationInfo().GetActivationPredictionKey(); // 取该实例的预测键
		if (Key.IsValidKey()) // 若该键有效（非默认/未初始化）
		{
			return Key; // 找到了可用的 PredictionKey，立即返回
		}
	}

	// 步骤 3：仍未找到 → 回退到旧字段（仅 NonInstanced 真正适用；也能兜住少见的过早调用时序）
	return Spec.ActivationInfo.GetActivationPredictionKey(); // 兼容层：避免在极端情况下拿不到键
}


/**
 * @brief 判断两个 Actor 是否“不是朋友”
 * @param FirstActor  第一个角色
 * @param SecondActor 第二个角色
 * @return bool 结果：true=敌对（不是朋友），false=朋友
 * @details
 *  - 如果两个都是 Player，说明是队友 → 返回 false
 *  - 如果两个都是 Enemy，说明是同阵营 → 返回 false
 *  - 其他情况 → 不是朋友 → 返回 true
 */
bool UAuraAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	// 是否都是玩家
	const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));
	
	// 是否都是敌人
	const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));
	
	// 好友关系：同为玩家 或 同为敌人
	const bool bFriends = bBothArePlayers || bBothAreEnemies;
	
	// 返回“不是朋友”
	return !bFriends;
}




