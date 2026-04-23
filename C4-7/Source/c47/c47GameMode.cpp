#include "c47GameMode.h"

#include "c47Character.h"
#include "UObject/ConstructorHelpers.h"
#include "c47/Quentin/Checkpoints/CheckPointActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Util/ElementLinearization.h"

void Ac47GameMode::SetCurrentCheckpoint(ACheckpointActor* NewCheckpoint)
{
	if(NewCheckpoint)
	{
		if(currentCheckpoint != nullptr)
		{
			currentCheckpoint->UnactivateCheckpoint();	
		}
		currentCheckpoint = NewCheckpoint;
	}
}

void Ac47GameMode::RespawnPlayerAtCheckpoint()
{
	FTransform CheckpointTransform;
	if (currentCheckpoint)
	{
		CheckpointTransform = currentCheckpoint->GetCheckpointTransform();
	}
	else
	{
		CheckpointTransform = FindPlayerStart(UGameplayStatics::GetPlayerController(GetWorld(), 0))->GetTransform();
	}
	SpawnPlayerAtTransform(CheckpointTransform);
}

void Ac47GameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
}

void Ac47GameMode::undisolvePlayer()
{
	scallarParam -= 0.02;
	matInstance->SetScalarParameterValue("Offset_Dissolve", scallarParam);
	Ac47Character* player = Cast<Ac47Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if(scallarParam <= -1)
	{
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		UMaterialInterface *baseMaterial = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetMesh()->GetMaterial(1);
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetMesh()->SetMaterial(0, baseMaterial);
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		scallarParam = 1;
		player->SetHealth(player->GetMaxHealth());
		GetWorldTimerManager().ClearTimer(MemberTimerHandle);
	}
}

void Ac47GameMode::BeginPlay()
{
	Super::BeginPlay();
	if (!OnPlayerDied.IsBound())
	{
		OnPlayerDied.AddDynamic(this, &Ac47GameMode::PlayerDied);
	}
}

void Ac47GameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Ac47Character* player = Cast<Ac47Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));
	// if( player->getStar() == starToCollect )
	// {
	// 	
	// 	//widget 
	// }
}

void Ac47GameMode::PlayerDied(ACharacter* Character)
 {
	UE_LOG(LogGameMode, Log, TEXT("player killed, Start Respawn..."))
	RespawnPlayerAtCheckpoint();
	Ac47Character* player = Cast<Ac47Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	matInstance = UMaterialInstanceDynamic::Create(player->DeathAndRespawnMaterial, this);
	matInstance->SetScalarParameterValue("Offset_Dissolve", scallarParam);
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetMesh()->SetMaterial(0,matInstance);

	GetWorldTimerManager().SetTimer(MemberTimerHandle, this,  &Ac47GameMode::undisolvePlayer, 0.02, true, 0);
 }

bool Ac47GameMode::isCheckpointIsReady()
{
	if(currentCheckpoint != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}


void Ac47GameMode::SpawnPlayerAtTransform(const FTransform& SpawnTransform)
{
	const UE::Math::TVector<double> loc=  SpawnTransform.GetLocation();
	const UE::Math::TRotator<double> rot=  SpawnTransform.GetRotation().Rotator();
	ACharacter* player = UGameplayStatics::GetPlayerCharacter(GetWorld(),0);
	
	player->TeleportTo(loc, rot);
	UE_LOG(LogGameMode, Log, TEXT("player Spawn, possess..."))
}

Ac47GameMode::Ac47GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
