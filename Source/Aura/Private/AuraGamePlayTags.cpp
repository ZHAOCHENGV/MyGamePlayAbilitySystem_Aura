


#include "AuraGamePlayTags.h"
#include"GameplayTagsManager.h"


//这是静态成员变量 GamePlayTags 的定义，意思是为 FAuraGamePlayTags 类的静态成员变量 GamePlayTags 分配内存并初始化它。
//在 C++ 中，类的静态成员变量必须在类外部进行定义（即在 .cpp 文件中）。
FAuraGamePlayTags FAuraGamePlayTags::GamePlayTags;


void FAuraGamePlayTags::InitializeNativeGameplayTags()
{
	/*
	 * 主要属性游戏标签
	 */
	// 向游戏标签管理器中添加一个新的游戏标签，并关联变量
    GamePlayTags.Attributes_Primary_Strength = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Strength"),
    FString("力量:提高物理攻击和负重能力"));

    GamePlayTags.Attributes_Primary_Intelligence = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Intelligence"),
    FString("智力:提高魔法攻击和魔法使用效率"));

    GamePlayTags.Attributes_Primary_Resilience = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Resilience"),
    FString("体力:提高耐久度和抗击打能力"));

    GamePlayTags.Attributes_Primary_Vigor = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Vigor"),
    FString("活力:提高体力恢复和战斗持续能力"));


	/*
	 * 次要属性游戏标签
	 */
    GamePlayTags.Attributes_Secondary_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.Armor"),
    FString("护甲:减少伤害，提高格挡几率"));

    GamePlayTags.Attributes_Secondary_ArmorPenetration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.ArmorPenetration"),
    FString("护甲穿透:增加穿透敌人护甲的能力"));

    GamePlayTags.Attributes_Secondary_BlockChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.BlockChance"),
    FString("阻挡几率:提高受到攻击时格挡的概率"));

    GamePlayTags.Attributes_Secondary_CriticalHitChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalHitChance"),
    FString("暴击率:提高攻击时造成暴击的几率"));

    GamePlayTags.Attributes_Secondary_CriticalHitDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalHitDamage"),
    FString("暴击伤害:暴击时增加额外伤害"));

    GamePlayTags.Attributes_Secondary_CriticalResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalResistance"),
    FString("暴击抗性:减少受到暴击伤害的比例"));

    GamePlayTags.Attributes_Secondary_HealthRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.HealthRegeneration"),
    FString("血量恢复:随着时间逐渐恢复生命值"));

    GamePlayTags.Attributes_Secondary_ManaRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.ManaRegeneration"),
    FString("魔法值恢复:随着时间逐渐恢复魔法值"));

	GamePlayTags.Attributes_Secondary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.MaxHealth"),
    FString("最大生命:角色能够承受的最大生命值"));

	GamePlayTags.Attributes_Secondary_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.MaxMana"),
    FString("最大魔法值:角色能够拥有的最大魔法值"));
	

}
