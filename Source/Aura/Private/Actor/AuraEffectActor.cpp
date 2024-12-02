// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraEffectActor.h"
#include "AbilitySystemInterface.h"
#include "Components/SphereComponent.h"
#include "GAS/AuraAttributeSet.h"


AAuraEffectActor::AAuraEffectActor()
{
 	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());

}

void AAuraEffectActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//获取使用ASC接口的Actor
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OtherActor))
	{
		//1 获取这个Actor的ASC组件
		//2 从ASC组件获取属性集
		//3 Cast类型转换为UAuraAttributeSet属性集
		//UAuraAttributeSet::StaticClass()方法返回一个指向 UClass 的指针,该 UClass 对象为UAuraAttributeSet类型
		const UAuraAttributeSet * AuraAttributeSet = Cast<UAuraAttributeSet>(ASCInterface->GetAbilitySystemComponent()->GetAttributeSet(UAuraAttributeSet::StaticClass()));
		//const_cast 类型转换工具，用于去掉对象的 const 限制。它允许将一个 const 指针转换为非 const 指针
		//这行代码的作用是将const UAuraAttributeSet* 类型的指针转换为 UAuraAttributeSet*
		UAuraAttributeSet * MutableAuraAttributeSet = const_cast<UAuraAttributeSet*>(AuraAttributeSet);
		MutableAuraAttributeSet->SetHealth(AuraAttributeSet->GetHealth()+25.F);
		MutableAuraAttributeSet->SetHealth(AuraAttributeSet->GetMana()+25.F);
		Destroy(); 
	}
	
}

void AAuraEffectActor::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}


void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
	//当球体开始重叠时，绑定OnOverlap函数
	Sphere->OnComponentBeginOverlap.AddDynamic(this,&AAuraEffectActor::OnOverlap);
	//当球体结束重叠时，绑定EndOverlap函数
	Sphere->OnComponentEndOverlap.AddDynamic(this,&AAuraEffectActor::EndOverlap);
}


