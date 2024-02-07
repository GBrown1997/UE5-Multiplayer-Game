// Fill out your copyright notice in the Description page of Project Settings.


#include "Transporter.h"

#include "PressurePlate.h"
#include "CollectableKey.h"

// Sets default values for this component's properties
UTransporter::UTransporter()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	//Turn on Replication for the actor component
	SetIsReplicatedByDefault(true);

	//Set MoveTime and ActivatedTriggerCount
	//MoveTime  = arbitrary number
	//Activated triggers are 0 at start of the game
	MoveTime = 3.0f;
	ActivatedTriggerCount = 0;

	ArePointsSet = false;
	StartPoint = FVector::Zero();
	EndPoint = FVector::Zero();
}

void UTransporter::SetPoints(FVector Point1, FVector Point2)
{
	if (Point1.Equals(Point2)) return;

	StartPoint = Point1;
	EndPoint = Point2;
	ArePointsSet = true;
}


// Called when the game starts
void UTransporter::BeginPlay()
{
	Super::BeginPlay();

	//if the actor that contains the transporter component is a TriggerActor (such as the Pressure Plate), then it is added to the list of trigger actors
	//this allows the actor to select itself as a TriggerActor
	if (OwnerIsTriggerActor)
	{
		TriggerActors.Add(GetOwner());
	}

	//Goes through all TriggerActors inside the TriggerActor array
	for (AActor* TA : TriggerActors)
	{
		//Casts to a pressure plate actor
		APressurePlate *PressurePlateActor = Cast<APressurePlate>(TA);

		//If casting works, binds the functions OnPressurePlateActivated and Deactivated to the pressure plates delegates
		if (PressurePlateActor) 
		{
			PressurePlateActor->OnActivated.AddDynamic(this, &UTransporter::OnTriggerActorActivated);
			PressurePlateActor->OnDeactivated.AddDynamic(this, &UTransporter::OnTriggerActorDeactivated);
			continue;
		}

		ACollectableKey* KeyActor = Cast<ACollectableKey>(TA);
		if (KeyActor)
		{
			KeyActor->OnCollected.AddDynamic(this, &UTransporter::OnTriggerActorActivated);
		}
	}
}

void UTransporter::OnTriggerActorActivated()
{
	ActivatedTriggerCount++;

	FString Msg = FString::Printf(TEXT("Transporter Activated: %d"), ActivatedTriggerCount);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, Msg);
}

void UTransporter::OnTriggerActorDeactivated()
{
	ActivatedTriggerCount--;
	FString Msg = FString::Printf(TEXT("Transporter Activated: %d"), ActivatedTriggerCount);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, Msg);
}


// Called every frame
void UTransporter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TriggerActors.Num() > 0)
	{
		//Shortened form of below commented if else statement
		AllTriggerActorsTriggered = (ActivatedTriggerCount >= TriggerActors.Num());

		if (AllTriggerActorsTriggered)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString("AllTriggerActorsTriggered!"));
		}
		

		/*
		if (ActivatedTriggerCount >= TriggerActors.Num())
		{
			AllTriggerActorsTriggered = true;
		}
		else
		{
			AllTriggerActorsTriggered = false;
		}
		*/
	}
	
	//Getting the owner and saving it in MyOwner variable
	AActor* MyOwner = GetOwner();
	//Checking if valid and has authority and points are set
	if (MyOwner && MyOwner->HasAuthority() && ArePointsSet)
	{
		//Gets current location of the owning actor, and saves it to CurrentLocation (local variable)
		FVector CurrentLocation = MyOwner->GetActorLocation();
		//Calculate Speed value using MoveTime and the distance between StartPoint and EndPoint
		float Speed = FVector::Distance(StartPoint, EndPoint) / MoveTime;

		//Ternary statement. If AllTriggerActorsTriggered = true, then the actor will move to the EndPoint. If it is false, then the actor moves back to the StartPoint
		FVector TargetLocation = AllTriggerActorsTriggered ? EndPoint : StartPoint;
		//if CurrentLocation is NOT equal to TargetLocation then the actor needs to move. If it is equal then no movement is necessary
		if (!CurrentLocation.Equals(TargetLocation))
		{
			//FMath::VInterpConstantTo is a function that moves the actor from the CurrentLocation to the TargetLocation, using the Speed variable
			FVector NewLocation =  FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, Speed);
			//Setting the owners location to the NewLocation
			MyOwner->SetActorLocation(NewLocation);
		}
	}
}

