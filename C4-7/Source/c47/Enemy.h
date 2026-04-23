// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "Runtime/AIModule/Classes/Perception/AISightTargetInterface.h"
#include "Enemy.generated.h"

UCLASS()
class C47_API AEnemy : public ACharacter, public IAISightTargetInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FGenericTeamId TeamId;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	int32 ID = 0;

	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

	UFUNCTION(Blueprintable, BlueprintCallable)
	void UpdateWalkSpeed(float newWalkSpeed);

};
