// titanfall-like mechanics for wall traversal
// made by Ivan Feklistov i.a.feklistov@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallRunComponent.generated.h"

class UCharacterMovementComponent;
class UAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWallEventDelegate, float, WallSide);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOffWallEventDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FClimbEventDelegate, FVector, ImpactPoint);

// logging to screen during PIE
// e.g. Message = FString::Printf(TEXT("x: %f"), f)
#define PrintToScreen(Duration, Message) GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::White, Message) 


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WALLRUN_API UWallRunComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWallRunComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WallRun")
	TArray<TEnumAsByte<ECollisionChannel>> ObjectTypesForWallRun;

	UPROPERTY(BlueprintReadOnly, Category = "WallRun")
	bool bOnWall;

	// how fast player moves on wall
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float OnWallAirControl;

	// how much gravity applied to player on wall
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float OnWallGravity;

	// how much to launch player up the wall when start wall run
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float LaunchOnStickUp;

	// how much to launch player to side in direction the wall when start wall run
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float LaunchOnStickSide;

	// how much of velocity impulse is added to initial impulse along the wall
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float MovementumAdjust;

	UFUNCTION(Category = "WallJump")
	void WallJump();

	UPROPERTY(BlueprintReadOnly, Category = "WallJump")
	bool bCanJumpFromWall;

	// how much go off the wall when wall jump
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallJump")
	float LaunchStrengthNormal;

	// how much velocity added along the movement direction when wall jump
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallJump")
	float LaunchStrengthLook;

	// how much go up when wall jump
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallJump")
	float LaunchStrengthZ;

	// maximum wall jump velocity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallJump")
	float MaxWallJumpVelocity;

	// how much velocity added to climb ledge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float ClimbStrength;

	// least allowed deviation of movement from wall 0-1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float AllowedDeviationFromWall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	USoundBase* WallRunSound;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	UAudioComponent* AudioRunComp;

	// reference to player's character
	UPROPERTY()
	ACharacter* CompOwner;

	// reference to player's CharacterMovementComponent
	UPROPERTY()
	UCharacterMovementComponent* MoveComp;

	UPROPERTY()
	FTimerHandle TimerHandle_WallRun;

	UPROPERTY()
	FTimerHandle TimerHandle_CoyoteTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WallJump")
	float CoyoteTime;

	UFUNCTION()
	void CoyoteTime_Elapsed();

	UPROPERTY()
	float DefaultAirControl;

	UPROPERTY()
	float DefaultGravity;

	// how long to stick to wall
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float WallRunDuration;

	UPROPERTY(BlueprintReadOnly, Category = "WallRun")
	FVector WallNormal;

	UPROPERTY(BlueprintReadOnly, Category = "WallRun")
	FVector WallDirection;

	UPROPERTY()
	float LastWallSide = 0.f;

	// implement wallrunning state
	UFUNCTION()
	void StickToWall();



	UPROPERTY()
	bool bOnFloor;

	UPROPERTY()
	bool bClimbingLedge;

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	bool DebugLog;

	// stop wallrunning state
	UFUNCTION(BlueprintCallable, Category = "WallRun")
	void OffWall();

	UFUNCTION(BlueprintNativeEvent, Category = "WallRun")
	void OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	// calculate whether player moves backwards, based on his look direction and velocity
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WallRun")
	bool IsCharacterMovingBackwards();

	// calculate whether player is looking at the wall he sticked to using Threashold (0-1) for what is considered "looking at wall'
	// where 1 - is perpendicular to the wall, and 0 - is parallel to the wall
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WallRun")
	bool IsCharacterLookingAtWall(float Threshold = 0.75f);

	// calculate whether wall on the right side (1) or on the left side (-1) of character
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WallRun")
	float CalculateWallSide();

	// event for blueprint to add actions when stick on wall
	// 1 - wall on the right side of player
	// -1 - wall on the left side of player
	// 0 - player not on the wall
	UPROPERTY(BlueprintAssignable, Category = "WallRun")
	FOnWallEventDelegate OnWallEvent;

	// event for blueprint to add actions when unstick from wall
	UPROPERTY(BlueprintAssignable, Category = "WallRun")
	FOffWallEventDelegate OffWallEvent;

	// event for blueprint to execute climbing on ledge
	UPROPERTY(BlueprintAssignable, Category = "WallRun")
	FClimbEventDelegate ClimbEvent;


	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
