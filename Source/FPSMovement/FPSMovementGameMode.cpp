// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSMovementGameMode.h"
#include "FPSMovementCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFPSMovementGameMode::AFPSMovementGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
