// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyController.h"
#include "Enemy.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Runtime/AIModule/Classes/Perception/AISenseConfig_Sight.h"

void AEnemyController::BeginPlay()
{
	Super::BeginPlay();
	AEnemy* enemy = Cast<AEnemy>(GetPawn());
	if (enemy)
	{
		Agent = enemy;
		TeamId = FGenericTeamId(Agent->ID);
	}

	
}

AEnemyController::AEnemyController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SenseConfig"));
	hearing = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

	AIPerceptionComponent->ConfigureSense(*sight);
	AIPerceptionComponent->ConfigureSense(*hearing);
	sight->SightRadius = sightRadius;
	sight->LoseSightRadius = loseSightRadius;
	sight->PeripheralVisionAngleDegrees = peripheralVisionAngleDegrees;

	hearing->HearingRange = hearingRange;
}

void AEnemyController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UE_LOG(LogTemp, Warning, TEXT("%f"), sight->SightRadius);
}

void AEnemyController::OnPerception(AActor* Actor, FAIStimulus Stimulus)
{
}

void AEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AEnemyController::SetConfigSenseValue()
{
	if (sightRadius < 0)
	{
		sightRadius = 0;
	}
	sight->SightRadius = sightRadius;
	if (loseSightRadius < 0)
	{
		loseSightRadius = 0;
	}
	sight->LoseSightRadius = loseSightRadius;
	if (peripheralVisionAngleDegrees > 180)
	{
		peripheralVisionAngleDegrees = 180;
	} else if (peripheralVisionAngleDegrees < 0)
	{
		peripheralVisionAngleDegrees = 0;
	}
	sight->PeripheralVisionAngleDegrees = peripheralVisionAngleDegrees;

	if (hearingRange < 0 )
	{
		hearingRange = 0;
	}
	hearing->HearingRange = hearingRange;

	AIPerceptionComponent->RequestStimuliListenerUpdate();
}

ETeamAttitude::Type AEnemyController::GetTeamAttitudeTowards(const AActor& Other) const
{
	const APawn* otherPawn = Cast<APawn>(&Other);
	if (otherPawn == nullptr)
	{
		return  ETeamAttitude::Neutral;
	}

	auto Player = Cast<IGenericTeamAgentInterface>(&Other);
	class IGenericTeamAgentInterface* Bot = Cast<IGenericTeamAgentInterface>(otherPawn->GetController());
	if (Bot == nullptr && Player == nullptr)
	{
		return ETeamAttitude::Neutral;
	}

	FGenericTeamId otherActorTeamId = NULL;
	if (Bot != nullptr)
	{
		otherActorTeamId = Bot->GetGenericTeamId();
		
	}
	else if (Player != nullptr)
	{
		otherActorTeamId = Player->GetGenericTeamId();
	}

	FGenericTeamId ThisId = GetGenericTeamId();
	if ( otherActorTeamId == 8)
	{
		return  ETeamAttitude::Neutral;
	} else if ( otherActorTeamId == ThisId)
	{
		return ETeamAttitude::Friendly;
	}
	else
	{
		return ETeamAttitude::Hostile;
	}
}




