// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
//Allow for connection to the online subsystem
//Can then connect to a session interface so players can find and host games
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include <FindSessionsCallbackProxy.h>



//PrintString function to easily print strings to screen
void PrintString(const FString& Str)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, Str);
	}
}

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
{
	//PrintString("MSS Constructor");

	CreateServerAfterDestroy = false;
	DestroyServerName = "";
	ServerNameToFind = "";
	MySessionName = FName("Multiplayer Game Session Name");
}

void UMultiplayerSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	//PrintString("MSS Initialize");

	IOnlineSubsystem *OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		//Testing which Online Subsystem we are connecting to. Prints the name to the console/screen.
		FString SubsystemName = OnlineSubsystem->GetSubsystemName().ToString();
		PrintString(SubsystemName);

		SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			//A Delegate is a data type that stores references to functions, and calls those functions when it is fired
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnJoinSessionComplete);
		}
	}
}

void UMultiplayerSessionsSubsystem::Deinitialize()
{
	//UE_LOG(LogTemp, Warning, TEXT("MSS Deinitialize"));
}

void UMultiplayerSessionsSubsystem::CreateServer(FString ServerName)
{
	PrintString("CreateServer");

	//if the server name is empty then nothing happens. Server requires a name to be made.
	if (ServerName.IsEmpty())
	{
		PrintString("Server name cannot be empty!");
		ServerCreateDel.Broadcast(false);
		return;
	}

	//Check if session name already exists
	//If it exists then below string is printed, and the session is then destroyed and the function in returned
	FNamedOnlineSession *ExistingSession = SessionInterface->GetNamedSession(MySessionName);
	if (ExistingSession)
	{
		FString Msg = FString::Printf(TEXT("Session with name %s already exists, destroying it."), *MySessionName.ToString());
		PrintString(Msg);
		CreateServerAfterDestroy = true;
		DestroyServerName = ServerName;
		SessionInterface->DestroySession(MySessionName);
		return;
	}

	FOnlineSessionSettings SessionSettings;

	//Allows players to join the game while the game is running
	SessionSettings.bAllowJoinInProgress = true;
	
	//Server IS NOT a dedicated server
	SessionSettings.bIsDedicated = false;

	//Session is advertised on online servers. Players can find the session
	SessionSettings.bShouldAdvertise = true;

	//Maximum amount of players allowed in a session/game
	SessionSettings.NumPublicConnections = 4;

	//Connects to STEAM's lobbies API
	SessionSettings.bUseLobbiesIfAvailable = true;

	//Region Systems and User Information STEAM uses. STEAM uses region of player to find sessions
	SessionSettings.bUsesPresence = true;

	SessionSettings.bAllowJoinViaPresence = true;

	bool IsLAN = false;
	if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		IsLAN = true;
	}
	//if set to true game is LAN only
	SessionSettings.bIsLANMatch = IsLAN;

	SessionSettings.Set(FName("SERVER_NAME"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	//calls the SessionInterface function CreateSession. Server needs a Session in order to be created
	//("HostingPlayerNum" 0=us, "SessionName"=FName, "&NewSessionSettings" Needs SessionSettings header file)
	//Session name != server name. Used to differentiate sessions. No two session names can be the same
	SessionInterface->CreateSession(0, MySessionName, SessionSettings);
}

void UMultiplayerSessionsSubsystem::FindServer(FString ServerName)
{
	PrintString("FindServer");

	if (ServerName.IsEmpty())
	{
		PrintString("Server name cannot be empty!");
		ServerJoinDel.Broadcast(false);
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	bool IsLAN = false;
	if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		IsLAN = true;
	}

	SessionSearch->bIsLanQuery = IsLAN;
	SessionSearch->MaxSearchResults = 9999;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	ServerNameToFind = ServerName;

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	PrintString(FString::Printf(TEXT("OnCreateSessionComplete: %d"), bWasSuccessful));

	ServerCreateDel.Broadcast(bWasSuccessful);

	if (bWasSuccessful)
	{
		FString Path = "/Game/ThirdPerson/Maps/ThirdPersonMap?listen";

		if (!GameMapPath.IsEmpty())
		{
			Path = FString::Printf(TEXT("%s?listen"), *GameMapPath);
		}

		GetWorld()->ServerTravel(Path);
	}
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	FString Msg = FString::Printf(TEXT("OnDestroySessionComplete, SessionName: %s, Success: %d"), *SessionName.ToString(), bWasSuccessful);
	PrintString(Msg);

	if (CreateServerAfterDestroy)
	{
		CreateServerAfterDestroy = false;
		CreateServer(DestroyServerName);
	}
	
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful) return;
	if (ServerNameToFind.IsEmpty()) return;

	TArray<FOnlineSessionSearchResult> Results = SessionSearch->SearchResults;
	FOnlineSessionSearchResult* CorrectResult = 0;

	if (Results.Num() > 0)
	{
		FString Msg = FString::Printf(TEXT("%d sessions found."), Results.Num());
		PrintString(Msg);

		for (FOnlineSessionSearchResult Result : Results)
		{
			if (Result.IsValid())
			{
				FString ServerName = "No-Name";
				Result.Session.SessionSettings.Get(FName("SERVER_NAME"), ServerName);

				if (ServerName.Equals(ServerNameToFind))
				{
					CorrectResult = &Result;
					FString Msg2 = FString::Printf(TEXT("Found Server With Name: %s"), *ServerName);
					PrintString(Msg2);
					break;
				}
			}
		}

		if (CorrectResult)
		{
			SessionInterface->JoinSession(0, MySessionName, *CorrectResult);
		}
		else
		{
			PrintString(FString::Printf(TEXT("Couldn't Find Server: %s"), *ServerNameToFind));
			ServerNameToFind = "";
			ServerJoinDel.Broadcast(false);
		}
	}
	else
	{
		PrintString("Zero sessions found.");
		ServerJoinDel.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	ServerJoinDel.Broadcast(Result == EOnJoinSessionCompleteResult::Success);

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString Msg = FString::Printf(TEXT("Successfully Joined Session %s"), *SessionName.ToString());
		PrintString(Msg);

		FString Address = "";
		bool Success = SessionInterface->GetResolvedConnectString(MySessionName, Address);
		if (Success)
		{
			PrintString(FString::Printf(TEXT("Address: %s"), *Address));
			APlayerController *PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
		else
		{
			PrintString("GetResolvedConnectString returned false!");
		}
	}
	else
	{
		PrintString("OnJoinSessionComplete failed");
	}
}