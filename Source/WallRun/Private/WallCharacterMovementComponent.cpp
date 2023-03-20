// Fill out your copyright notice in the Description page of Project Settings.


#include "WallCharacterMovementComponent.h"

//empty because crouch uncrouch sets character crouch state in tick... we set ourselves in AIGCharacter
void UWallCharacterMovementComponent::Crouch(bool bClientSimulation)
{
	//Super::Crouch(bClientSimulation);
}

//empty because crouch uncrouch sets character crouch state in tick... we set ourselves in AIGCharacter
void UWallCharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
	//Super::UnCrouch(bClientSimulation);
}