// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/StaticMeshComponent.h"
#include "Transporter.h"

#include "PressurePlate.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPressurePlateOnActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPressurePlateOnDeactivated);

UCLASS()
class MULTIPLAYERGAME_API APressurePlate : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APressurePlate();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Pointer to Root Component. Everything else will be a child of this component
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	USceneComponent* RootComp;

	//Pointer to component that will detect collision. Is anything standing on top of the pressure plate?
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UStaticMeshComponent* TriggerMesh;

	//Pointer to component that will be the 3d model used for the presssure plate. Will look like a pressure plate and have collision
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	//True if something is on top of pressure plate. False if nothing on top
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	bool Activated;

	UPROPERTY(BlueprintAssignable)
	FPressurePlateOnActivated OnActivated;

	UPROPERTY(BlueprintAssignable)
	FPressurePlateOnDeactivated OnDeactivated;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UTransporter* Transporter;
};
