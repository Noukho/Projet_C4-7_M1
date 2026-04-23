#include "SynchronisationTool.h"

#include "JsonObjectConverter.h"
#if WITH_EDITOR
#include "Selection.h"
#endif
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/SavePackage.h"


USynchronisationTool* USynchronisationTool::Instance = nullptr;

USynchronisationTool::USynchronisationTool()
{
	#if WITH_EDITOR
		LoadConfig();
		FCoreUObjectDelegates::OnObjectTransacted.AddUObject(this, &USynchronisationTool::OnObjectModified);
		USelection::SelectObjectEvent.AddUObject(this, &USynchronisationTool::OnSelectionChanged);
	#endif
}

USynchronisationTool::~USynchronisationTool()
{
	#if WITH_EDITOR
		FCoreUObjectDelegates::OnObjectTransacted.RemoveAll(this);
	#endif
}

USynchronisationTool* USynchronisationTool::GetTracker()
{
#if WITH_EDITOR
	if (!Instance)
	{
		Instance = NewObject<USynchronisationTool>();
		Instance->AddToRoot();
	}

	return Instance;
#else
	return nullptr;
#endif
}

void USynchronisationTool::TrackObject(AActor* Actor)
{
	#if WITH_EDITOR
		if (Actor)
		{
			int32 ObjectId = Actor->GetUniqueID();
			ObjectStates.Add(ObjectId, Actor->GetActorTransform());
		}
	#endif
}

void USynchronisationTool::ClearTrackedObjects()
{
	#if WITH_EDITOR
		ObjectStates.Empty();
	#endif
}

UTrackedObjectsDataAsset* USynchronisationTool::LoadTrackedObjectsDataAsset()
{
	#if WITH_EDITOR
		const FString AssetPath = TEXT("/Game/TrackedObjectsAsset");
		UTrackedObjectsDataAsset* Asset = Cast<UTrackedObjectsDataAsset>(StaticLoadObject(UTrackedObjectsDataAsset::StaticClass(), nullptr, *AssetPath));

		if (!Asset)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load data asset for TrackedObjects"));

			Asset = NewObject<UTrackedObjectsDataAsset>(GetTransientPackage(), UTrackedObjectsDataAsset::StaticClass(), NAME_None, RF_Public | RF_Standalone);

			if (!Asset)
			{
				UE_LOG(LogTemp, Error , TEXT("Failed to create an data asset for TrackedObjects"));

				return nullptr;
			}
			
			FString PackagePath = TEXT("/Game/TrackedObjectsAsset");
			UPackage* Package = CreatePackage(*PackagePath);
			Asset->Rename(*FString("TrackedObjectsAsset"), Package);
			FAssetRegistryModule::AssetCreated(Asset);
			Package->MarkPackageDirty();
			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
			SaveArgs.SaveFlags = SAVE_NoError;
			UPackage::SavePackage(Package, Asset, *FPackageName::LongPackageNameToFilename(PackagePath, TEXT(".uasset")), SaveArgs);
		}
	
		return Asset;
	#else
		return nullptr;
	#endif
}

