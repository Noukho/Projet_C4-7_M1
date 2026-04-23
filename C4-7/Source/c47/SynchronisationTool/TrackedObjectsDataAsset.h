#pragma once

#include "CoreMinimal.h"
#include "FTrackedObjectData.h"
#include "Engine/DataAsset.h"
#include "TrackedObjectsDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class C47_API UTrackedObjectsDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
	public:

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracked Objects")
		TArray<FTrackedObjectData> TrackedObjects;
};

USTRUCT()
struct FTrackedObjectDataContainer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FTrackedObjectData> TrackedObjects;
};