// Fill out your copyright notice in the Description page of Project Settings.


#include "KillBox.h"

#include "c47Character.h"
#include "c47GameMode.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AKillBox::AKillBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	boxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	boxCollider->SetupAttachment(RootComponent);
	boxCollider->OnComponentBeginOverlap.AddDynamic(this, &AKillBox::OnBoxBeginOverlap);
	
}

// Called when the game starts or when spawned
void AKillBox::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AKillBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemplateCharacter, Error, TEXT("wah"));

}

void AKillBox::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	Ac47Character* player =  Cast<Ac47Character>(OtherActor);
	if(player)
	{
		player->takeDamage(1, true);
	}
}

