// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "c47GameMode.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "c47Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAC_CameraController;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTravelStartDelegate);

UCLASS(config=Game)
class Ac47Character : public ACharacter
{
	GENERATED_BODY()
	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UAC_CameraController* CameraController;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FTravelStartDelegate TravelStart;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = playerStat, meta = (AllowPrivateAccess = "true"))
	int32 healthPoint = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = playerStat, meta = (AllowPrivateAccess = "true"))
	int32 MaxHealthPoint = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = playerStat, meta = (AllowPrivateAccess = "true"))
	int32 coinCounter = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = playerStat, meta = (AllowPrivateAccess = "true"))
	int32 starCounter = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = playerStat, meta = (AllowPrivateAccess = "true"))
	AActor* lastPlatformStepOn;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Center Camera Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CenterCameraAction;

	/** FP Camera Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FPCameraAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float WalkingSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float SprintingSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FRotator RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MaxAcceleration = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float JumpingMaxAcceleration = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float JumpHorizontalForce = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float JumpVerticalForce = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LedgeToleranceCD = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float JumpBufferCD = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool EnableInputLockToCamera = true;
	
	// Activate input tilt ratio on jumps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool IsJumpInputTiltRatioActive = false;

	
	UPROPERTY()
	float scallarParam = -1;

	UPROPERTY(EditAnywhere, Category = Sound)
	TArray<USoundWave*> CatHurtSounds;

protected:
	float ledgeToleranceTimer = 0.f;
	float jumpBufferTimer = 0.f;
	float jumpHoldTimer = 0.f;
	bool canJump = false;
	bool IsSprinting = false;
	bool IsLooking = false;
	bool IsFPMode = false;
	bool IsJumping = false;
	bool IsTryingToJump = false;

	void OnStopLooking();
	void OnStopMoving();
	FVector LastInputValue;
	FVector ForwardDirection;
	FVector RightDirection;

public:
	Ac47Character();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsInDreamWorld = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DreamBoolValueHasChanged = true;

	UPROPERTY()
	UMaterialInstanceDynamic* matInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = playerStat)
	UMaterialInterface* DeathAndRespawnMaterial;

	int GetMaxHealth(){return MaxHealthPoint;}
	void SetHealth(int healthToSet){healthPoint = healthToSet;}

	UFUNCTION(BlueprintCallable)
	int getStar(){return starCounter;}
	
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UPROPERTY()
	FOnPlayerDiedSignature OnPlayerDied;

	FTimerHandle MemberTimerHandle;

	//void Jump() override;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();
	void DisolvePlayer();
	
	void CallPlayerDied();
	
	void Tick(float DeltaSeconds) override;

	UFUNCTION()
	virtual void OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
							FVector NormalImpulse, const FHitResult& Hit);


public:
	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = Movement)
	bool GetIsSprinting() { return IsSprinting; }
	UFUNCTION(BlueprintCallable, Category = Movement)
	void SetIsSprinting(bool Sprint) { IsSprinting = Sprint; }

	UFUNCTION(BlueprintCallable, Category = Camera)
	bool GetIsLooking() { return IsLooking; }
	UFUNCTION(BlueprintCallable, Category = Camera)
	void SetIsLooking(bool Look) { IsLooking = Look; }

	/** Called for sprint input */
	void StartSprinting(const FInputActionValue& Value);
	void StopSprinting(const FInputActionValue& Value);

	/** Called for camera centering input */
	void CenterCamera(const FInputActionValue& Value);

	/** Called for FP camera input */
	void TriggerFPCamera(const FInputActionValue& Value);

	//void Jump() override;
	void Jump() override;
	void StopJumping() override;
	void ApplyJumpForce(float DeltaTime);
	//bool CanJumpInternal_Implementation() override;
	void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	FVector GetLastInputValue() { return LastInputValue; };
	void Landed(const FHitResult& Hit) override;
	
	UFUNCTION(BlueprintCallable, Category = Movement)
	void OnTravelStart();
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns CameraController subobject **/
	FORCEINLINE class UAC_CameraController* GetCameraController() const { return CameraController; }

	UFUNCTION(BlueprintCallable)
	void takeDamage(int damagePoint, bool isTeleport);

	UFUNCTION()
	void killPlayer();
};

