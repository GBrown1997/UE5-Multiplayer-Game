// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Transporter.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )

class MULTIPLAYERGAME_API UTransporter : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTransporter();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Transporter will move coresponding actor from the given start point to the given endpoint
	//These FVectors will be two locations in the game world
	FVector StartPoint;
	FVector EndPoint;

	//Boolean to check if the start and end points are set
	bool ArePointsSet;

	//Will contain/control the amount of time (in seconds) it takes to move the actor from the start to the end point
	UPROPERTY(EditAnywhere)
	float MoveTime;

	//Adds boolean to determine whether the actor that has the transporter component on it, is a TriggerActor
	UPROPERTY(EditAnywhere)
	bool OwnerIsTriggerActor;

	//The pressure plates that will cause the actor to move
	UPROPERTY(EditAnywhere)
	TArray<AActor*> TriggerActors;

	//The amount of activated trigger actors needed to cause the actor to move
	UPROPERTY(VisibleAnywhere)
	int ActivatedTriggerCount;

	//Boolean to check if all the correct trigger actors are triggered/activated
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	bool AllTriggerActorsTriggered;

	UFUNCTION(BlueprintCallable)
	void SetPoints(FVector Point1, FVector Point2);

	UFUNCTION()
	void OnTriggerActorActivated();

	UFUNCTION()
	void OnTriggerActorDeactivated();
};
