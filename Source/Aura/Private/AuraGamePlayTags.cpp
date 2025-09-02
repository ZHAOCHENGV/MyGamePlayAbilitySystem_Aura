


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
    FString(TEXT("力量:提高物理攻击和负重能力")));

    GamePlayTags.Attributes_Primary_Intelligence = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Intelligence"),
    FString(TEXT("智力:提高魔法攻击和魔法使用效率")));

    GamePlayTags.Attributes_Primary_Resilience = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Resilience"),
    FString(TEXT("体力:提高耐久度和抗击打能力")));

    GamePlayTags.Attributes_Primary_Vigor = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Vigor"),
    FString(TEXT("活力:提高体力恢复和战斗持续能力")));


	/*
	 * 次要属性游戏标签
	 */
    GamePlayTags.Attributes_Secondary_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.Armor"),
    FString(TEXT("护甲:减少伤害，提高格挡几率")));

    GamePlayTags.Attributes_Secondary_ArmorPenetration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.ArmorPenetration"),
    FString(TEXT("护甲穿透:增加穿透敌人护甲的能力")));

    GamePlayTags.Attributes_Secondary_BlockChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.BlockChance"),
    FString(TEXT("阻挡几率:提高受到攻击时格挡的概率")));

    GamePlayTags.Attributes_Secondary_CriticalHitChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalHitChance"),
    FString(TEXT("暴击率:提高攻击时造成暴击的几率")));

    GamePlayTags.Attributes_Secondary_CriticalHitDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalHitDamage"),
    FString(TEXT("暴击伤害:暴击时增加额外伤害")));

    GamePlayTags.Attributes_Secondary_CriticalHitResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalHitResistance"),
    FString(TEXT("暴击抗性:减少受到暴击伤害的比例")));

    GamePlayTags.Attributes_Secondary_HealthRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.HealthRegeneration"),
    FString(TEXT("血量恢复:随着时间逐渐恢复生命值")));

    GamePlayTags.Attributes_Secondary_ManaRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.ManaRegeneration"),
    FString(TEXT("魔法值恢复:随着时间逐渐恢复魔法值")));

	GamePlayTags.Attributes_Secondary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.MaxHealth"),
    FString(TEXT("最大生命:角色能够承受的最大生命值")));

	GamePlayTags.Attributes_Secondary_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.MaxMana"),
    FString(TEXT("最大魔法值:角色能够拥有的最大魔法值")));

	/*
	 * 元属性
	 */
	
	GamePlayTags.Attributes_Meta_IncomingXP = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Meta.IncomingXP"),
	FString(TEXT("XP:传入的经验值")));

	/*
	 * 输入操作标签
	 */
	GamePlayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.LMB"),
	FString(TEXT("左键:鼠标输入左键标签")));
	
	GamePlayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.RMB"),
	FString(TEXT("右键:鼠标输入右键标签")));
	
	GamePlayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.1"),
	FString(TEXT("1:键盘1输入按键标签")));
	
	GamePlayTags.InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.2"),
	FString(TEXT("2:键盘2输入按键标签")));
	
	GamePlayTags.InputTag_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.3"),
	FString(TEXT("3:键盘3输入按键标签")));
	
	GamePlayTags.InputTag_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.4"),
	FString(TEXT("4:键盘4输入按键标签")));
	
	GamePlayTags.InputTag_5 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.5"),
	FString(TEXT("5:键盘5输入按键标签")));
	
	GamePlayTags.InputTag_Passive_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Passive.1"),
	FString(TEXT("Passive_1:被动技能1")));
	
	GamePlayTags.InputTag_Passive_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Passive.2"),
	FString(TEXT("Passive_2:被动技能2")));

	/*
	 * 伤害类型标签
	 */
	GamePlayTags.Damage= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage"),
	FString(TEXT("伤害")));
	GamePlayTags.Damage_Fire= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Fire"),
	FString(TEXT("伤害类型_火焰")));
	GamePlayTags.Damage_Lightning= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Lightning"),
	FString(TEXT("伤害类型_闪电")));
	GamePlayTags.Damage_Arcane= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Arcane"),
	FString(TEXT("伤害类型_奥术")));
	GamePlayTags.Damage_Physical= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Physical"),
	FString(TEXT("伤害类型_物理")));

	
	/*
	 * 减益类型
	 */

	GamePlayTags.Debuff_Burn= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Burn"),
	FString(TEXT("减益效果 燃烧")));
	GamePlayTags.Debuff_Stun= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Stun"),
	FString(TEXT("减益效果 眩晕")));
	GamePlayTags.Debuff_Physical= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Physical"),
	FString(TEXT("减益效果 物理")));
	GamePlayTags.Debuff_Arcane= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Arcane"),
	FString(TEXT("减益效果 魔法")));

	/*
	 * 减益参数
	 */

	GamePlayTags.Debuff_Chance= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Chance"),
	FString(TEXT("减益几率")));
	GamePlayTags.Debuff_Damage= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Damage"),
	FString(TEXT("减益伤害")));
	GamePlayTags.Debuff_Frequency= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Frequency"),
	FString(TEXT("减益频率")));
	GamePlayTags.Debuff_Duration= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Duration"),
	FString(TEXT("减益持续时间")));

	/*
	 * 伤害类型抗性标签
	 */
	GamePlayTags.Attributes_Resistance_Fire= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Fire"),
	FString(TEXT("火焰伤害抗性")));
	GamePlayTags.Attributes_Resistance_Lightning= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Lightning"),
	FString(TEXT("闪电伤害抗性")));
	GamePlayTags.Attributes_Resistance_Arcane= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Arcane"),
	FString(TEXT("奥术伤害抗性")));
	GamePlayTags.Attributes_Resistance_Physical= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Physical"),
	FString(TEXT("物理伤害抗性")));

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
	FString(TEXT("击中效果")));

	/*
	 * 技能
	 */
	GamePlayTags.Abilities_None= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.None"),
	FString(TEXT("没有技能 - 类似与标签中的空指针")));
	
	GamePlayTags.Abilities_Attack= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Attack"),
	FString(TEXT("技能攻击")));
	GamePlayTags.Abilities_Summon= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Summon"),
	FString(TEXT("召唤攻击")));

	GamePlayTags.Abilities_HitReact= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.HitReact"),
	FString(TEXT("击中反应")));
	
	GamePlayTags.Abilities_Status_Locked= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Status.Locked"),
	FString(TEXT("技能状态：锁定")));
	GamePlayTags.Abilities_Status_Eligible= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Status.Eligible"),
	FString(TEXT("技能状态：符合解锁资格")));
	GamePlayTags.Abilities_Status_Unlocked= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Status.Unlocked"),
	FString(TEXT("技能状态：解锁")));
	GamePlayTags.Abilities_Status_Equipped= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Status.Equipped"),
	FString(TEXT("技能状态：装备")));

	GamePlayTags.Abilities_Type_Offensive= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Type.Offensive"),
	FString(TEXT("技能类型：攻击技能")));
	GamePlayTags.Abilities_Type_Passive= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Type.Passive"),
	FString(TEXT("技能类型：被动技能")));
	GamePlayTags.Abilities_Type_None= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Type.None"),
	FString(TEXT("技能类型：无")));
	
	
	GamePlayTags.Abilities_Fire_FireBolt= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Fire.FireBolt"),
	FString(TEXT("火球技能")));
	GamePlayTags.Abilities_Lightning_Electrocute= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Lightning.Electrocute"),
	FString(TEXT("闪电技能")));
	GamePlayTags.Abilities_Arcane_ArcaneShards= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Arcane.ArcaneShards"),
	FString(TEXT("奥术碎片技能")));

	GamePlayTags.Abilities_Passive_HaloOfProtection= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Passive.HaloOfProtection"),
