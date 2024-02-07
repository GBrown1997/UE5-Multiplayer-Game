// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/BoxComponent.h"

#include "WinArea.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWinAreaOnWinCondition);

UCLASS()
class MULTIPLAYERGAME_API AWinArea : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWinArea();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	bool WinCondition;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UBoxComponent* WinAreaBox;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCWin();

	UPROPERTY(BlueprintAssignable)
	FWinAreaOnWinCondition OnWinCondition;
};
