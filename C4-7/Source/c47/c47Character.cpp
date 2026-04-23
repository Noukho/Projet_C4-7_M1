// Copyright Epic Games, Inc. All Rights Reserved.

#include "c47Character.h"

#include "c47GameMode.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
//#include "ToolBuilderUtil.h"
#include "Kismet/GameplayStatics.h"
#include "AC_CameraController.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "PacketHandlers/StatelessConnectHandlerComponent.h"
#include "Tasks/Task.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// Ac47Character

Ac47Character::Ac47Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = RotationRate; // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 550.f;
	GetCharacterMovement()->AirControl = 1.f;
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
	GetCharacterMovement()->MaxAcceleration = MaxAcceleration;

	GetCharacterMovement()->GroundFriction = 15.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->SetWalkableFloorAngle(60.f);
	GetCharacterMovement()->BrakingDecelerationWalking = 3500.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 500.f;
	GetCharacterMovement()->FallingLateralFriction = 100.f;
	GetCharacterMovement()->GravityScale = 3.f;

	this->JumpMaxHoldTime = 0.25f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 200.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	CameraController = CreateDefaultSubobject<UAC_CameraController>(TEXT("CameraController"));
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &Ac47Character::OnActorHit);
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void Ac47Character::BeginDestroy()
{
	//CameraController->RemoveFromRoot();
	Super::BeginDestroy();
}

void Ac47Character::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(GetCameraController()))
	{
		TravelStart.AddUniqueDynamic(this->CameraController, &UAC_CameraController::Teleport);
	}
}

void Ac47Character::DisolvePlayer()
{
	scallarParam += 0.02;
	matInstance->SetScalarParameterValue("Offset_Dissolve", scallarParam);
	if(scallarParam >= 1)
	{
		scallarParam = -1;
		CallPlayerDied();
		GetWorldTimerManager().ClearTimer(MemberTimerHandle);
	}
}

void Ac47Character::CallPlayerDied()
{
	Ac47GameMode* GameMode = Cast<Ac47GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->GetOnPlayerDied().Broadcast(this);
	}
}

void Ac47Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (GetCharacterMovement()->MovementMode != MOVE_Falling)
	{
		canJump = true;
	}
	else if (ledgeToleranceTimer > 0)
	{
		ledgeToleranceTimer -= DeltaSeconds;
		if (ledgeToleranceTimer <= 0)
		{
			canJump = false;
			ledgeToleranceTimer = 0;
		}
	}
	if (IsJumping)
	{
		if (jumpHoldTimer > 0)
		{
			ApplyJumpForce(DeltaSeconds);
			//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, FString::Printf(TEXT("add force")));
			jumpHoldTimer -= DeltaSeconds;
			if (jumpHoldTimer <= 0)
			{
				StopJumping();
			}
		}
	}
	if (jumpBufferTimer > 0)
	{
		jumpBufferTimer -= DeltaSeconds;
		if (jumpBufferTimer <= 0)
		{
			IsTryingToJump = false;
			jumpBufferTimer = 0;
		}
	}
}

void Ac47Character::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                               FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor != nullptr)
	{
		lastPlatformStepOn = OtherActor;
	}
}

void Ac47Character::FellOutOfWorld(const class UDamageType& dmgType)
{
	this->takeDamage(1, true);
}

