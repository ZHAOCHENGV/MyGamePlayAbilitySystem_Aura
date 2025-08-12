#include "AuraAbilityTypes.h"
/**
 * @brief 自定义 FAuraGameplayEffectContext 的网络序列化函数
 * @param Ar         网络读写通道（Saving = 写出到网络包；Loading = 从网络包读入）
 * @param Map        UPackageMap：对象与网络 GUID 映射表（用于 UObject 网络传输）
 * @param bOutSuccess 输出：序列化是否成功
 * @return bool      固定返回 true（表示自定义序列化已成功完成）
 *
 * 功能说明：
 * - 使用一个无符号整数 RepBits 当“位图”，每一位对应一个需要复制的字段（例如 Instigator、HitResult 等）。
 * - Saving 阶段：判断各字段有效性 → 按位设置 RepBits → 写出 RepBits → 按位序列化实际数据。
 * - Loading 阶段：读取 RepBits → 按位加载数据（必要时分配对象）→ 补回 Instigator/EffectCauser 引用。
 *
 * 背景知识（UE/GAS）：
 * - NetSerialize：允许开发者自定义网络复制的读写过程，比引擎默认复制更节省带宽或支持特殊类型。
 * - FArchive：UE 的通用序列化接口，Saving / Loading 决定读还是写。
 * - UPackageMap：将 UObject 在网络上传输时转换为 GUID，再在接收端还原。
 * - GameplayEffectContext：描述一次 GE 应用的上下文（施加者、来源对象、命中结果、额外参数等）。
 *
 * 详细流程：
 * 【Saving 路径】
 *   1. 初始化 RepBits = 0。
 *   2. 按字段依次检查有效性（有效 → 对应位 = 1），如：
 *      - bit0: Instigator（施加者 Pawn/角色）
 *      - bit1: EffectCauser（造成效果的对象，比如武器/投射物）
 *      - bit2: AbilityCDO（技能类默认对象）
 *      - bit3: SourceObject（来源对象）
 *      - bit4: Actors 数组（附加的目标集合）
 *      - bit5: HitResult（命中信息）
 *      - bit6: bHasWorldOrigin（世界坐标）
 *      - bit7: bIsBlockedHit（是否格挡命中）
 *      - bit8: bIsCriticalHit（是否暴击）
 *      - bit9: bIsSuccessfulDebuff（减益是否成功）
 *      - bit10: DebuffDamage（减益伤害）
 *      - bit11: DebuffDuration（减益持续时间）
 *      - bit12: DebuffFrequency（减益频率）
 *      - bit13: DamageType（伤害类型 Tag）
 *   3. 写出 RepBits（⚠ 注意位数必须覆盖最高位）。
 *   4. 按位逐个写出实际数据。
 *
 * 【Loading 路径】
 *   1. 读取 RepBits。
 *   2. 按位逐个加载对应字段（必要时动态分配，例如 HitResult、DamageType）。
 *   3. 对 bHasWorldOrigin 之外的未置位字段，保持默认值（注意可能会残留旧值）。
 *   4. 调用 AddInstigator() 补回施加者与效果来源的引用关系。
 *   5. 标记 bOutSuccess = true，返回 true。
 *
 * 注意事项：
 * - 位数要与置位的最高位一致，否则高位字段会被截断。
 * - “非零才复制”的 float 字段（DebuffDamage 等）可能导致清零不同步的问题。
 * - Loading 时未置位的字段不清理可能导致旧数据残留，建议必要时手动清理。
 */

bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	// 准备一个 32 位无符号整数当“位图”，每一位代表一个字段是否需要网络同步
	uint32 RepBits = 0;
	
	// 如果当前是“往外写”（服务器打包/客户端发RPC等）
	if (Ar.IsSaving())
	{
		// 如果允许同步 Instigator 且 Instigator 有值，就把第0位设为1
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		
		// 同理：EffectCauser 对应第1位
		if (bReplicateEffectCauser && EffectCauser.IsValid() )
		{
			RepBits |= 1 << 1;
		}
	
		// AbilityCDO（Ability 的类默认对象）对应第2位
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		
		// SourceObject（比如武器实例）对应第3位
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		
		// Actors 数组非空，对应第4位
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		
		// 有有效的 HitResult，对应第5位
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		
		// 有 WorldOrigin（世界坐标），对应第6位
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		
		// 是否格挡命中（Blocked），对应第7位
		if (bIsBlockedHit)
		{
			RepBits |= 1 << 7;
		}
		
		// 是否暴击（Critical），对应第8位
		if (bIsCriticalHit)
		{
			RepBits |= 1 << 8;
		}

		// Debuff 是否成功，对应第9位
		if (bIsSuccessfulDebuff)
		{
			RepBits |= 1 << 9;
		}

		// Debuff 伤害非零才传，对应第10位
		if (DebuffDamage)
		{
			RepBits |= 1 << 10;
		}

		// Debuff 持续时间非零才传，对应第11位
		if (DebuffDuration)
		{
			RepBits |= 1 << 11;
		}

		// Debuff 频率非零才传，对应第12位
		if (DebuffFrequency)
		{
			RepBits |= 1 << 12;
		}

		// DamageType（FGameplayTag）有效才传，对应第13位
		if (DamageType)
		{
			RepBits |= 1 << 13;
		}
	}

	// 把 RepBits 自己写/读一下，让双方都知道“哪些字段会被同步”
	// ⚠ 这里写的是 13，表示只处理 bit[0..12] 一共13位；
	//    但上面用到了第13位（DamageType），加起来是 14 位（0..13）。
	//    这样会把第13位“截掉”，导致 DamageType 不会被同步。
	Ar.SerializeBits(&RepBits, 13);

	// 如果第0位为1：同步 Instigator
	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	
	// 第1位：同步 EffectCauser
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	
	// 第2位：同步 AbilityCDO
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	
	// 第3位：同步 SourceObject
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	
	// 第4位：同步 Actors 数组（带安全上限的序列化）
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	
	// 第5位：同步 HitResult（读入时要先 new 出来）
	if (RepBits & (1 << 5))
	{
		// 如果是读入，且当前指针还没分配，就先分配一个
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		// 交给 FHitResult 自己的 NetSerialize 去处理内容
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	
	// 第6位：同步 WorldOrigin，并把 bHasWorldOrigin 设为 true；否则设为 false
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}
	
	// 第7位：同步 bIsBlockedHit
	if (RepBits & (1 << 7))
	{
		Ar << bIsBlockedHit;
	}
	
	// 第8位：同步 bIsCriticalHit
	if (RepBits & (1 << 8))
	{
		Ar << bIsCriticalHit;
	}

	// 第9位：同步 bIsSuccessfulDebuff
	if (RepBits & (1 << 9))
	{
		Ar << bIsSuccessfulDebuff;
	}

	// 第10位：同步 DebuffDamage（float）
	if (RepBits & (1 << 10))
	{
		Ar << DebuffDamage;
	}

	// 第11位：同步 DebuffDuration（float）
	if (RepBits & (1 << 11))
	{
		Ar << DebuffDuration;
	}

	// 第12位：同步 DebuffFrequency（float）
	if (RepBits & (1 << 12))
	{
		Ar << DebuffFrequency;
	}

	// 第13位：同步 DamageType（FGameplayTag；用 TSharedPtr 持有，需要先确保有对象）
	if (RepBits & (1 << 13))
	{
		// 读入时如果还没对象，就 new 一个
		if (Ar.IsLoading())
		{
			if (!DamageType.IsValid())
			{
				DamageType = TSharedPtr<FGameplayTag>(new FGameplayTag());
			}
		}
		// 调用 Tag 自己的 NetSerialize
		DamageType->NetSerialize(Ar, Map, bOutSuccess);
	}
	
	// 如果是“读入完成”，把 Instigator/EffectCauser 的关系补回 Context 内部
	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get()); 
	}	
	
	// 标记成功并返回
	bOutSuccess = true;
	return true;
}
