// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/EnemyCharacter.h"

#include "AbilitySystemComponent.h"
#include "AI/EnemyAIController.h"
#include "Aura/Aura.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Aura/Public/AuraGamePlayTags.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GAS/AuraAbilitySystemComponent.h"
#include "GAS/AuraAbilitySystemLibrary.h"
#include "GAS/AuraAttributeSet.h"
#include "UI/Widget/AuraUserWidget.h"

void AEnemyCharacter::PossessedBy(AController* NewController)
{
	
	Super::PossessedBy(NewController);
	// 如果当前不是服务器端执行（即是客户端），则跳过后续操作，防止重复执行
	if(!HasAuthority()) return;
	// 将传入的控制器转换为 AEnemyAIController 类型
	EnemyAIController = Cast<AEnemyAIController>(NewController);
	// 初始化黑板，黑板是 BehaviorTree 中用来存储与 AI 行为相关的数据
	EnemyAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	// 启动敌人的行为树（控制敌人 AI 的逻辑）
	EnemyAIController->RunBehaviorTree(BehaviorTree);
	EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
}

AEnemyCharacter::AEnemyCharacter()
{
	//设置网格体碰撞对可视性阻挡
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
	//初始化AbilitySystemComponent组件
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	//设置AbilitySystemComponent组件为可复制
	AbilitySystemComponent->SetIsReplicated(true);
	//设置AbilitySystemComponent组件的复制模式
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	//创建AttributeSet属性集合
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());

	//设置移动速度
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	
}

void AEnemyCharacter::HigHlihtActor()
{
	
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetRenderCustomDepth(true);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void AEnemyCharacter::UnHigHlightActor()
{
	
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

void AEnemyCharacter::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* AEnemyCharacter::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

int32 AEnemyCharacter::GetPlayerLevel_Implementation()
{
	return Level;
}

void AEnemyCharacter::Die()
{
	SetLifeSpan(LifeSpan);
	if(EnemyAIController)EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
	Super::Die();
}


void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();
	if (HasAuthority())
	{
		// 如果有服务器权限（即当前是服务器端），调用给角色赋予初始能力的方法
		UAuraAbilitySystemLibrary::GiveStartupAbilities(this,AbilitySystemComponent,CharacterClass);
	}
	

	// 检查 HealthBar 是否存在有效的 UserWidget 对象，并将其转换为自定义的 UAuraUserWidget 类型
	if (UAuraUserWidget * AuraUserWidget = Cast<UAuraUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		// 如果转换成功，将当前对象（通常是一个控制器类）传递给用户界面控件
		// 目的是让用户界面控件知道它的控制器是哪个类，从而实现双向通信
		AuraUserWidget->SetWidgetController(this);
	}
	// 检查 AttributeSet 是否存在有效对象，并将其转换为自定义的 UAuraAttributeSet 类型
	if (const UAuraAttributeSet * AuraAS = Cast<UAuraAttributeSet>(AttributeSet))
	{
		// 注册监听器，当 Health 属性的值发生变化时，触发 Lambda 回调函数
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData & Data)
			{
				// 广播生命值变化事件，将最新的 Health 值传递给监听者
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);
		// 注册监听器，当 MaxHealth 属性的值发生变化时，触发 Lambda 回调函数
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData & Data)
			{
				// 广播最大生命值变化事件，将最新的 MaxHealth 值传递给监听者
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);

		// 注册Gameplay Tag事件，用于监听“Effects_HitReact”标签的添加或移除
		AbilitySystemComponent->RegisterGameplayTagEvent(
			FAuraGamePlayTags::Get().Effects_HitReact,// 要监听的Gameplay Tag
			EGameplayTagEventType::NewOrRemoved// 监听标签的添加或移除事件
			).AddUObject(
			this,// 绑定的对象
			&AEnemyCharacter::HitReactTagChanged // 当事件触发时调用的回调函数
		);

		
		// 立即广播当前的生命值到监听者（初始化）
		OnHealthChanged.Broadcast(AuraAS->GetHealth());
		// 立即广播当前的最大生命值到监听者（初始化）
		OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
	}







	
}

void AEnemyCharacter::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// 如果标签被添加（NewCount > 0），进入受击状态；否则恢复正常状态
	bHitReacting = NewCount > 0;
	// 如果在受击状态，将角色移动速度设置为0；否则恢复默认速度
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	if (EnemyAIController && EnemyAIController->GetBlackboardComponent())
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
	
}


void AEnemyCharacter::InitAbilityActorInfo()
{
	//设置拥有者Owner Actor和Avater actor 为自身
	AbilitySystemComponent->InitAbilityActorInfo(this,this);
	//获取类型为UAuraAbilitySystemComponent技能组件，并且初始化技能属性集
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	if (HasAuthority())
	{
		//应用游戏效果，来初始化默认属性
        InitializeDefaultAttributes();
	}
	OnAscRegistered.Broadcast(AbilitySystemComponent);
}

void AEnemyCharacter::InitializeDefaultAttributes() const
{
	//传入，职业和等级，能力组件，初始化默认属性
	UAuraAbilitySystemLibrary::InitializeDefaultAttribute(this,CharacterClass,Level,AbilitySystemComponent);
}



