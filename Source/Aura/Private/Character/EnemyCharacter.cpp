// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/EnemyCharacter.h"

void AEnemyCharacter::HigHlihtActor()
{
	bHighlighted = true;
	GetMesh()->bRenderCustomDepth = true;
	GetMesh()->CustomDepthStencilValue = 255;
}

void AEnemyCharacter::UnHigHlightActor()
{
	bHighlighted = false;
	GetMesh()->bRenderCustomDepth = false;
}
