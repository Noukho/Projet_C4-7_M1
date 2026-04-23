// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Perception/AIPerceptionComponent.h"
#include "AIController.h"
#include "EnemyController.generated.h"

class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
/**
 * 
 */
UCLASS()
class C47_API AEnemyController : public AAIController
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay();

	FGenericTeamId TeamId;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

public:
	AEnemyController();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UAIPerceptionComponent* AIPerceptionComponent;
	
	UPROPERTY(BlueprintReadWrite)
	class AEnemy* Agent;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	float sightRadius = 100;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	float loseSightRadius = 100;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	float peripheralVisionAngleDegrees = 45;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	float hearingRange = 100;
	
	UFUNCTION()
	void OnPerception(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	UAISenseConfig_Sight* sight;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	UAISenseConfig_Hearing* hearing;
	
	virtual void OnPossess(APawn* InPawn) override;

	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

	UFUNCTION(BlueprintCallable)
	void SetConfigSenseValue();
	
};