void Ac47Character::takeDamage(int damagePoint, bool isTeleport)
{
	/*IsInDreamWorld = true;
	DreamBoolValueHasChanged = true;*/
	healthPoint = healthPoint - abs(damagePoint);
	int randomIndex = FMath::RandRange(0, 1);
	UGameplayStatics::PlaySound2D(this, CatHurtSounds[randomIndex]);
	if(healthPoint <= 0)
	{
		killPlayer();
	}
	else if (isTeleport)
	{
		Ac47GameMode* gMode = Cast<Ac47GameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if(lastPlatformStepOn != nullptr && !lastPlatformStepOn->Tags.IsEmpty() && lastPlatformStepOn->Tags[0] == "platform")
		{
			FVector playerLoc = lastPlatformStepOn->GetActorLocation();
			playerLoc.Z = playerLoc.Z + lastPlatformStepOn->GetComponentsBoundingBox().Max.Z ;
			TeleportTo(playerLoc, lastPlatformStepOn->GetActorRotation());	
		}
		else if(gMode->isCheckpointIsReady() == true)
		{
			gMode->RespawnPlayerAtCheckpoint();
		}
		else
		{
			AActor* playerStart = UGameplayStatics::GetGameMode(GetWorld())->FindPlayerStart(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			TeleportTo(playerStart->GetActorLocation(),playerStart->GetActorRotation());
		}
	}
}

void Ac47Character::killPlayer()
{
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->bBlockInput = true;
	const UE::Math::TVector<double> velocityZero(0,0,0) ;
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetCharacterMovement()->Velocity = velocityZero;
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetCharacterMovement()->SetMovementMode(MOVE_Flying) ;
	
	UMaterialInterface* baseMaterial = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetMesh()->GetMaterial(0);
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetMesh()->SetMaterial(1, baseMaterial);
	
	matInstance = UMaterialInstanceDynamic::Create(DeathAndRespawnMaterial, this);;
	matInstance->SetScalarParameterValue("Offset_Dissolve", scallarParam);
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetMesh()->SetMaterial(0, matInstance );
	DisableInput(UGameplayStatics::GetPlayerController(GetWorld(),0));
	GetWorldTimerManager().SetTimer(MemberTimerHandle, this, &Ac47Character::DisolvePlayer, 0.02f, true, 0);
}

//////////////////////////////////////////////////////////////////////////
// Input

void Ac47Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
	
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &Ac47Character::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &Ac47Character::OnStopMoving);

		// Sprint
		EnhancedInputComponent->BindAction(SprintAction,  ETriggerEvent::Started, this, &Ac47Character::StartSprinting);
		EnhancedInputComponent->BindAction(SprintAction,  ETriggerEvent::Completed, this, &Ac47Character::StopSprinting);

		// Looking 
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &Ac47Character::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &Ac47Character::OnStopLooking);
		
		// Centering camera 
		EnhancedInputComponent->BindAction(CenterCameraAction, ETriggerEvent::Started, this, &Ac47Character::CenterCamera);

		// Triggering FP camera 
		EnhancedInputComponent->BindAction(FPCameraAction, ETriggerEvent::Started, this, &Ac47Character::TriggerFPCamera);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void Ac47Character::OnTravelStart()
{
	TravelStart.Broadcast();
}

void Ac47Character::Move(const FInputActionValue& Value)
{
	if (this->IsFPMode)
	{
		return;
	}
	// input is a Vector2D
	FVector MovementVector = Value.Get<FVector>();
	float DeltaAngle = acosf(FVector::DotProduct(MovementVector.GetSafeNormal(), LastInputValue.GetSafeNormal())) * (180 / 3.1415926);
	//GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow, FString::Printf(TEXT("LastInput Y: %f"), MovementVector.Y));

	if (Controller != nullptr)
	{
		if (!EnableInputLockToCamera || LastInputValue.Length() == 0 || DeltaAngle > 2.9f)
		{
			LastInputValue = MovementVector;
			// get forward vector
			ForwardDirection = FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::X);
			
			// get right vector 
			RightDirection = FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::Y);
		}
	}
	float MovementSpeed = IsSprinting ? SprintingSpeed : WalkingSpeed;
	// adapt max speed to analog input value
	this->GetCharacterMovement()->MaxWalkSpeed = MovementVector.Size() / 50 * MovementSpeed;

	// add movement 
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);

	if (EnableInputLockToCamera && IsValid(GetCameraController()) && this->GetCharacterMovement()->MovementMode == MOVE_Walking)
	{
		GetCameraController()->SetIsCharacterMoving(true, DeltaAngle);
	}
}

void Ac47Character::OnStopMoving()
{
	LastInputValue = FVector().Zero();
	if (IsValid(GetCameraController()))
	{
		GetCameraController()->SetIsCharacterMoving(false, 0.f);
	}
}


void Ac47Character::StartSprinting(const FInputActionValue& Value)
{
	if (this->IsFPMode)
	{
		return;
	}
	this->IsSprinting = true;
}

void Ac47Character::StopSprinting(const FInputActionValue& Value)
{
	this->IsSprinting = false;
}

