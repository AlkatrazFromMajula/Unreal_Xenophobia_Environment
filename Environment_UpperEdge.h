// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Environment_UpperEdge.generated.h"

UCLASS()
class XENOPHOBIA_EXP_API AEnvironment_UpperEdge : public AActor
{
	GENERATED_BODY()

	UFUNCTION()
	void OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:	
	// Sets default values for this actor's properties
	AEnvironment_UpperEdge();

private:
	UStaticMeshComponent* LeftFrameComponent;
	UStaticMeshComponent* RightFrameComponent;
	UStaticMeshComponent* JumpTargetComponent;
	UStaticMeshComponent* FloatingHolderComponent;
	UBoxComponent* ControllerDetector;
	ACharacter* Character;

	FVector LeftFrame;
	FVector RightFrame;
	FVector FloatingHolder;
	FVector JumpTarget;

	bool CharacterIsNearby;
	bool JumpIsPossible;
	double MaxJumpAngle;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to get target jump location
	FVector GetJumpTargetLocation();

private:
	// Called to find new target location
	FVector CalculateNewTargetLocation();
};
