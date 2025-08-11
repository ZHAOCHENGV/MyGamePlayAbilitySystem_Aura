// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Data/LevelUpInfo.h"


/**
 * 根据当前经验值计算对应的角色等级
 * @param XP 当前累积的经验值
 * @return 对应的角色等级（最小为1级）
 */

int32 ULevelUpInfo::FindLevelForXp(int32 XP) const
{
	// 初始化当前等级（从1级开始计算）
	int32 Level = 1;
	// 循环控制标志位
	bool bSearching = true;
	// 遍历直到找到合适等级
	while (bSearching)
	{
		// 边界检查：当等级超过数据表最大索引时直接返回
		// 潜在问题：LevelUpInformation[Level]可能导致数组越界
		// 示例：当LevelUpInformation有3个元素（索引0-2）时：
		// LevelUpInformation.Num()-1 = 2
		// 当Level=2时条件成立，但此时LevelUpInformation[2]对应的是3级需求
		if(LevelUpInformation.Num() - 1 <= Level)return Level;

		// 检查经验是否满足升级需求
		if(XP >= LevelUpInformation[Level].LevelUpRequirement)
		{
			// 满足条件则提升等级
			++Level;
		}
		else
		{
			// 经验不足时结束搜索
			bSearching = false;
		}
	}
	// 返回最终确定的等级
	return Level;
}
