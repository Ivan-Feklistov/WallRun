// Fill out your copyright notice in the Description page of Project Settings.


#include "WallRunComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UWallRunComponent::UWallRunComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bOnWall = false;
	bOnFloor = true;
	bCanJumpFromWall = false;
	bClimbingLedge = false;
	WallRunDuration = 3.0f;
	LaunchStrengthNormal = 150.f;
	LaunchStrengthLook = 400.f;
	LaunchStrengthZ = 550.f;
	MaxWallJumpVelocity = 1200.f;
	CoyoteTime = 0.2f;
	OnWallGravity = 0.4f;
	OnWallAirControl = 0.5f;
	LaunchOnStickUp = 400.f;
	LaunchOnStickSide = 500.f;
	MovementumAdjust = 0.1f;
	ClimbStrength = 100.f;
	AllowedDeviationFromWall = 0.35f;
	DebugLog = false;
	
	AudioRunComp = CreateDefaultSubobject<UAudioComponent>("AudioComp");
	
}


// Called when the game starts
void UWallRunComponent::BeginPlay()
{
	Super::BeginPlay();
	// set sound indication duration of wallrun
	if (WallRunSound)
	{
		AudioRunComp->SetSound(WallRunSound);
	}

	CompOwner = Cast<ACharacter>(GetOwner());
	if (CompOwner)
	{
		MoveComp = CompOwner->GetCharacterMovement();
		if (MoveComp)
		{
			CompOwner->OnActorHit.AddDynamic(this, &UWallRunComponent::OnHit);
			DefaultGravity = MoveComp->GravityScale;
			DefaultAirControl = MoveComp->AirControl;
			WallDirection = FVector::ZeroVector;
		}
	}	
}


void UWallRunComponent::OnHit_Implementation(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	float Verticality = FMath::RoundHalfFromZero(Hit.Normal.Z); // 0 for wall, 1 for floor

	// check if player collided with wall
	if (ObjectTypesForWallRun.Contains(Hit.Component->GetCollisionObjectType()) && MoveComp->IsFalling() && Verticality == 0.f && bOnWall == false)
	{
		if (DebugLog)
			UE_LOG(LogTemp, Log, TEXT("Wall Hit"));
		// detect if it's the same wall to not stick to it
		FVector NewWallDirection = FVector::CrossProduct(FVector::UpVector, Hit.Normal);
		if (NewWallDirection != WallDirection)
		{
			WallNormal = Hit.Normal;
			StickToWall();
		}		
	}

	// check if ledge in front and climb it
	if (ObjectTypesForWallRun.Contains(Hit.Component->GetCollisionObjectType()) && MoveComp->IsFalling() && Verticality == 0.f && bOnWall == true && bClimbingLedge == false)
	{
		// @todo change way to detect ledge to L-like trace
		FHitResult LedgeHit;
		FVector Start = CompOwner->GetActorLocation() + FVector(0.f, 0.f, 50.f);
		FVector End = Start + (-WallNormal) * 100.f;
		GetWorld()->LineTraceSingleByChannel(LedgeHit, Start, End, ECC_Visibility);
		if (LedgeHit.bBlockingHit == false && IsCharacterLookingAtWall())
		{
			if (DebugLog)
				UE_LOG(LogTemp, Log, TEXT("Climb ledge"));
			bClimbingLedge = true;
			OffWall();
			ClimbEvent.Broadcast(Hit.ImpactPoint);
		}
	}

	//check if player collided with floor
	if (ObjectTypesForWallRun.Contains(Hit.Component->GetCollisionObjectType()) && Verticality == 1.f && bOnFloor == false || !MoveComp->IsFalling())
	{
		if (DebugLog)
			UE_LOG(LogTemp, Log, TEXT("Floor Hit"));
		bOnFloor = true;
		bClimbingLedge = false;
		if (bOnWall)
		{
			OffWall();
		}	
		WallDirection = FVector::ZeroVector;
	}
}



void UWallRunComponent::StickToWall()
{
	if (DebugLog)
		UE_LOG(LogTemp, Log, TEXT("Stick to wall"));
	bOnWall = true;
	bCanJumpFromWall = true;
	bOnFloor = false;
	bClimbingLedge = false;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_WallRun);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CoyoteTime);

	// timer to stop wallrunning after set time (to not infinitely run on one wall)
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_WallRun, this, &UWallRunComponent::OffWall, WallRunDuration, false);
	MoveComp->GravityScale = OnWallGravity;
	MoveComp->AirControl = OnWallAirControl;

	// calculate wall direction
	WallDirection = FVector::CrossProduct(FVector::UpVector, WallNormal);

	// calculate whether wall on the right side (1) or on the left side (-1) of character
	const float WallSide = CalculateWallSide();
	//LastWallSide = WallSide;
	
	const FVector WallDirectionSide = WallDirection * WallSide; // make WallDirection point to the same direction as player

	// calculate how much (strong) player is trying to move into the wall 
	const FVector DirectionalStrength = MoveComp->GetLastInputVector();
	float StrengthOfSideLaunch = FVector::DotProduct(WallDirection, DirectionalStrength);
	StrengthOfSideLaunch = FMath::Abs(StrengthOfSideLaunch);

	// dont launch player along the wall if he touches wall moving backwards
	if (IsCharacterMovingBackwards())
	{
		StrengthOfSideLaunch = StrengthOfSideLaunch * 0.2;
	}

	// give movement impulse to player along the wall in the direction of velocity and slightly up
	FVector Velocity = MoveComp->Velocity;
	float MovementImpulse = Velocity.Size();
	Velocity.Normalize();
	Velocity.Z = 0.f;
	// LaunchVelocity = (strength up) + (Strength in direction of look along the wall) + (Strength of momentum) + (impulse into wall)
	FVector LaunchVelocity = (FVector::UpVector * LaunchOnStickUp) + (WallDirectionSide * LaunchOnStickSide * StrengthOfSideLaunch) + 
								(Velocity * MovementImpulse * MovementumAdjust) + (-WallNormal * 100.f);
	CompOwner->LaunchCharacter(LaunchVelocity, true, true);
	
	// play sound of wallrunning
	if (WallRunSound && AudioRunComp != nullptr)
	{
		AudioRunComp->Play();
	}	
	// in blueprint: if no mouse input make camera look forward along the wall
	OnWallEvent.Broadcast(WallNormal);
	
}



