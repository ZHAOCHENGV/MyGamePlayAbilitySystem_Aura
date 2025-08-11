#include "AuraAbilityTypes.h"

bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	// 定义一个 32 位无符号整数，用于存储需要序列化的数据项标志（位域）。
	// 每个位对应一个数据项是否需要序列化。
	uint32 RepBits = 0;
	// 如果当前归档（Ar）处于保存（写入）模式
	if (Ar.IsSaving())
	{
		// 如果需要复制 Instigator，并且 Instigator 对象有效，则将第 0 位设置为 1
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		// 如果需要复制 EffectCauser，并且 EffectCauser 对象有效，则将第 1 位设置为 1
		if (bReplicateEffectCauser && EffectCauser.IsValid() )
		{
			RepBits |= 1 << 1;
		}
		// 如果 AbilityCDO（能力类默认对象）有效，则将第 2 位设置为 1
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		// 如果需要复制 SourceObject，并且 SourceObject 对象有效，则将第 3 位设置为 1
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		// 如果 Actors 数组中有至少一个 Actor，则将第 4 位设置为 1
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		// 如果 HitResult 有效（例如发生了碰撞），则将第 5 位设置为 1
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		// 如果 bHasWorldOrigin 为 true，则将第 6 位设置为 1，表示包含世界原点数据
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		// 如果 bIsBlockedHit 为 true，则将第 7 位设置为 1，表示有阻挡（Blocked Hit）的状态
		if (bIsBlockedHit)
		{
			RepBits |= 1 << 7;
		}
		// 如果 bIsCriticalHit 为 true，则将第 8 位设置为 1，表示为暴击（Critical Hit）
		if (bIsCriticalHit)
		{
			RepBits |= 1 << 8;
		}
	}
	// 将 RepBits 的前 9 位序列化到归档中
	// 这一步是将保存哪些数据项的标志写入到数据流中，方便加载时读取
	Ar.SerializeBits(&RepBits, 9);
	// 根据 RepBits 中各个位的状态，有条件地序列化相应的数据项


	// 如果第 0 位为 1，则序列化 Instigator 数据
	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	// 如果第 1 位为 1，则序列化 EffectCauser 数据
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	// 如果第 2 位为 1，则序列化 AbilityCDO 数据
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	// 如果第 3 位为 1，则序列化 SourceObject 数据
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	// 如果第 4 位为 1，则序列化 Actors 数组（使用安全序列化模板函数，参数 31 表示序列化的最大位数）
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	// 如果第 5 位为 1，则序列化 HitResult 数据
	if (RepBits & (1 << 5))
	{
		// 如果处于加载（反序列化）模式，检查 HitResult 是否有效；若无效，则创建一个新的 FHitResult 实例
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		// 调用 HitResult 的 NetSerialize 方法进行序列化
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	// 如果第 6 位为 1，则序列化 WorldOrigin 数据，并将 bHasWorldOrigin 设置为 true
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}
	// 如果第 7 位为 1，则序列化 bIsBlockedHit
	if (RepBits & (1 << 7))
	{
		Ar << bIsBlockedHit;
	}
	// 如果第 8 位为 1，则序列化 bIsCriticalHit
	if (RepBits & (1 << 8))
	{
		Ar << bIsCriticalHit;
	}
	// 当处于加载模式时，调用 AddInstigator 初始化 InstigatorAbilitySystemComponent（确保反序列化后能正确初始化）
	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get()); // Just to initialize InstigatorAbilitySystemComponent
	}	
	// 序列化成功，将 bOutSuccess 设置为 true，并返回 true
	bOutSuccess = true;
	return true;
}
