#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "CheckPointActor.generated.h"


UCLASS()
class C47_API ACheckpointActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ACheckpointActor();
	
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
    void ActivateCheckpoint();

	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void UnactivateCheckpoint();
    
    UFUNCTION(BlueprintPure, Category = "Checkpoint")
    FTransform GetCheckpointTransform() const;
	
protected:
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
	// USceneComponent* SceneComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
	UStaticMeshComponent* checkpointMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	UNiagaraComponent* NiagaraPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FTransform checkpointSpawnTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	bool checkpointIsActive;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	// UNiagaraSystem* activeNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	UMaterialInterface* activeMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	UMaterialInterface* inactiveMaterial;

	virtual void BeginPlay() override;

private:	
	void UpdateCheckpointVisuals() const;
};
