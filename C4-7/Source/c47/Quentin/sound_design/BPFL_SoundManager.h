// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BPFL_SoundManager.generated.h"

/**
 * 
 */
UCLASS()
class C47_API UBPFL_SoundManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = SoundManager)
    static void PlaySpecificSound(TArray<UAudioComponent*> SoundList, UAudioComponent* SoundToPlay, float Volume, bool Loop);
	
};
	