void UWallRunComponent::OffWall()
{
	if (DebugLog)
		UE_LOG(LogTemp, Log, TEXT("Unstick form wall"));
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CoyoteTime);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_WallRun);
	bOnWall = false;	
	MoveComp->GravityScale = DefaultGravity;
	MoveComp->AirControl = DefaultAirControl;
	LastWallSide = 0.f;
	OffWallEvent.Broadcast();

	// coyote time for jump from wall
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_CoyoteTime, this, &UWallRunComponent::CoyoteTime_Elapsed, CoyoteTime, false);
	if (WallRunSound && AudioRunComp != nullptr)
	{
		AudioRunComp->Stop();
	}
}

void UWallRunComponent::CoyoteTime_Elapsed()
{
	bCanJumpFromWall = false;
}

void UWallRunComponent::WallJump()
{
	if (bCanJumpFromWall)
	{
		if (DebugLog)
			UE_LOG(LogTemp, Log, TEXT("Wall jump"));
		bCanJumpFromWall = false;
		OffWall();
		FVector Velocity = MoveComp->Velocity;
		float MovementImpulse = Velocity.Size();
		Velocity.Normalize();
		Velocity.Z = 0.f;
		// WallJumpVelocity = (strength away from wall) + (strength in direction of player input movement) + (strength up) + (momentum)
		FVector WallJumpVelocity = (WallNormal * LaunchStrengthNormal) + (MoveComp->GetLastInputVector() * LaunchStrengthLook) +
 							(FVector::UpVector * LaunchStrengthZ) + (Velocity * MovementImpulse);
		WallJumpVelocity = UKismetMathLibrary::ClampVectorSize(WallJumpVelocity, 0.f, MaxWallJumpVelocity);
		CompOwner->LaunchCharacter(WallJumpVelocity, true, true);
	}
}


bool UWallRunComponent::IsCharacterMovingBackwards()
{
	if (!CompOwner || !MoveComp)
	{
		return false;
	}
	// calculate how relative actor look direction to direction of movement 
	FVector Velocity = MoveComp->Velocity;
	Velocity.Normalize();
	// if negative then player looks in the opposite direction from movement, meaning - moves backwards
	float MovementDirection = FVector::DotProduct(CompOwner->GetActorForwardVector(), Velocity);
	return MovementDirection < 0.0f ? true : false;
}

bool UWallRunComponent::IsCharacterLookingAtWall(float Threshold)
{
	if (CompOwner && bOnWall)
	{
		float LookAtWall = FVector::DotProduct(-WallNormal, CompOwner->GetActorForwardVector());
		if (LookAtWall > Threshold)
		{
			return true;
		}
		return false;
	}
	return false;
}

float UWallRunComponent::CalculateWallSide()
{
	if (!CompOwner || !MoveComp || WallDirection.IsZero() || !bOnWall)
	{
		return 0.f;
	}

	FVector CharVelocity = MoveComp->Velocity;
	CharVelocity.Normalize();
	float WallSide = FVector::DotProduct(WallDirection, CharVelocity);
	if (WallSide < 0.f)
	{
		return -1.f;
	}
	else
	{
		return 1.f;
	}
}

// Called every frame
void UWallRunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bOnWall)
	{
		// stop wallrunning if player moves away from wall 
		FVector CharVelocity = MoveComp->Velocity;
		CharVelocity.Normalize();
		float DeviationFromWall = FVector::DotProduct(WallNormal, CharVelocity);
		
		if (DeviationFromWall > AllowedDeviationFromWall)
		{
			OffWall();
			if (DebugLog)
				UE_LOG(LogTemp, Log, TEXT("Moved away from wall"));
		}

		// stop wallrunning if wall ends (detect edge of wall)
		FHitResult Hit;
		FVector Start = CompOwner->GetActorLocation();
		FVector End = Start + (-WallNormal) * 100;
		GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility);
		if (!Hit.bBlockingHit)
		{
			OffWall();
		}
;	}
}

