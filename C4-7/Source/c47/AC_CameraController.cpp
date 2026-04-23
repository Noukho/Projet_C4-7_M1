// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AC_CameraController.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "c47Character.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UAC_CameraController::UAC_CameraController()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;
    // ...
}


// Called when the game starts
void UAC_CameraController::BeginPlay()
{
    Super::BeginPlay();
    Character = Cast<Ac47Character>(GetOwner());
  
    if (IsValid(GetCharacter()))
    {
        this->CameraBoom = GetCharacter()->GetCameraBoom();
        if (IsValid(this->CameraBoom))
        {
            this->CameraBoom->TargetArmLength = DefaultCameraDistance;
            this->CameraBoom->CameraLagSpeed = CameraLagValue;
            this->CameraBoom->CameraRotationLagSpeed = 10.f;
        }
    }
    APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
    if (IsValid(CameraManager))
    {
        CameraManager->ViewPitchMin = this->DefaultMinPitchAngle;
        CameraManager->ViewPitchMax = this->DefaultMaxPitchAngle;
    }
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (IsValid(PlayerController))
    {
        PlayerController->SetControlRotation(this->DefaultCameraAngle.Rotation());
    }
    LookAhead();
}

void UAC_CameraController::BeginDestroy()
{
    Super::BeginDestroy();
}


// Called every frame
void UAC_CameraController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (IsTeleporting)
    {
        if (TeleportTimer <= 0)
        {
            this->SetTeleporting(false);
        } else
        {
            this->TeleportTimer -= DeltaTime;
        }
    }
    if (IsCharacterMoving && GetCharacter()->GetCharacterMovement()->MovementMode == MOVE_Walking)
    {
        if (autoTravelDelayTimer > 0)
        {
            autoTravelDelayTimer -= DeltaTime;
        } else
        {
            TravelBackToCharacter(DeltaTime);
        }
    }
}

inline void UAC_CameraController::SetIsCharacterMoving(bool IsMoving, float DeltaAngle)
{
    this->IsCharacterMoving = IsMoving;
    if (DeltaAngle > 2.9f)
    {
        autoTravelDelayTimer = AutoTravelDelay;
    }
}

void UAC_CameraController::StartLooking()
{
    if (IsValid(GetCharacter()))
    {
        if (IsValid(GetCameraBoom()))
        {
            this->SetFPCamera();
        }
    }
}

void UAC_CameraController::StopLooking()
{
    if (IsValid(GetCharacter()))
    {
        if (IsValid(GetCameraBoom()))
        {
            this->ResetCamera();
        }
    }
}

void UAC_CameraController::ResetCamera()
{
    if (IsValid(GetCharacter()))
    {
        if (IsValid(GetCameraBoom()))
        {
            GetCharacter()->GetMesh()->SetOwnerNoSee(false);
            GetCameraBoom()->SetRelativeTransform(FTransform(DefaultCameraAngle.Rotation(), DefaultCameraLocation));
            GetCameraBoom()->TargetArmLength = DefaultCameraDistance;
            this->CurrentMaxYawAngle = DefaultMaxYawAngle;
            APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
            if (IsValid(CameraManager))
            {
                CameraManager->ViewPitchMin = this->DefaultMinPitchAngle;
                CameraManager->ViewPitchMax = this->DefaultMaxPitchAngle;
            }
            this->LookAhead();
        }
    }
}

void UAC_CameraController::SetFPCamera()
{
    if (IsValid(GetCharacter()))
    {
        if (IsValid(GetCameraBoom()))
        {
            GetCharacter()->GetMesh()->SetOwnerNoSee(true);
            GetCameraBoom()->SetRelativeTransform(FTransform(FPCameraAngle.Rotation(), FPCameraLocation));
            GetCameraBoom()->TargetArmLength = FPCameraDistance;
            this->CurrentMaxYawAngle = FPMaxYawAngle;
            APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
            if (IsValid(CameraManager))
            {
                CameraManager->ViewPitchMin = this->FPMinPitchAngle;
                CameraManager->ViewPitchMax = this->FPMaxPitchAngle;
            }
            this->LookAhead();
        }
    }
}

