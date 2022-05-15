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


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WALLRUN_API UWallRunComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWallRunComponent();

	UPROPERTY(BlueprintReadOnly, Category = "WallRun")
	bool bOnWall;

	// how fast player moves on wall
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float OnWallAirControl;

	// how much gravity applied to player on wall
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float OnWallGravity;

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

	// how much of velocity impulse is added to initial impulse along the wall
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float MovementImpulseAdjust;

	// least allowed deviation of movement from wall 0-1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float AllowedDeviationFromWall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	USoundBase* WallRunSound;



protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UAudioComponent* AudioRunComp;

	// reference to player's character
	ACharacter* CompOwner;

	// reference to player's CharacterMovementComponent
	UCharacterMovementComponent* MoveComp;

	FTimerHandle TimerHandle_WallRun;

	FTimerHandle TimerHandle_CoyoteTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WallJump")
	float CoyoteTime;

	UFUNCTION()
	void CoyoteTime_Elapsed();

	FVector WallDirection;

	float DefaultAirControl;

	float DefaultGravity;

	UPROPERTY(BlueprintReadOnly, Category = "WallJump")
	FVector WallJumpVelocity;

	// how long to stick to wall
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float WallRunDuration;

	UPROPERTY(BlueprintReadOnly, Category = "WallRun")
	FVector WallNormal;

	// if wall on the right side of character then 1.0f or if on the left side then -1.0f
	UPROPERTY(BlueprintReadOnly, Category = "WallRun")
	float WallSide;

	// implement wallrunning state
	UFUNCTION()
	void StickToWall();

	// stop wallrunning state
	UFUNCTION()
	void OffWall();

	bool bOnFloor;

	bool bClimbingLedge;

public:	

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "WallRun")
	void OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "WallRun")
	bool IsCharacterMovingBackwards();

	// event for blueprint to add actions when stick on wall
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
