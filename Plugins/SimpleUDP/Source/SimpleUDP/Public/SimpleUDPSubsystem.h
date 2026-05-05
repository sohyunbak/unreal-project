// Copyright (c) 2025 mengzhishanghun. All rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "Async/Future.h"
#include "SimpleUDPSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SimpleUDPSubsystem.generated.h"

class FSocket;
class FThreadSafeBool;

/**
 * 
 */
UCLASS()
class SIMPLEUDP_API USimpleUDPSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "SimpleUDP")
	static bool UpdateReceiveChannelConfig(const FName ReceiveChannel, const FUDPReceiveConfig& Config, const bool OverrideIfExists = true, const bool CreateIfNotExists = true);

	UFUNCTION(BlueprintCallable, Category = "SimpleUDP")
	static bool GetReceiveChannelConfig(const FName ReceiveChannel, FUDPReceiveConfig& OutConfig);

	UFUNCTION(BlueprintCallable, Category = "SimpleUDP")
	static TMap<FName, FUDPReceiveConfig> GetAllReceiveChannelConfig();

	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FSimpleUDPMessageDelegate, const FString&, SenderIP, const int32&, SenderPort, const TArray<uint8>&, Message);
	UFUNCTION(BlueprintCallable, Category = "SimpleUDP")
	static void BindUDPMessageHandler(const FName ReceiveChannel, const FSimpleUDPMessageDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "SimpleUDP")
	static void UnBindUDPMessageHandler(const FName ReceiveChannel, const FSimpleUDPMessageDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "SimpleUDP")
	static void UnBindAllUDPMessageHandler(const FName ReceiveChannel);

	UFUNCTION(BlueprintCallable, Category = "SimpleUDP")
	static bool UpdateSendChannelConfig(const FName SendChannel, const FUDPSendConfig& Config, const bool OverrideIfExists = true, const bool CreateIfNotExists = true);

	UFUNCTION(BlueprintCallable, Category = "SimpleUDP")
	static bool GetSendChannelConfig(const FName SendChannel, FUDPSendConfig& OutConfig);

	UFUNCTION(BlueprintCallable, Category = "SimpleUDP")
	static TMap<FName, FUDPSendConfig> GetAllSendChannelConfig();

	UFUNCTION(BlueprintCallable, Category = "SimpleUDP")
	static void SendUDPMessage(const FName SendChannel, const TArray<uint8>& Message);
private:
	static bool ParseEndpoint(const FString Input, FString& OutIP, int32& OutPort);
	static bool IsIPInCIDR(const FString IP, const FString Rule);
	static TSharedPtr<FSocket> GetOrCreateReceiveSocket(const FName ReceiveChannel);
	static TSharedPtr<FSocket> CreateReceiveSocket(const FName ReceiveChannel);
	static void StartReceiveThread(const FName ReceiveChannel);
	static void CheckCreateReceiveSocket(const FName ReceiveChannel);
	static void CheckCloseReceiveSocket(const FName ReceiveChannel);
	static void DestroyReceiveSocket(const FName ReceiveChannel);
private:
	TMap<FName, FUDPReceiveConfig> ReceiveConfigMap;
	TMap<FName, FUDPSendConfig> SendConfigMap;
	TMap<FName, TSharedPtr<FSocket>> ReceiveSocketMap;
	TMap<FName, TArray<FSimpleUDPMessageDelegate>> ReceiveCallbackMap;
	TMap<FName, TSharedPtr<FThreadSafeBool>> ReceiveThreadControlMap;
	TMap<FName, TFuture<void>> ReceiveThreadFutureMap;
	TMap<FName, TSharedPtr<FSocket>> SendSocketMap;
	static inline USimpleUDPSubsystem* Instance = nullptr;
};