void UAC_CameraController::TravelBackToCharacter(float DeltaTime)
{
    if (IsValid(GetCharacter()) && GetCharacter()->GetIsLooking())
    {
        return;    
    }
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (IsValid(PlayerController))
    {
        float CharacterYaw = GetCharacter()->GetActorRotation().Yaw + 180.f;
        float CameraYaw = PlayerController->GetControlRotation().Yaw;
        float DeltaYaw = UKismetMathLibrary::Abs(CharacterYaw - CameraYaw);

        if (DeltaYaw > 180.f + AutoTravelDeltaTrigger || DeltaYaw < 180.f - AutoTravelDeltaTrigger)
        {
            float TravellingSpeedRatio = UKismetMathLibrary::Abs(DeltaYaw - 180.f) / 180.f;
            float ControllerYawInput = UKismetMathLibrary::FClamp(CharacterYaw - CameraYaw, -1.f, 1.f) * AutoTravellingSpeed * TravellingSpeedRatio * DeltaTime;
            if (UKismetMathLibrary::Abs(CharacterYaw - CameraYaw) < 180.f)
            {
                ControllerYawInput = -ControllerYawInput;
            }
            GetCharacter()->AddControllerYawInput(ControllerYawInput);
        }

        // Causes camera shaking on landscapes

        /*
        float CharacterPitch = GetCharacter()->GetActorRotation().Pitch + 180.f;

        float CameraPitch = PlayerController->GetControlRotation().Pitch;
        float DeltaPitch = CharacterPitch - CameraPitch ;
        DeltaPitch > 0.f ? DeltaPitch -= 180.f : DeltaPitch += 180.f;
        float TargetPitch = - DefaultCameraAngle.Y;
        if (DeltaPitch - AutoTravelDeltaTrigger < TargetPitch || DeltaPitch + AutoTravelDeltaTrigger > TargetPitch)
        {
            float ControllerPitchInput = UKismetMathLibrary::FClamp(DeltaPitch - TargetPitch, -1.f, 1.f) * AutoTravellingSpeed/3.f * DeltaTime * -1.f;
            GetCharacter()->AddControllerPitchInput(ControllerPitchInput);
        }
        */
    }
}

void UAC_CameraController::LookAhead()
{
    if (IsValid(GetCharacter()))
    {
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (IsValid(PlayerController))
        {
            FRotator ControllerRotation = PlayerController->GetControlRotation();
            FRotator CharacterRotation = GetCharacter()->GetActorRotation();

            PlayerController->SetControlRotation(FRotator(ControllerRotation.Pitch, CharacterRotation.Yaw, ControllerRotation.Roll));
        }
    }
}

void UAC_CameraController::GetControllerInput(float inputX, float inputY)
{
    if (IsValid(GetCharacter()))
    {
        float AddedYaw = inputX * ManualTravellingSpeedRatio;
        float AddedPitch = inputY * ManualTravellingSpeedRatio;
        if (abs(AddedYaw) >= abs(AddedPitch))
        {
            APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (IsValid(PlayerController))
            {
                float ControllerYaw = PlayerController->GetControlRotation().Yaw;
                float CharacterYaw = GetCharacter()->GetActorRotation().Yaw + 180.f;
                float DeltaYaw = UKismetMathLibrary::NormalizedDeltaRotator(FRotator(0, ControllerYaw,0), FRotator(0, CharacterYaw, 0)).Yaw;

                if (!(AddedYaw > 0 && DeltaYaw < 0 && abs(DeltaYaw) < 180 - CurrentMaxYawAngle) && !(AddedYaw < 0 && DeltaYaw > 0 && abs(DeltaYaw) < 180 - CurrentMaxYawAngle))
                {
                    GetCharacter()->AddControllerYawInput(inputX * ManualTravellingSpeedRatio);
                }
            }
        } else
        {
            GetCharacter()->AddControllerPitchInput(inputY * ManualTravellingSpeedRatio);
        }
    }
}

void UAC_CameraController::DisableCameraLag()
{
    if (IsValid(GetCameraBoom()))
    {
        GetCameraBoom()->bEnableCameraLag = false;
    }
}

void UAC_CameraController::EnableCameraLag()
{
    if (IsValid(GetCameraBoom()))
    {
        GetCameraBoom()->bEnableCameraLag = true;
    }
}

void UAC_CameraController::Teleport()
{
    SetTeleporting(true);
}

void UAC_CameraController::SetTeleporting(bool isTeleporting)
{
    this->TeleportTimer = TeleportCooldown;
    this->IsTeleporting = isTeleporting;
    if (isTeleporting)
    {
        this->DisableCameraLag();
    } else
    {
        this->EnableCameraLag();
    }
}