void USynchronisationTool::SyncedObjects()
{
	#if WITH_EDITOR
		UTrackedObjectsDataAsset* Asset = LoadTrackedObjectsDataAsset();
	Asset->Modify();

	if (!Asset)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load data asset for TrackedObjects"));
	}
	
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("World is null! Cannot spawn actors."));
		return;
	}

	for (int32 i = 0 ; i < Asset->TrackedObjects.Num() ; i++)
	{
		FTrackedObjectData& TrackedObject = Asset->TrackedObjects[i];
		if (TrackedObject.Action == ESynchronisationAction::Ignore || TrackedObject.SyncedActor != nullptr)
		{
			continue;
		}

		if (!TrackedObject.TrackedActor.IsValid())
		{
			continue;
		}

		AActor* OriginalActor = TrackedObject.TrackedActor.Get();
		FVector SpawnLocation = OriginalActor->GetActorLocation();
		FRotator SpawnRotation = OriginalActor->GetActorRotation();
		
		UStaticMesh* NewStaticMesh = nullptr;

		SpawnLocation += GetDefault<USynchronisationTool>()->NightmareOffset;

		if (TrackedObject.Action == ESynchronisationAction::Replace)
		{
			if (!TrackedObject.ReplaceActorClass)
			{
				continue;
			}

			if (TrackedObject.ReplaceActorClass == AStaticMeshActor::StaticClass() )
			{
				NewStaticMesh = TrackedObject.ReplaceActorMesh;
				
				if (!NewStaticMesh)
				{
					continue;
				}

				FActorSpawnParameters SpawnParams;
				AStaticMeshActor* SpawnedMeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);

				if (SpawnedMeshActor)
				{
					SpawnedMeshActor->SetActorLabel(*FString::Printf(TEXT("%s_Synced"), *NewStaticMesh->GetName().Append("_").Append(*OriginalActor->GetActorLabel())));

					if (UStaticMeshComponent* StaticMeshComp = SpawnedMeshActor->GetStaticMeshComponent())
					{
						StaticMeshComp->SetStaticMesh(NewStaticMesh);

						TrackedObject.SyncedActor = TSoftObjectPtr<AActor>(SpawnedMeshActor);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Failed to get StaticMeshComponent"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to spawn StaticMeshActor"));
				}
			}
			else
			{
				FActorSpawnParameters SpawnParams;
				AActor* SpawnedMeshActor = World->SpawnActor<AActor>(TrackedObject.ReplaceActorClass, SpawnLocation, SpawnRotation, SpawnParams);

				TrackedObject.SyncedActor = TSoftObjectPtr<AActor>(SpawnedMeshActor);
			}
		}

		if (TrackedObject.Action == ESynchronisationAction::Duplicate)
		{
			GEditor->edactDuplicateSelected(OriginalActor->GetLevel(), true);

			TArray<AActor*> SelectedActors;
			GEditor->GetSelectedActors()->GetSelectedObjects(SelectedActors);

			for (AActor* DuplicatedActor : SelectedActors)
			{
				if (DuplicatedActor != OriginalActor)
				{
					DuplicatedActor->SetActorLabel(*FString::Printf(TEXT("%s_Synced"), *OriginalActor->GetActorLabel()));
					DuplicatedActor->SetActorLocation(SpawnLocation);
					TrackedObject.SyncedActor = DuplicatedActor;
				}
			}
		}
	}

	FString PackageName = Asset->GetOutermost()->GetName();

		if (UPackage* Package = FindPackage(nullptr, *PackageName))
	{
		FAssetRegistryModule::AssetCreated(Asset);
		Package->MarkPackageDirty();
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
		UPackage::SavePackage(Package, Asset, *PackageName, SaveArgs);
	}
#endif
}

void USynchronisationTool::InitializeTrackedObjects()
{
	#if WITH_EDITOR
		UTrackedObjectsDataAsset* Asset = LoadTrackedObjectsDataAsset();
		for (int32 i = 0 ; i < Asset->TrackedObjects.Num() ; i++)
		{
			FTrackedObjectData& TrackedObject = Asset->TrackedObjects[i];

			TrackObject(Cast<AActor>(TrackedObject.TrackedActor.Get()));
			TrackObject(Cast<AActor>(TrackedObject.SyncedActor.Get()));
		}
	#endif
}

void USynchronisationTool::editObject(FTrackedObjectData trackedObjectData)
{
#if WITH_EDITOR
	UTrackedObjectsDataAsset* Asset = LoadTrackedObjectsDataAsset();

	if (!Asset)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load data asset while try to synced objects"));
	}

	int32 index = Asset->TrackedObjects.IndexOfByPredicate([trackedObjectData](const FTrackedObjectData& Data)
	{
		return Data.TrackedActor == trackedObjectData.TrackedActor;
	});

	if (index != -1)
	{
		Asset->TrackedObjects.RemoveAt(index);
	}

	Asset->TrackedObjects.Push(trackedObjectData);

	ExportData();
#endif
}

FTrackedObjectData USynchronisationTool::GetTrackedObject(AActor* actorReference)
{
#if WITH_EDITOR
	UTrackedObjectsDataAsset* Asset = LoadTrackedObjectsDataAsset();

	if (!Asset) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load data asset"));

		return FTrackedObjectData();
	}
	
	FTrackedObjectData* FoundData = Asset->TrackedObjects.FindByPredicate([actorReference](const FTrackedObjectData& Data)
				{
					return Data.TrackedActor.Get() == actorReference || Data.SyncedActor.Get() == actorReference;
				});

	if (!FoundData) 
	{
		return FTrackedObjectData();
	}

	return *FoundData;
#else
	return FTrackedObjectData();
#endif
}

void USynchronisationTool::SaveConfigSettings()
{
#if WITH_EDITOR
	SaveConfig(CPF_Config, *GetDefaultConfigFilename());
#endif
}

bool USynchronisationTool::SaveDataAssetToJson(UTrackedObjectsDataAsset* DataAsset)
{
#if WITH_EDITOR
	if (!DataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("No DataAsset!"));

		return false;
	}

	FTrackedObjectDataContainer Container;
	Container.TrackedObjects = DataAsset->TrackedObjects;

	FString JsonString;

	if (!FJsonObjectConverter::UStructToJsonObjectString(Container, JsonString))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to serialize to JSON"));
		return false;
	}

	FString FilePath = FPaths::ProjectContentDir() / TEXT("TrackedObjectsDataAsset.json");
	if (!FFileHelper::SaveStringToFile(JsonString, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save file: %s"), *FilePath);
		return false;
	}

	return true;
#else
	return false;
#endif
}

