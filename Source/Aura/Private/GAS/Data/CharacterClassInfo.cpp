// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Data/CharacterClassInfo.h"

FCharacterClassDefaultInfo UCharacterClassInfo::GetClassDefault(ECharacterClass CharacterClass)
{
	// 使用 FindChecked 确保 CharacterClass 存在，否则会触发断言
	return CharacterClassInformation.FindChecked(CharacterClass);
}
