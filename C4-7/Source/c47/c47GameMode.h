#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "c47GameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDiedSignature, ACharacter*, Character);

class ACheckpointActor;
class APlayerCharacter;

UCLASS(minimalapi)
class Ac47GameMode : public AGameModeBase
{
	GENERATED_BODY()

private:
	UPROPERTY()
	float scallarParam = 1;
	
	UPROPERTY()
	UMaterialInstanceDynamic* matInstance;

public:
	Ac47GameMode();
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Gameplay")
	int32 starToCollect = 5;

	 //UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Gameplay")
	 //UUserWidget* widget;

	const FOnPlayerDiedSignature& GetOnPlayerDied() const { return OnPlayerDied; }

	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void SetCurrentCheckpoint(ACheckpointActor* NewCheckpoint);

	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void RespawnPlayerAtCheckpoint();
	
	virtual void RestartPlayer(AController* NewPlayer) override;
	
	UFUNCTION()//BlueprintImplementableEvent)
	void PlayerDied(ACharacter* Character);
	
	bool isCheckpointIsReady();
protected:
	FTimerHandle MemberTimerHandle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
	ACheckpointActor* currentCheckpoint;
	
	UPROPERTY(BlueprintAssignable)
	FOnPlayerDiedSignature OnPlayerDied;

	void undisolvePlayer();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	void SpawnPlayerAtTransform(const FTransform& SpawnTransform);
};



