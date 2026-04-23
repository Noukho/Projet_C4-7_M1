#include "MovieSceneSequenceID.h"
#include "c47/Quentin/Checkpoints/CheckPointActor.h"
#include "Components/StaticMeshComponent.h"
#include "c47/c47GameMode.h"

ACheckpointActor::ACheckpointActor()
{
	// SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	// RootComponent = SceneComponent;
	
	checkpointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CheckpointMesh"));
	checkpointMesh->SetupAttachment(RootComponent);

	NiagaraPosition = CreateDefaultSubobject<UNiagaraComponent>(TEXT("CheckpointNiagara"));
	NiagaraPosition->SetWorldScale3D(FVector(0.00001,0.00001,0.00001));
	NiagaraPosition->SetRelativeLocation(FVector(0,0,40));
	NiagaraPosition->SetupAttachment(checkpointMesh);

	checkpointIsActive = false;
}
void ACheckpointActor::BeginPlay()
{
	Super::BeginPlay();
	
	checkpointSpawnTransform = GetActorTransform();
	UpdateCheckpointVisuals();
}

void ACheckpointActor::ActivateCheckpoint()
{
	Ac47GameMode* GameMode = GetWorld()->GetAuthGameMode<Ac47GameMode>();
	if (GameMode)
	{
		GameMode->SetCurrentCheckpoint(this);
	}

	checkpointIsActive = true;

	UpdateCheckpointVisuals();
}

void ACheckpointActor::UnactivateCheckpoint()
{
	checkpointIsActive = false;

	UpdateCheckpointVisuals();
}

FTransform ACheckpointActor::GetCheckpointTransform() const
{
	return checkpointSpawnTransform;
}

void ACheckpointActor::UpdateCheckpointVisuals() const
{
	if (checkpointMesh)
	{
		if (checkpointIsActive)
		{
			NiagaraPosition->Activate();
			UE_LOG(LogController, Log, TEXT("Activate Checkpoint..."))
			//checkpointMesh->SetMaterial(0, activeMaterial);
		}
		else if (!checkpointIsActive)
		{
			NiagaraPosition->Deactivate();
			UE_LOG(LogController, Log, TEXT("Deactivate Checkpoint..."))
			//checkpointMesh->SetMaterial(0, inactiveMaterial);
		}
	}
}

void ACheckpointActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

