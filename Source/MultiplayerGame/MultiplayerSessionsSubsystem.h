// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
//Makes it so we can access the session interface
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MultiplayerSessionsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerCreateDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerJoinDelegate, bool, bWasSuccessful);

/**
 * 
 */
UCLASS()
class MULTIPLAYERGAME_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	

public:
	UMultiplayerSessionsSubsystem();

	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	//Make SessionInterface a member variable. Easier to call for multiple functions
	IOnlineSessionPtr SessionInterface;

	UFUNCTION(BluePrintCallable)

	void CreateServer(FString ServerName);

	UFUNCTION(BluePrintCallable)

	void FindServer(FString ServerName);

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	bool CreateServerAfterDestroy;
	FString DestroyServerName;
	FString ServerNameToFind;
	FName MySessionName;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UPROPERTY(BlueprintAssignable)
	FServerCreateDelegate ServerCreateDel;

	UPROPERTY(BlueprintAssignable)
	FServerJoinDelegate ServerJoinDel;

	UPROPERTY(BlueprintReadWrite)
	FString GameMapPath;
};
