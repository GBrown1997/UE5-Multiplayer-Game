// Fill out your copyright notice in the Description page of Project Settings.


#include "Net/UnrealNetwork.h"
#include "MultiplayerGameCharacter.h"

#include "CollectableKey.h"

// Sets default values
ACollectableKey::ACollectableKey()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Capsule = CreateDefaultSubobject <UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(RootComp);
	Capsule->SetIsReplicated(true);
	//Turns off collision for the capsule component
	Capsule->SetCollisionProfileName(FName("OverlapAllDynamic"));
	Capsule->SetCapsuleHalfHeight(150.0f);
	Capsule->SetCapsuleRadius(100.0f);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);
	//Turns off collision for the mesh
	Mesh->SetCollisionProfileName(FName("OverlapAllDynamic"));

	CollectAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("CollectAudio"));
	CollectAudio->SetupAttachment(RootComp);
	//Makes sure audio does not play on game start
	CollectAudio->SetAutoActivate(false);

	RotationSpeed = 100.0f;
}

// Called when the game starts or when spawned
void ACollectableKey::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACollectableKey::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (HasAuthority())
	{
		//Rotates the StaticMesh
		Mesh->AddRelativeRotation(FRotator(0.0f, RotationSpeed * DeltaTime, 0.0f));
		
		//COLLISION DETECTION CODE
		TArray<AActor*> OverlapActors;
		//Ensures that GetOverlappingActors will ignore every type of actor except MultiplayerGameCharacter character actors
		Capsule->GetOverlappingActors(OverlapActors, AMultiplayerGameCharacter::StaticClass());

		if (OverlapActors.Num() > 0)
		{
			//A player character is overlapping the capsule

			if (!IsCollected) 
			{
				//The IsCollected boolean is changed, meaning the OnRep_IsCollected function is called on the client
				IsCollected = true;
				//Ensures that the OnRep_IsCollected function is also called on the server
				OnRep_IsCollected();
			}
		}
	}
}

void ACollectableKey::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//
	DOREPLIFETIME(ACollectableKey, IsCollected);
}

void ACollectableKey::OnRep_IsCollected()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Display, TEXT("OnRep_IsCollected called from the Server!"));

		if (IsCollected)
		{
			OnCollected.Broadcast();
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("OnRep_IsCollected called from the Client!"));
	}

	Mesh->SetVisibility(!IsCollected);

	//Plays the key collecting sound when the key is selected
	CollectAudio->Play();

	if (IsCollected)
	{
		if (KeyHolderRef)
		{
			KeyHolderRef->ActivateKeyMesh();
		}
	}
}

