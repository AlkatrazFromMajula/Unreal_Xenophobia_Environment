// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment_UpperEdge.h"

// Sets default values
AEnvironment_UpperEdge::AEnvironment_UpperEdge()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CharacterIsNearby = false;
	JumpIsPossible = false;
	MaxJumpAngle = 45;
}

// Called when the game starts or when spawned
void AEnvironment_UpperEdge::BeginPlay()
{
	Super::BeginPlay();

	// get references to components
	LeftFrameComponent = Cast<UStaticMeshComponent>(GetComponentsByTag(UStaticMeshComponent::StaticClass(), FName("LeftFrame"))[0]);
	RightFrameComponent = Cast<UStaticMeshComponent>(GetComponentsByTag(UStaticMeshComponent::StaticClass(), FName("RightFrame"))[0]);
	JumpTargetComponent = Cast<UStaticMeshComponent>(GetComponentsByTag(UStaticMeshComponent::StaticClass(), FName("JumpTarget"))[0]);
	FloatingHolderComponent = Cast<UStaticMeshComponent>(GetComponentsByTag(UStaticMeshComponent::StaticClass(), FName("FloatingHolder"))[0]);
	ControllerDetector = Cast<UBoxComponent>(GetComponentsByTag(UBoxComponent::StaticClass(), FName("ControllerDetector"))[0]);

	// get location of both bounds and target
	LeftFrame = LeftFrameComponent->GetComponentLocation();
	RightFrame = RightFrameComponent->GetComponentLocation();
	JumpTarget = JumpTargetComponent->GetComponentLocation();
	FloatingHolder = FloatingHolderComponent->GetComponentLocation();

	// set collider to proper extent
	double EdgeWidth = (RightFrame - LeftFrame).Length();
	ControllerDetector->SetBoxExtent(FVector(2, EdgeWidth / 2, 2));

	// declare delegates for overlap events
	ControllerDetector->OnComponentBeginOverlap.AddDynamic(this, &AEnvironment_UpperEdge::OverlapBegin);
	ControllerDetector->OnComponentEndOverlap.AddDynamic(this, &AEnvironment_UpperEdge::OverlapEnd);
}

// Called every frame
void AEnvironment_UpperEdge::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// set new target location
	if (CharacterIsNearby && Character != nullptr)
		FloatingHolderComponent->SetWorldLocation(CalculateNewTargetLocation());
}

// Called to find new target location
FVector AEnvironment_UpperEdge::CalculateNewTargetLocation()
{
	// get location of all components and actors used in calculation
	FloatingHolder = FloatingHolderComponent->GetComponentLocation();
	JumpTarget = JumpTargetComponent->GetComponentLocation();
	FVector2D RightFrame2D = FVector2D(RightFrame.X, RightFrame.Y);
	FVector2D LeftFrame2D = FVector2D(LeftFrame.X, LeftFrame.Y);
	FVector2D CharLoc = FVector2D(Character->GetMesh()->GetComponentLocation().X, Character->GetMesh()->GetComponentLocation().Y);

	// get forward and right vectors of character and edge
	FVector2D CharForward = FVector2D(Character->GetMesh()->GetForwardVector().X, Character->GetMesh()->GetForwardVector().Y);
	FVector2D EdgeForward = FVector2D(this->GetActorForwardVector().X, this->GetActorForwardVector().Y);
	FVector2D EdgeRight = FVector2D(this->GetActorRightVector().X, this->GetActorRightVector().Y);

	// get vectors from character to both frames
	FVector2D CharToRightFrame = RightFrame2D - CharLoc;
	FVector2D CharToLeftFrame = LeftFrame2D - CharLoc;


	// normalize vectors
	FVector2D CharToRightFrameNormalized = CharToRightFrame / CharToRightFrame.Length();
	FVector2D CharToLeftFrameNormalized = CharToLeftFrame / CharToLeftFrame.Length();

	// find angles between found vectors and edge forward vector
	double CharForwardAngle = FMath::Acos(FVector2D::DotProduct(CharForward, EdgeForward));
	double CharToRightFrameAngle = FMath::Acos(FVector2D::DotProduct(CharToRightFrameNormalized, EdgeForward));
	double CharToLeftFrameAngle = FMath::Acos(FVector2D::DotProduct(CharToLeftFrameNormalized, EdgeForward));

	// add sign to found angle
	CharForwardAngle *= FVector2D::DotProduct(CharForward, EdgeRight) >= 0 ? -1 : 1;
	CharToRightFrameAngle *= FVector2D::DotProduct(CharToRightFrameNormalized, EdgeRight) >= 0 ? -1 : 1;
	CharToLeftFrameAngle *= FVector2D::DotProduct(CharToLeftFrameNormalized, EdgeRight) >= 0 ? -1 : 1;

	// clamp new target location between right and left frames
	if ((((CharToRightFrameAngle * CharToLeftFrameAngle < 0) && (CharForwardAngle > CharToRightFrameAngle || CharForwardAngle < CharToLeftFrameAngle)) ||
		((CharToRightFrameAngle * CharToLeftFrameAngle > 0) && (CharForwardAngle > CharToRightFrameAngle && CharForwardAngle < CharToLeftFrameAngle)))
		&& FMath::Abs(CharToLeftFrameAngle) > PI / 2 && FMath::Abs(CharToRightFrameAngle) > PI / 2)
	{
		// decide whether jump is possible
		JumpIsPossible = FMath::Abs(CharForwardAngle) > 150.0 / 180 * PI;

		// calculate distance beteen left frame and new target location using law of sines
		double AngleBetweenFrames = FMath::Acos(FVector2D::DotProduct(CharToRightFrameNormalized, CharToLeftFrameNormalized));
		double SideB = CharToLeftFrame.Length();
		double Alpha = FMath::Sin(FMath::Acos(FVector2D::DotProduct(CharForward, CharToLeftFrameNormalized)) > AngleBetweenFrames ?
			AngleBetweenFrames : FMath::Acos(FVector2D::DotProduct(CharForward, CharToLeftFrameNormalized)));
		double Betha = FMath::Sin(FMath::Acos(FVector2D::DotProduct(CharForward, EdgeRight)));
		double SideA = SideB * Alpha / Betha;

		// return new target location
		return FMath::Lerp(LeftFrame, RightFrame, FMath::Clamp(SideA / (RightFrame - LeftFrame).Length(), 0, 1));
	}
	else
	{
		// block jump and set new target position at center
		JumpIsPossible = false;
		return FMath::Lerp(LeftFrame, RightFrame, 0.5);
	}
}

// Called to get target jump location
FVector AEnvironment_UpperEdge::GetJumpTargetLocation()
{
	return JumpIsPossible ? JumpTarget : FVector::ZeroVector;
}
void AEnvironment_UpperEdge::OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp->ComponentHasTag(FName("OuterDetectionSphere")))
	{
		Character = Cast<ACharacter>(OtherActor);
		CharacterIsNearby = true;
	}
}
void AEnvironment_UpperEdge::OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherComp->ComponentHasTag(FName("OuterDetectionSphere")))
	{
		Character = nullptr;
		CharacterIsNearby = false;
		JumpIsPossible = false;
	}
}