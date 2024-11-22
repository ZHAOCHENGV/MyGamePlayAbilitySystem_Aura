


#include "Character/CharacterBase.h"


ACharacterBase::ACharacterBase()
{
 	PrimaryActorTick.bCanEverTick = false;
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(),("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	

}


void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	
}