bool USynchronisationTool::LoadDataAssetFromJson(UTrackedObjectsDataAsset* DataAsset)
{
#if WITH_EDITOR

	if (!DataAsset) return false;

	FTrackedObjectDataContainer Container;
	FString FilePath = FPaths::ProjectContentDir() / TEXT("TrackedObjectsDataAsset.json");
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		return false;
	}

	TArray<FTrackedObjectData> ParsedObjects;
	if (FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &Container, 0, 0, true))
	{
		DataAsset->TrackedObjects = Container.TrackedObjects;
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON into DataAsset!"));
	return false;
#else
	return false;
#endif
}

bool USynchronisationTool::ExportData()
{
#if WITH_EDITOR
	UTrackedObjectsDataAsset* Asset = LoadTrackedObjectsDataAsset();

	if (!Asset) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load data asset"));
		return false;
	}

	return SaveDataAssetToJson(Asset);
#else
	return false;
#endif
}

#if WITH_EDITOR
void USynchronisationTool::OnObjectModified(UObject* Object, const FTransactionObjectEvent& Event)
{
		if (!Object) return;

		AActor* Actor = nullptr;

		if(Object->IsA<AActor>())
		{
			Actor = Cast<AActor>(Object);
		} else if (UActorComponent* Component = Cast<UActorComponent>(Object))
		{
			Actor = Component->GetOwner();
		}

		if (!Actor) return;

		int32 ObjectId = Actor->GetUniqueID();
		FTransform Transform = Actor->GetActorTransform();

		if (FTransform* StoredTransform = ObjectStates.Find(ObjectId))
		{
			if (!Transform.Equals(*StoredTransform))
			{
				ObjectStates[ObjectId] = Transform;

				UTrackedObjectsDataAsset* Asset = LoadTrackedObjectsDataAsset();

				bool isTrackedActorSelected = true;

				int32 FoundIndex = INDEX_NONE;

				FTrackedObjectData* FoundTrackedObject = nullptr;

				for (int32 idx = 0; idx < Asset->TrackedObjects.Num(); idx++)
				{
					FTrackedObjectData trackedObjectData = Asset->TrackedObjects[idx];

					if ((!IsValid(trackedObjectData.TrackedActor.Get()) || !IsValid(trackedObjectData.SyncedActor.Get())) && trackedObjectData.Action != ESynchronisationAction::Ignore)	
					{
						FoundIndex = idx;
						FoundTrackedObject = &trackedObjectData;

						isTrackedActorSelected = !IsValid(trackedObjectData.TrackedActor.Get());

						break;
					}
						
					if (trackedObjectData.TrackedActor.Get()->GetPathName() == Actor->GetPathName())
					{
						isTrackedActorSelected = true;
						FoundTrackedObject = &trackedObjectData;
						
						break;
					}

					if (trackedObjectData.SyncedActor.Get()->GetPathName() == Actor->GetPathName())
					{
						isTrackedActorSelected = false;
						FoundTrackedObject = &trackedObjectData;
						break;
					}
				}
				
				if (FoundTrackedObject && FoundIndex == INDEX_NONE)
				{
					if ((isTrackedActorSelected ? FoundTrackedObject->SyncedActor : FoundTrackedObject->TrackedActor) && FoundTrackedObject->Action != ESynchronisationAction::Ignore)
					{
						if (isTrackedActorSelected)
						{
							FoundTrackedObject->SyncedActor->SetActorLocation(Actor->GetActorLocation() + GetDefault<USynchronisationTool>()->NightmareOffset);
							FoundTrackedObject->SyncedActor->SetActorRotation(Actor->GetActorRotation());
							FoundTrackedObject->SyncedActor->Modify();
						}
						else
						{
							FoundTrackedObject->TrackedActor->SetActorLocation(Actor->GetActorLocation() - GetDefault<USynchronisationTool>()->NightmareOffset);
							FoundTrackedObject->TrackedActor->SetActorRotation(Actor->GetActorRotation());
							FoundTrackedObject->TrackedActor->Modify();
						}
					}
				}
				else
				{
					if (FoundTrackedObject && (IsValid(FoundTrackedObject->SyncedActor.Get()) || IsValid(FoundTrackedObject->TrackedActor.Get())))
					{
						isTrackedActorSelected ? FoundTrackedObject->SyncedActor->Destroy() : FoundTrackedObject->TrackedActor->Destroy();
						Asset->TrackedObjects.RemoveAt(FoundIndex);
					}
				}
			}
		}
}
#endif

#if WITH_EDITOR
void USynchronisationTool::OnSelectionChanged(UObject* Object)
{
		AActor* Actor = Cast<AActor>(Object);
		if (!Actor)
		{
			return;
		}
	
		selectedTrackedActor = GetTrackedObject(Actor);
		onSelectTrackedActor.Broadcast();
}
#endif