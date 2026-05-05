// Copyright (c) 2025 mengzhishanghun. All rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SimpleUDPSettings.generated.h"

UENUM(BlueprintType)
enum class EUDPFilterMode : uint8
{
	None UMETA(DisplayName = "No Filtering"),
	WhitelistOnly UMETA(DisplayName = "Whitelist Only"),
	BlacklistOnly UMETA(DisplayName = "Blacklist Only")
};

USTRUCT(BlueprintType)
struct FUDPReceiveConfig
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, config, Category="Receive")
	FString BindAddress = TEXT("127.0.0.1:9000");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, config, Category="Receive")
	EUDPFilterMode FilterMode = EUDPFilterMode::None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, config, Category="Receive", meta = (EditCondition = "FilterMode == EUDPFilterMode::WhitelistOnly"))
	TArray<FString> WhitelistIPs;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, config, Category="Receive", meta = (EditCondition = "FilterMode == EUDPFilterMode::BlacklistOnly"))
	TArray<FString> BlacklistIPs;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, config, Category="Receive", meta=(ClampMin="256", ClampMax="65507"))
	int32 MaxReceiveBytes = 1024;

};

USTRUCT(BlueprintType)
struct FUDPSendConfig
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, config, Category="Send")
	TArray<FString> Targets;  // "IP:Port"

	UPROPERTY(BlueprintReadWrite, EditAnywhere, config, Category="Send", meta=(ClampMin="0", ClampMax="65535"))
	int32 FixedPort = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, config, Category="Send", meta=(ClampMin="256", ClampMax="65507"))
	int32 MaxSendBytes  = 1024;
};

/**
 * 
 */
UCLASS(config=SimpleUDP, defaultconfig, meta=(DisplayName="Simple UDP Settings"))
class SIMPLEUDP_API USimpleUDPSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	/** Receive-only channels */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, config, Category = "SimpleUDP")
	TMap<FName, FUDPReceiveConfig> ReceiveChannels;

	/** Send-only channels */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, config, Category = "SimpleUDP")
	TMap<FName, FUDPSendConfig> SendChannels;
};
