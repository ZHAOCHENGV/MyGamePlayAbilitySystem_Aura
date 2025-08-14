


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

    GamePlayTags.Attributes_Secondary_CriticalHitResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalHitResistance"),
    FString("暴击抗性:减少受到暴击伤害的比例"));

    GamePlayTags.Attributes_Secondary_HealthRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.HealthRegeneration"),
    FString("血量恢复:随着时间逐渐恢复生命值"));

    GamePlayTags.Attributes_Secondary_ManaRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.ManaRegeneration"),
    FString("魔法值恢复:随着时间逐渐恢复魔法值"));

	GamePlayTags.Attributes_Secondary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.MaxHealth"),
    FString("最大生命:角色能够承受的最大生命值"));

	GamePlayTags.Attributes_Secondary_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.MaxMana"),
    FString("最大魔法值:角色能够拥有的最大魔法值"));

	/*
	 * 元属性
	 */
	
	GamePlayTags.Attributes_Meta_IncomingXP = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Meta.IncomingXP"),
	FString("XP:传入的经验值"));

	/*
	 * 输入操作标签
	 */
	GamePlayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.LMB"),
	FString("左键:鼠标输入左键标签"));
	
	GamePlayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.RMB"),
	FString("右键:鼠标输入右键标签"));
	
	GamePlayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.1"),
	FString("1:键盘1输入按键标签"));
	
	GamePlayTags.InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.2"),
	FString("2:键盘2输入按键标签"));
	
	GamePlayTags.InputTag_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.3"),
	FString("3:键盘3输入按键标签"));
	
	GamePlayTags.InputTag_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.4"),
	FString("4:键盘4输入按键标签"));
	
	GamePlayTags.InputTag_5 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.5"),
	FString("5:键盘5输入按键标签"));
	
	GamePlayTags.InputTag_Passive_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Passive.1"),
	FString("Passive_1:被动技能1"));
	
	GamePlayTags.InputTag_Passive_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Passive.2"),
	FString("Passive_2:被动技能2"));

	/*
	 * 伤害类型标签
	 */
	GamePlayTags.Damage= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage"),
	FString("伤害"));
	GamePlayTags.Damage_Fire= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Fire"),
	FString("伤害类型_火焰"));
	GamePlayTags.Damage_Lightning= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Lightning"),
	FString("伤害类型_闪电"));
	GamePlayTags.Damage_Arcane= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Arcane"),
	FString("伤害类型_奥术"));
	GamePlayTags.Damage_Physical= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Physical"),
	FString("伤害类型_物理"));

	
	/*
	 * 减益类型
	 */

	GamePlayTags.Debuff_Burn= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Burn"),
	FString("减益效果 燃烧"));
	GamePlayTags.Debuff_Stun= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Stun"),
	FString("减益效果 眩晕"));
	GamePlayTags.Debuff_Physical= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Physical"),
	FString("减益效果 物理"));
	GamePlayTags.Debuff_Arcane= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Arcane"),
	FString("减益效果 魔法"));

	/*
	 * 减益参数
	 */

	GamePlayTags.Debuff_Chance= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Chance"),
	FString("减益几率"));
	GamePlayTags.Debuff_Damage= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Damage"),
	FString("减益伤害"));
	GamePlayTags.Debuff_Frequency= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Frequency"),
	FString("减益频率"));
	GamePlayTags.Debuff_Duration= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Duration"),
	FString("减益持续时间"));

	/*
	 * 伤害类型抗性标签
	 */
	GamePlayTags.Attributes_Resistance_Fire= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Fire"),
	FString("火焰伤害抗性"));
	GamePlayTags.Attributes_Resistance_Lightning= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Lightning"),
	FString("闪电伤害抗性"));
	GamePlayTags.Attributes_Resistance_Arcane= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Arcane"),
	FString("奥术伤害抗性"));
	GamePlayTags.Attributes_Resistance_Physical= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Physical"),
	FString("物理伤害抗性"));

	/*
	 * 抗性伤害类型映射
	 */
	GamePlayTags.DamageTypesToResistance.Add(GamePlayTags.Damage_Arcane, GamePlayTags.Attributes_Resistance_Arcane);
	GamePlayTags.DamageTypesToResistance.Add(GamePlayTags.Damage_Lightning, GamePlayTags.Attributes_Resistance_Lightning);
	GamePlayTags.DamageTypesToResistance.Add(GamePlayTags.Damage_Physical, GamePlayTags.Attributes_Resistance_Physical);
	GamePlayTags.DamageTypesToResistance.Add(GamePlayTags.Damage_Fire, GamePlayTags.Attributes_Resistance_Fire);

	GamePlayTags.DamageTypesToDebuffs.Add(GamePlayTags.Damage_Arcane, GamePlayTags.Debuff_Arcane);
	GamePlayTags.DamageTypesToDebuffs.Add(GamePlayTags.Damage_Lightning, GamePlayTags.Debuff_Stun);
	GamePlayTags.DamageTypesToDebuffs.Add(GamePlayTags.Damage_Physical, GamePlayTags.Debuff_Physical);
	GamePlayTags.DamageTypesToDebuffs.Add(GamePlayTags.Damage_Fire, GamePlayTags.Debuff_Burn);


	
	GamePlayTags.Effects_HitReact= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Effects.HitReact"),
	FString("击中效果"));

	/*
	 * 技能
	 */
	GamePlayTags.Abilities_None= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.None"),
	FString("没有技能 - 类似与标签中的空指针"));
	
	GamePlayTags.Abilities_Attack= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Attack"),
	FString("技能攻击"));
	GamePlayTags.Abilities_Summon= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Summon"),
	FString("召唤攻击"));

	GamePlayTags.Abilities_HitReact= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.HitReact"),
	FString("击中反应"));
	
	GamePlayTags.Abilities_Status_Locked= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Status.Locked"),
	FString("技能状态：锁定"));
	GamePlayTags.Abilities_Status_Eligible= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Status.Eligible"),
	FString("技能状态：符合解锁资格"));
	GamePlayTags.Abilities_Status_Unlocked= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Status.Unlocked"),
	FString("技能状态：解锁"));
	GamePlayTags.Abilities_Status_Equipped= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Status.Equipped"),
	FString("技能状态：装备"));

	GamePlayTags.Abilities_Type_Offensive= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Type.Offensive"),
	FString("技能类型：攻击技能"));
	GamePlayTags.Abilities_Type_Passive= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Type.Passive"),
	FString("技能类型：被动技能"));
	GamePlayTags.Abilities_Type_None= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Type.None"),
	FString("技能类型：无"));
	
	
	GamePlayTags.Abilities_Fire_FireBolt= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Fire.FireBolt"),
	FString("火球攻击"));
	GamePlayTags.Abilities_Lightning_Electrocute= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Lightning.Electrocute"),
	FString("闪电攻击"));

	/*
	 * 技能冷却
	 */
	GamePlayTags.Cooldown_Fire_FireBolt= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.Fire.FireBolt"),
	FString("火球冷却"));
	GamePlayTags.Cooldown_Lightning_Electrocute= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.Lightning.Electrocute"),
	FString("闪电冷却"));
	/*
	 * 战斗插槽
	 */
	GamePlayTags.CombatSocket_Weapon= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("CombatSocket.Weapon"),
	FString("武器插槽"));
	GamePlayTags.CombatSocket_RightHand= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("CombatSocket.RightHand"),
	FString("右手插槽"));
	GamePlayTags.CombatSocket_LeftHand= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("CombatSocket.LeftHand"),
	FString("左手插槽"));
	GamePlayTags.CombatSocket_Tail= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("CombatSocket.Tail"),
	FString("尾部插槽"));

	
	/*
	 * 蒙太奇插槽
	 */
	GamePlayTags.Montage_Attack_1= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.1"),
	FString("攻击 1"));
	GamePlayTags.Montage_Attack_2= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.2"),
	FString("攻击 2"));
	GamePlayTags.Montage_Attack_3= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.3"),
	FString("攻击 3"));
	GamePlayTags.Montage_Attack_4= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.4"),
	FString("攻击 4"));
}
