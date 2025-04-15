// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSMovementCharacter.h"
#include "FPSMovementProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AFPSMovementCharacter


AFPSMovementCharacter::AFPSMovementCharacter()
{
	
	DefaultMappingContext = nullptr;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	JumpCount = 0;
	MaxJumpCount = 2;
	
}

//////////////////////////////////////////////////////////////////////////// Input

void AFPSMovementCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AFPSMovementCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AFPSMovementCharacter::Jump);
		

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFPSMovementCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPSMovementCharacter::Look);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AFPSMovementCharacter::StartSprinting);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AFPSMovementCharacter::StopSprinting);

		// Sliding
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &AFPSMovementCharacter::StartSliding);
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Completed, this, &AFPSMovementCharacter::StopSliding);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AFPSMovementCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AFPSMovementCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFPSMovementCharacter::Jump()
{

	UE_LOG(LogTemplateCharacter, Log, TEXT("jump called"));
	if (JumpCount < MaxJumpCount)
	{
		if (JumpCount == 0 || GetCharacterMovement()->IsFalling())
		{
			ACharacter::LaunchCharacter(FVector(0.f, 0.f, 600.f), false, true);
			JumpCount++;
			UE_LOG(LogTemplateCharacter, Log, TEXT("JumpCount: %d"), JumpCount);
			
		}
	}
}

void AFPSMovementCharacter::Grapple()
{
	if (IsGrappling)
	{
		return;
	}

	FHitResult Hit;
	FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	FVector End = Start + GetFirstPersonCameraComponent()->GetForwardVector() * 1000.f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		GrappleTarget = Hit.Location
	}
}

void AFPSMovementCharacter::GrappleRelease()
{
}

void AFPSMovementCharacter::Landed(const FHitResult& Hit)
{
	UE_LOG(LogTemplateCharacter, Log, TEXT("landed"));
	Super::Landed(Hit);
	JumpCount = 0;
	
}

void AFPSMovementCharacter::StartSprinting()
{
	UE_LOG(LogTemplateCharacter, Log, TEXT("StartSprinting called"));
	GetCharacterMovement()-> MaxWalkSpeed = 1800.f;
	
}

void AFPSMovementCharacter::StopSprinting()
{
	UE_LOG(LogTemplateCharacter, Log, TEXT("StopSprinting called"));
	GetCharacterMovement()->MaxWalkSpeed = 1200.f;
	
}

void AFPSMovementCharacter::StartSliding()
{
	UE_LOG(LogTemplateCharacter, Log, TEXT("StartSliding called"));

	GetCharacterMovement()->MaxWalkSpeed = 800.f;
	GetCharacterMovement()->BrakingFrictionFactor = 0.f;
	
	GetCapsuleComponent()->SetCapsuleHalfHeight(48.0f);
	GetCapsuleComponent()->SetCapsuleRadius(30.0f);
}


void AFPSMovementCharacter::StopSliding()
{
	UE_LOG(LogTemplateCharacter, Log, TEXT("StopSliding called"));

	GetCharacterMovement()->MaxWalkSpeed = 1200.f;
	GetCharacterMovement()->BrakingFrictionFactor = 2.f;
	
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	GetCapsuleComponent()->SetCapsuleRadius(55.0f);
}
