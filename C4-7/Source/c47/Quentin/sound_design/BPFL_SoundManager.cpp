// Fill out your copyright notice in the Description page of Project Settings.


#include "c47/Quentin/sound_design/BPFL_SoundManager.h"

#include "Components/AudioComponent.h"

void UBPFL_SoundManager::PlaySpecificSound(TArray<UAudioComponent*> SoundList, UAudioComponent* SoundToPlay, float Volume, bool Loop)
{
	SoundToPlay->Play();
	SoundToPlay->FadeIn(1.0f, 1.0f);
	
	if (SoundList.Num() < 0)
		return;
	
	for (auto audioClip : SoundList)
	{
		if(audioClip == SoundToPlay)
			continue;

		if(audioClip->IsPlaying())
		{
			audioClip->FadeOut(1.0f, 0.0f);
		}
	}
}
