// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WallCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class WALLRUN_API UWallCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	virtual void Crouch(bool bClientSimulation = false) override;
	
	/**
	 * Checks if default capsule size fits (no encroachment), and trigger OnEndCrouch() on the owner if successful.
	 * @param	bClientSimulation	true when called when bIsCrouched is replicated to non owned clients, to update collision cylinder and offset.
	 */
	virtual void UnCrouch(bool bClientSimulation = false) override;
};
