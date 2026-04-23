#pragma once

#include "CoreMinimal.h"
#include "FTrackedObjectData.h"
#include "TrackedObjectsDataAsset.h"
#include "UObject/Object.h"
#include "SynchronisationTool.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnselectTrackedActor);

/**
 * 
 */
UCLASS(config = Editor)
class C47_API USynchronisationTool : public UObject
{
	GENERATED_BODY()

public:
	USynchronisationTool();
	virtual ~USynchronisationTool() override;

	UFUNCTION(BlueprintCallable, Category = "Synchronisation Tools")
	static USynchronisationTool* GetTracker();

	UFUNCTION(BlueprintCallable, Category = "Synchronisation Tools")
	void TrackObject(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Synchronisation Tools")
	void ClearTrackedObjects();

	UFUNCTION(BlueprintCallable, Category = "Synchronisation Tools")
	UTrackedObjectsDataAsset* LoadTrackedObjectsDataAsset();

	UFUNCTION(BlueprintCallable, Category = "Synchronisation Tools")
	void SyncedObjects();

	UFUNCTION(BlueprintCallable, Category = "Synchronisation Tools")
	void InitializeTrackedObjects();

	UFUNCTION(BlueprintCallable, Category = "Synchronisation Tools")
	void editObject(FTrackedObjectData trackedObjectData);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synchronisation Tools")
	TArray<FTrackedObjectData> TrackedObjects;

	UFUNCTION(BlueprintCallable, Category = "Synchronisation Tools")
	FTrackedObjectData GetTrackedObject(AActor* actorReference);

	UFUNCTION(BlueprintCallable, Category = "Synchronisation Tools")
	void SaveConfigSettings();

	UFUNCTION(BlueprintCallable, Category = "Synchronisation Tools")
	bool SaveDataAssetToJson(UTrackedObjectsDataAsset* DataAsset);

	UFUNCTION(BlueprintCallable, Category = "Synchronisation Tools")
	bool LoadDataAssetFromJson(UTrackedObjectsDataAsset* DataAsset);

	UFUNCTION(BlueprintCallable, Category = "Synchronisation Tools")
	bool ExportData();

	UPROPERTY(BlueprintAssignable)
	FOnselectTrackedActor onSelectTrackedActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config, Category = "Synchronisation Tools")
	FVector NightmareOffset = FVector(2000.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synchronisation Tools")
	FTrackedObjectData selectedTrackedActor;

private:
	static USynchronisationTool* Instance;
	TMap<int32, FTransform> ObjectStates;

	#if WITH_EDITOR
		void OnObjectModified(UObject* Object, const FTransactionObjectEvent& Event);
		void OnSelectionChanged(UObject* Object);
	#endif
};
