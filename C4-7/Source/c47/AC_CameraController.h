// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/ActorComponent.h"
#include "AC_CameraController.generated.h"

class Ac47Character;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class C47_API UAC_CameraController : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = Camera)
    void LookAhead();
    UFUNCTION(BlueprintCallable, Category = Camera)
    void Teleport();
    UFUNCTION(BlueprintCallable, Category = Camera)
    void StartLooking();
    UFUNCTION(BlueprintCallable, Category = Camera)
    void StopLooking();
    UFUNCTION(BlueprintCallable, Category = Camera)
    void GetControllerInput(float InputX, float InputY);
    // Sets default values for this component's properties
    UAC_CameraController();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
    Ac47Character* Character;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float CameraLagValue = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float AutoTravelDelay = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AutoTravel, meta = (AllowPrivateAccess = "true"))
    float AutoTravelDeltaTrigger = 15.f;

    FVector DefaultCameraAngle = FVector(0, -7.f, 0);
    FVector DefaultCameraLocation = FVector(0, 0, 0);
    float DefaultCameraDistance = 200.0;

    FVector FPCameraAngle = FVector(0, 0, 0);
    FVector FPCameraLocation = FVector(20, 0, 0);
    float FPCameraDistance = 0.0;

    float AutoTravellingSpeed = 120.f;
    float ManualTravellingSpeedRatio = 1.5f;

    float DefaultMaxPitchAngle = 0.f;
    float DefaultMinPitchAngle = -60.0;
    float FPMaxPitchAngle = 75.0;
    float FPMinPitchAngle = -75.0;
    
    float CurrentMaxYawAngle = 180.0;
    float FPMaxYawAngle = 90.0;
    float DefaultMaxYawAngle = 180.0;

    float TeleportTimer = 0.0;
    float TeleportCooldown = 0.1f;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;
    void BeginDestroy() override;

    float autoTravelDelayTimer;
    bool IsTeleporting = false;
    bool IsCharacterMoving = false;

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void ResetCamera();
    void SetFPCamera();
    void TravelBackToCharacter(float DeltaTime);
    void DisableCameraLag();
    void EnableCameraLag();
    void SetTeleporting(bool IsTeleporting);
    void SetIsCharacterMoving(bool IsMoving, float DeltaAngle);

    FORCEINLINE class Ac47Character* GetCharacter() const { return Character; }
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};
