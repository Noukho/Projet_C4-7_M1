#pragma once

#include "CoreMinimal.h"
#include "FTrackedObjectData.generated.h"

UENUM(BlueprintType)
enum class ESynchronisationAction : uint8
{
	Duplicate UMETA(DisplayName = "Duplicate"),
	Replace   UMETA(DisplayName = "Replace"),
	Ignore    UMETA(DisplayName = "Ignore"),
};

USTRUCT(BlueprintType)
struct FTrackedObjectData
{
	GENERATED_BODY()

	FTrackedObjectData()
		: LastTransform(FTransform::Identity)
		, TrackedActor(nullptr)
		, SyncedActor(nullptr)
		, ReplaceActorClass(nullptr)
		, ReplaceActorMesh(nullptr)
		, Action(ESynchronisationAction::Duplicate)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform LastTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<AActor> TrackedActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<AActor> SyncedActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ReplaceActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* ReplaceActorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESynchronisationAction Action;
};