FString(TEXT("被动技能：护盾")));
	GamePlayTags.Abilities_Passive_LifeSiphon= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Passive.LifeSiphon"),
FString(TEXT("被动技能：生命摄取")));
	GamePlayTags.Abilities_Passive_ManaSiphon= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Passive.ManaSiphon"),
FString(TEXT("被动技能：法力摄取")));
	/*
	 * 技能冷却
	 */
	GamePlayTags.Cooldown_Fire_FireBolt= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.Fire.FireBolt"),
	FString(TEXT("火球冷却")));
	GamePlayTags.Cooldown_Lightning_Electrocute= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.Lightning.Electrocute"),
	FString(TEXT("闪电冷却")));
	GamePlayTags.Cooldown_Arcane_ArcaneShards= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.Arcane.ArcaneShards"),
	FString(TEXT("奥术碎片冷却")));
	/*
	 * 战斗插槽
	 */
	GamePlayTags.CombatSocket_Weapon= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("CombatSocket.Weapon"),
	FString(TEXT("武器插槽")));
	GamePlayTags.CombatSocket_RightHand= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("CombatSocket.RightHand"),
	FString(TEXT("右手插槽")));
	GamePlayTags.CombatSocket_LeftHand= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("CombatSocket.LeftHand"),
	FString(TEXT("左手插槽")));
	GamePlayTags.CombatSocket_Tail= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("CombatSocket.Tail"),
	FString(TEXT("尾部插槽")));

	
	/*
	 * 蒙太奇插槽
	 */
	GamePlayTags.Montage_Attack_1= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.1"),
	FString(TEXT("攻击 1")));
	GamePlayTags.Montage_Attack_2= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.2"),
	FString(TEXT("攻击 2")));
	GamePlayTags.Montage_Attack_3= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.3"),
	FString(TEXT("攻击 3")));
	GamePlayTags.Montage_Attack_4= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.4"),
	FString(TEXT("攻击 4")));

	/*
	 * 玩家操作
	 */
	GamePlayTags.Player_Block_CursorTrace= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Player.Block.CursorTrace"),
	FString(TEXT("阻止光标检查")));
	GamePlayTags.Player_Block_InputHeld= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Player.Block.InputHeld"),
	FString(TEXT("阻止鼠标长按")));
	GamePlayTags.Player_Block_InputPressed= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Player.Block.InputPressed"),
	FString(TEXT("阻止输入按下")));
	GamePlayTags.Player_Block_InputReleased= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Player.Block.InputReleased"),
	FString(TEXT("阻止鼠标已释放")));
}