void Ac47Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	this->IsLooking = true;

	if (Controller != nullptr)
	{
		if (!IsSprinting)
		{
			if (IsValid(GetCameraController()))
			{
				GetCameraController()->GetControllerInput(LookAxisVector.X, LookAxisVector.Y);
			}
		}
	}
}

void Ac47Character::OnStopLooking()
{
	this->IsLooking = false;
}

void Ac47Character::CenterCamera(const FInputActionValue& Value)
{
	if (IsValid(GetCameraController()))
	{
		GetCameraController()->LookAhead();
	}
}

void Ac47Character::TriggerFPCamera(const FInputActionValue& Value)
{
	if (IsValid(GetCameraController()))
	{
		if (IsFPMode)
		{
			this->IsFPMode = false;
			this->CameraController->StopLooking();
		}
		else
		{
			if (this->GetCharacterMovement()->MovementMode == MOVE_Walking)
			{
				this->IsFPMode = true;
				this->CameraController->StartLooking();
			}	
		}
	}
}

void Ac47Character::Jump()
{
	if (this->IsFPMode)
	{
		return;
	}
	if (canJump)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, FString::Printf(TEXT("Start jump")));
		//Super::Jump();
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		FVector ForwardVector = FVector::VectorPlaneProject(PlayerController->GetControlRotation().Vector(), this->GetActorUpVector()).GetSafeNormal();
		float InputAngle = UKismetMathLibrary::RadiansToDegrees(UKismetMathLibrary::Atan2(LastInputValue.X, LastInputValue.Y));
		FVector JumpDirection = ForwardVector.RotateAngleAxis(InputAngle, this->GetActorUpVector());
		//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, FString::Printf(TEXT("%f"), PlayerController->GetControlRotation().Yaw));
		
		float InputTiltRatio = LastInputValue.Length()/50;
		if (!IsJumpInputTiltRatioActive && InputTiltRatio > 0.1f)
		{
			InputTiltRatio = 0.5f;
		}
		FVector HorizontalForce = JumpDirection * JumpHorizontalForce * InputTiltRatio;
		FVector JumpForce = FVector(0, 0, JumpVerticalForce * 0.75) + HorizontalForce;
		this->GetCharacterMovement()->MaxAcceleration = JumpingMaxAcceleration;
		this->GetCharacterMovement()->Velocity = FVector::Zero();
		this->GetCharacterMovement()->AddImpulse(JumpForce.GetClampedToMaxSize(JumpingMaxAcceleration) * 0.75, true);
		IsJumping = true;
		jumpHoldTimer = JumpMaxHoldTime;
	} else
	{
		IsTryingToJump = true;
		jumpBufferTimer = JumpBufferCD;
	}
}

void Ac47Character::StopJumping()
{
	//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("STOP jump")));
	Super::StopJumping();
	GetCharacterMovement()->MaxAcceleration = MaxAcceleration;
	canJump = false;
	ledgeToleranceTimer = 0;
	jumpHoldTimer = 0;
	GetCharacterMovement()->RotationRate = RotationRate;
}

void Ac47Character::ApplyJumpForce(float DeltaTime)
{
	float InputTiltRatio = LastInputValue.Length()/50;
	if (!IsJumpInputTiltRatioActive && InputTiltRatio > 0.1f)
	{
		InputTiltRatio = 1.f;
	}
	FVector HorizontalForce = this->GetActorForwardVector() * JumpHorizontalForce * InputTiltRatio;
	FVector JumpForce = FVector(0, 0, JumpVerticalForce * 1.4f * (HorizontalForce.Length() / JumpHorizontalForce / 5.f + 1)) + HorizontalForce;
	this->GetCharacterMovement()->AddImpulse(JumpForce * 3 * DeltaTime, true);
}

void Ac47Character::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	ledgeToleranceTimer = LedgeToleranceCD;
}

void Ac47Character::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (IsJumping)
	{
		this->GetCharacterMovement()->Velocity *= 0.2f;
		IsJumping = false;
	}

	if (IsTryingToJump)
	{
		IsTryingToJump = false;
		canJump = true;
		jumpBufferTimer = 0.f;
		Jump();
	}
}
