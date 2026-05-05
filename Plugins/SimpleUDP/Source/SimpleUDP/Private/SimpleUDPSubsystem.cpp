// Copyright (c) 2025 mengzhishanghun. All rights reserved.


#include "SimpleUDPSubsystem.h"

#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadSafeBool.h"
#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"

DEFINE_LOG_CATEGORY_STATIC(LogSimpleUDP, Log, All);

void USimpleUDPSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Instance = this;

	if (const USimpleUDPSettings* Settings = GetDefault<USimpleUDPSettings>())
	{
		ReceiveConfigMap.Append(Settings->ReceiveChannels);
		SendConfigMap.Append(Settings->SendChannels);
	}
}

void USimpleUDPSubsystem::Deinitialize()
{
	Super::Deinitialize();

    TArray<FName> Channels;
    ReceiveSocketMap.GetKeys(Channels);
    for (const FName& Ch : Channels)
    {
        DestroyReceiveSocket(Ch);
    }

	for (auto& Pair : SendSocketMap)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value->Close();
			Pair.Value.Reset();
		}
	}
	SendSocketMap.Empty();
	ReceiveCallbackMap.Empty();
	ReceiveConfigMap.Empty();
	SendConfigMap.Empty();
	ReceiveThreadControlMap.Empty();
	ReceiveThreadFutureMap.Empty();
	ReceiveSocketMap.Empty();

	Instance = nullptr;
}

bool USimpleUDPSubsystem::ParseEndpoint(const FString Input, FString& OutIP, int32& OutPort)
{
	FString IPPart, PortPart;
	if (!Input.Split(TEXT(":"), &IPPart, &PortPart))
	{
		UE_LOG(LogSimpleUDP, Error, TEXT("Failed to parse endpoint: %s"), *Input);
		return false;
	}

	OutIP = IPPart;
	OutPort = FCString::Atoi(*PortPart);

	return true;
}

bool USimpleUDPSubsystem::IsIPInCIDR(const FString IP, const FString Rule)
{
	FString NetStr, MaskStr;

	if (!Rule.Split(TEXT("/"), &NetStr, &MaskStr))
	{
		return IP == Rule;
	}

	FIPv4Address TargetIP, NetIP;
	int32 MaskBits = FCString::Atoi(*MaskStr);
	if (!FIPv4Address::Parse(IP, TargetIP) || !FIPv4Address::Parse(NetStr, NetIP))
		return false;

	uint32 TargetInt = TargetIP.Value;
	uint32 NetInt = NetIP.Value;
	MaskBits = FMath::Clamp(MaskBits, 0, 32);
	uint32 Mask = (MaskBits == 0) ? 0u : (0xFFFFFFFFu << (32 - MaskBits));

	return (TargetInt & Mask) == (NetInt & Mask);
}

bool USimpleUDPSubsystem::UpdateReceiveChannelConfig(const FName ReceiveChannel, const FUDPReceiveConfig& Config,
	const bool OverrideIfExists, const bool CreateIfNotExists)
{
	if (!Instance || ReceiveChannel.IsNone())
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("UpdateReceiveChannel: Invalid instance or channel"));
		return false;
	}

	const bool bExists = Instance->ReceiveConfigMap.Contains(ReceiveChannel);

	if (bExists && !OverrideIfExists)
	{
		UE_LOG(LogSimpleUDP, Log, TEXT("UpdateReceiveChannel: Channel [%s] already exists and override not allowed"), *ReceiveChannel.ToString());
		return false;
	}

	if (!bExists && !CreateIfNotExists)
	{
		UE_LOG(LogSimpleUDP, Log, TEXT("UpdateReceiveChannel: Channel [%s] does not exist and creation not allowed"), *ReceiveChannel.ToString());
		return false;
	}

	Instance->DestroyReceiveSocket(ReceiveChannel);

	Instance->ReceiveConfigMap.FindOrAdd(ReceiveChannel) = Config;

	CheckCreateReceiveSocket(ReceiveChannel);

	return true;
}

bool USimpleUDPSubsystem::GetReceiveChannelConfig(const FName ReceiveChannel, FUDPReceiveConfig& OutConfig)
{
	if (!Instance || ReceiveChannel.IsNone())
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("GetReceiveChannelConfig: Invalid instance or channel"));
		return false;
	}

	const FUDPReceiveConfig* Config = Instance->ReceiveConfigMap.Find(ReceiveChannel);
	if (!Config)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("GetReceiveChannelConfig: No config found for channel [%s]"), *ReceiveChannel.ToString());
		return false;
	}

	OutConfig = *Config;
	return true;
}

TMap<FName, FUDPReceiveConfig> USimpleUDPSubsystem::GetAllReceiveChannelConfig()
{
	if (!Instance)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("GetAllReceiveChannelConfig: Instance is null"));
		return TMap<FName, FUDPReceiveConfig>();
	}
	return Instance->ReceiveConfigMap;
}

void USimpleUDPSubsystem::BindUDPMessageHandler(const FName ReceiveChannel, const FSimpleUDPMessageDelegate& Callback)
{
	if (!Instance)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("BindUDPMessageHandler: Instance is null"));
		return;
	}

	if (ReceiveChannel.IsNone())
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("BindUDPMessageHandler: Channel is None"));
		return;
	}

	if (auto& CallbackArray = Instance->ReceiveCallbackMap.FindOrAdd(ReceiveChannel); !CallbackArray.Contains(Callback))
	{
		CallbackArray.Add(Callback);
		UE_LOG(LogSimpleUDP, Log, TEXT("BindUDPMessageHandler: Bound delegate to channel [%s], total: %d"), *ReceiveChannel.ToString(), CallbackArray.Num());
	}
	else
	{
		UE_LOG(LogSimpleUDP, Log, TEXT("BindUDPMessageHandler: Delegate already bound for channel [%s]"), *ReceiveChannel.ToString());
	}

	CheckCreateReceiveSocket(ReceiveChannel);
}

void USimpleUDPSubsystem::UnBindUDPMessageHandler(const FName ReceiveChannel, const FSimpleUDPMessageDelegate& Callback)
{
	if (!Instance)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("UnBindUDPMessageHandler: Instance is null"));
		return;
	}

	if (ReceiveChannel.IsNone())
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("UnBindUDPMessageHandler: Channel is None"));
		return;
	}

	if (TArray<FSimpleUDPMessageDelegate>* CallbackArray = Instance->ReceiveCallbackMap.Find(ReceiveChannel))
	{
		if (CallbackArray->Remove(Callback) > 0)
		{
			UE_LOG(LogSimpleUDP, Log, TEXT("UnBindUDPMessageHandler: Unbound delegate from channel [%s]"), *ReceiveChannel.ToString());
		}
		else
		{
			UE_LOG(LogSimpleUDP, Warning, TEXT("UnBindUDPMessageHandler: Delegate not found in channel [%s]"), *ReceiveChannel.ToString());
		}
	}
	else
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("UnBindUDPMessageHandler: No callback array found for channel [%s]"), *ReceiveChannel.ToString());
	}

	CheckCloseReceiveSocket(ReceiveChannel);
}


void USimpleUDPSubsystem::UnBindAllUDPMessageHandler(const FName ReceiveChannel)
{
	if (!Instance)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("UnBindAllUDPMessageHandler: Instance is null"));
		return;
	}

	if (ReceiveChannel.IsNone())
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("UnBindAllUDPMessageHandler: Channel is None"));
		return;
	}

	if (Instance->ReceiveCallbackMap.Remove(ReceiveChannel) > 0)
	{
		UE_LOG(LogSimpleUDP, Log, TEXT("UnBindAllUDPMessageHandler: Cleared all delegates for channel [%s]"), *ReceiveChannel.ToString());
	}
	else
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("UnBindAllUDPMessageHandler: No delegates found for channel [%s]"), *ReceiveChannel.ToString());
	}

	CheckCloseReceiveSocket(ReceiveChannel);
}

bool USimpleUDPSubsystem::UpdateSendChannelConfig(const FName SendChannel, const FUDPSendConfig& Config,
	const bool OverrideIfExists, const bool CreateIfNotExists)
{
	if (!Instance || SendChannel.IsNone())
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("UpdateSendChannel: Invalid instance or channel"));
		return false;
	}

	const bool bExists = Instance->SendConfigMap.Contains(SendChannel);

	if (bExists && !OverrideIfExists)
	{
		UE_LOG(LogSimpleUDP, Log, TEXT("UpdateSendChannel: Channel [%s] already exists and override not allowed"), *SendChannel.ToString());
		return false;
	}

	if (!bExists && !CreateIfNotExists)
	{
		UE_LOG(LogSimpleUDP, Log, TEXT("UpdateSendChannel: Channel [%s] does not exist and creation not allowed"), *SendChannel.ToString());
		return false;
	}

	Instance->SendConfigMap.FindOrAdd(SendChannel) = Config;
	return true;
}

bool USimpleUDPSubsystem::GetSendChannelConfig(const FName SendChannel, FUDPSendConfig& OutConfig)
{
	if (!Instance || SendChannel.IsNone())
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("GetSendChannelConfig: Invalid instance or channel"));
		return false;
	}

	const FUDPSendConfig* Config = Instance->SendConfigMap.Find(SendChannel);
	if (!Config)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("GetSendChannelConfig: No config found for channel [%s]"), *SendChannel.ToString());
		return false;
	}

	OutConfig = *Config;
	return true;
}

TMap<FName, FUDPSendConfig> USimpleUDPSubsystem::GetAllSendChannelConfig()
{
	if (!Instance)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("GetAllSendChannelConfig: Instance is null"));
		return TMap<FName, FUDPSendConfig>();
	}
	return Instance->SendConfigMap;
}

void USimpleUDPSubsystem::SendUDPMessage(const FName SendChannel, const TArray<uint8>& Message)
{
	if (!Instance)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("SendUDPMessage: Instance is null"));
		return;
	}

	if (SendChannel.IsNone())
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("SendUDPMessage: ChannelName is None"));
		return;
	}

	const FUDPSendConfig* Config = Instance->SendConfigMap.Find(SendChannel);
	if (!Config)
	{
		UE_LOG(LogSimpleUDP, Error, TEXT("SendUDPMessage: Channel [%s] not found in SendChannels"), *SendChannel.ToString());
		return;
	}

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	TSharedPtr<FSocket>& Socket = Instance->SendSocketMap.FindOrAdd(SendChannel);
	if (Socket.IsValid())
	{
		if (Config->FixedPort > 0)
		{
			TSharedRef<FInternetAddr> OutAddr = SocketSubsystem->CreateInternetAddr();
			Socket->GetAddress(*OutAddr);
			if (OutAddr->GetPort() != Config->FixedPort)
			{
				Socket->Close();
				Socket.Reset();
			}
		}
	}

	if (!Socket.IsValid())
	{
		Socket = TSharedPtr<FSocket>(SocketSubsystem->CreateSocket(NAME_DGram, TEXT("SimpleUDPSender"), false));

		if (!Socket.IsValid())
		{
			UE_LOG(LogSimpleUDP, Error, TEXT("SendUDPMessage: Failed to create socket for channel [%s]"), *SendChannel.ToString());
			return;
		}

		Socket->SetReuseAddr(true);
		Socket->SetBroadcast(true);
		Socket->SetNonBlocking(true);
		Socket->SetRecvErr();

		TSharedRef<FInternetAddr> LocalAddr = SocketSubsystem->CreateInternetAddr();
		LocalAddr->SetAnyAddress();
		LocalAddr->SetPort(Config->FixedPort);
		bool bBindSuccess = Socket->Bind(*LocalAddr);
		if (!bBindSuccess)
		{
			UE_LOG(LogSimpleUDP, Error, TEXT("SendUDPMessage: Failed to bind socket for channel [%s]"), *SendChannel.ToString());
			Socket.Reset();
			return;
		}
	}

	for (const FString& Target : Config->Targets)
	{
		FString IP;
		int32 Port;
		if (!ParseEndpoint(Target, IP, Port))
		{
			UE_LOG(LogSimpleUDP, Warning, TEXT("SendUDPMessage: Invalid target address [%s]"), *Target);
			continue;
		}

		bool bIsValidIP = false;
		TSharedRef<FInternetAddr> RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		RemoteAddr->SetIp(*IP, bIsValidIP);
		RemoteAddr->SetPort(Port);

		if (!bIsValidIP)
		{
			UE_LOG(LogSimpleUDP, Warning, TEXT("SendUDPMessage: Invalid IP [%s]"), *IP);
			continue;
		}

		int32 BytesSent = 0;
		int32 BytesToSend = FMath::Min(Message.Num(), Config->MaxSendBytes);
		if (Socket->SendTo(Message.GetData(), BytesToSend, BytesSent, *RemoteAddr))
		{
			UE_LOG(LogSimpleUDP, Log, TEXT("SendUDPMessage: Sent %d bytes to [%s:%d] on channel [%s]"), BytesSent, *IP, Port, *SendChannel.ToString());
		}
		else
		{
			UE_LOG(LogSimpleUDP, Warning, TEXT("SendUDPMessage: Failed to send to [%s:%d]"), *IP, Port);
		}
	}
}

TSharedPtr<FSocket> USimpleUDPSubsystem::GetOrCreateReceiveSocket(const FName ReceiveChannel)
{
	if (!Instance)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("GetOrCreateReceiveSocket: Instance is null"));
		return nullptr;
	}

	if (Instance->ReceiveSocketMap.Contains(ReceiveChannel))
	{
		UE_LOG(LogSimpleUDP, Log, TEXT("GetOrCreateReceiveSocket: Reusing existing socket for channel [%s]"), *ReceiveChannel.ToString());
		return Instance->ReceiveSocketMap[ReceiveChannel];
	}

	TSharedPtr<FSocket> NewSocket = CreateReceiveSocket(ReceiveChannel);
	if (NewSocket.IsValid())
	{
		UE_LOG(LogSimpleUDP, Log, TEXT("GetOrCreateReceiveSocket: Socket created, adding to map for channel [%s]"), *ReceiveChannel.ToString());
		Instance->ReceiveSocketMap.Add(ReceiveChannel, NewSocket);
		StartReceiveThread(ReceiveChannel);
	}
	else
	{
		UE_LOG(LogSimpleUDP, Error, TEXT("GetOrCreateReceiveSocket: Failed to create socket for channel [%s]"), *ReceiveChannel.ToString());
	}

	return NewSocket;
}

TSharedPtr<FSocket> USimpleUDPSubsystem::CreateReceiveSocket(const FName ReceiveChannel)
{
	const FUDPReceiveConfig* Config = Instance->ReceiveConfigMap.Find(ReceiveChannel);
	if (!Config)
	{
		UE_LOG(LogSimpleUDP, Error, TEXT("CreateReceiveSocket: No config found for channel [%s]"), *ReceiveChannel.ToString());
		return nullptr;
	}

	FString IP;
	int32 Port;
	if (!ParseEndpoint(Config->BindAddress, IP, Port))
	{
		UE_LOG(LogSimpleUDP, Error, TEXT("CreateReceiveSocket: Invalid BindAddress [%s] for channel [%s]"), *Config->BindAddress, *ReceiveChannel.ToString());
		return nullptr;
	}

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	const FString DebugName = FString::Printf(TEXT("UDPRecv_%s"), *ReceiveChannel.ToString());

	UE_LOG(LogSimpleUDP, Log, TEXT("CreateReceiveSocket: Creating socket for channel [%s]"), *ReceiveChannel.ToString());

	FSocket* RawSocket = SocketSubsystem->CreateSocket(NAME_DGram, *DebugName, false);
	if (!RawSocket)
	{
		UE_LOG(LogSimpleUDP, Error, TEXT("CreateReceiveSocket: Failed to create raw socket for channel [%s]"), *ReceiveChannel.ToString());
		return nullptr;
	}

	RawSocket->SetReuseAddr(true);
	RawSocket->SetNonBlocking(true);
	RawSocket->SetRecvErr();

	TSharedRef<FInternetAddr> BindAddr = SocketSubsystem->CreateInternetAddr();
	bool bIsValidIP = false;
	BindAddr->SetIp(*IP, bIsValidIP);
	BindAddr->SetPort(Port);

	if (!bIsValidIP)
	{
		UE_LOG(LogSimpleUDP, Error, TEXT("CreateReceiveSocket: Invalid IP [%s] for channel [%s]"), *IP, *ReceiveChannel.ToString());
		SocketSubsystem->DestroySocket(RawSocket);
		return nullptr;
	}

	if (!RawSocket->Bind(*BindAddr))
	{
		UE_LOG(LogSimpleUDP, Error, TEXT("CreateReceiveSocket: Failed to bind [%s:%d] for channel [%s]"), *IP, Port, *ReceiveChannel.ToString());
		SocketSubsystem->DestroySocket(RawSocket);
		return nullptr;
	}

	UE_LOG(LogSimpleUDP, Log, TEXT("CreateReceiveSocket: Successfully bound [%s:%d] for channel [%s]"), *IP, Port, *ReceiveChannel.ToString());
	return MakeShareable(RawSocket);
}

void USimpleUDPSubsystem::StartReceiveThread(const FName ReceiveChannel)
{
	if (!Instance)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("StartReceiveThread: Instance is null"));
		return;
	}

	TSharedPtr<FSocket>* SocketPtr = Instance->ReceiveSocketMap.Find(ReceiveChannel);
	if (!SocketPtr || !SocketPtr->IsValid())
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("StartReceiveThread: No valid socket found for channel [%s]"), *ReceiveChannel.ToString());
		return;
	}

	TSharedPtr<FSocket> Socket = *SocketPtr;

	const FUDPReceiveConfig* Config = Instance->ReceiveConfigMap.Find(ReceiveChannel);
	if (!Config)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("StartReceiveThread: No config found for channel [%s]"), *ReceiveChannel.ToString());
		return;
	}

	// 创建控制器并保存
	TSharedPtr<FThreadSafeBool> bRunFlag = MakeShared<FThreadSafeBool>(true);
	Instance->ReceiveThreadControlMap.Add(ReceiveChannel, bRunFlag);

	int32 BufferSize = Config->MaxReceiveBytes > 0 ? Config->MaxReceiveBytes : 1024;

	UE_LOG(LogSimpleUDP, Log, TEXT("StartReceiveThread: Starting UDP listen thread for channel [%s] with buffer size %d"), *ReceiveChannel.ToString(), BufferSize);

	TFuture<void> ThreadFuture = Async(EAsyncExecution::Thread, [Socket, ReceiveChannel, bRunFlag, BufferSize]()
	{
		TArray<uint8> Buffer;
		Buffer.SetNumUninitialized(BufferSize);
		TSharedRef<FInternetAddr> SenderAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

		while (bRunFlag.IsValid() && *bRunFlag && Socket.IsValid())
		{
			/* 先等待最多 10 ms，看是否有数据可读，避免 CPU 空转。
			   UDP 套接字一直是“未连接”状态，因此不再使用 GetConnectionState。*/
			if (!Instance || !Socket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromMilliseconds(10)))
			{
				// 超时：10 ms 内没有数据，进入下一轮循环
				continue;
			}

			int32 BytesRead = 0;
			if (Socket->RecvFrom(Buffer.GetData(), Buffer.Num(), BytesRead, *SenderAddr))
			{
				FString SenderIP = SenderAddr->ToString(false);
				int32 SenderPort = SenderAddr->GetPort();

				// --- 👇 安全转码 & 裁剪，彻底杜绝脏数据 ---

				// 1. 保证不会越界，也避免把上一次残留内容带进来
				BytesRead = FMath::Clamp(BytesRead, 0, Buffer.Num() - 1);
				Buffer[BytesRead] = 0;            // 在有效数据尾部写 0 终止

				// 2. 将收到的“有效区”视为 UTF-8 字节流并转成 TCHAR

				bool bBlocked = false;
				const FUDPReceiveConfig ConfigCopy = Instance->ReceiveConfigMap.FindRef(ReceiveChannel);
				if (ConfigCopy.FilterMode == EUDPFilterMode::WhitelistOnly)
				{
					bBlocked = true;
					for (const FString& Rule : ConfigCopy.WhitelistIPs)
					{
						if (Instance->IsIPInCIDR(SenderIP, Rule))
						{
							bBlocked = false;
							break;
						}
					}

					if (bBlocked)
					{
						UE_LOG(LogSimpleUDP, Warning, TEXT("StartReceiveThread: [%s] rejected (not in whitelist)"), *SenderIP);
					}
				}
				else if (ConfigCopy.FilterMode == EUDPFilterMode::BlacklistOnly)
				{
					for (const FString& Rule : ConfigCopy.BlacklistIPs)
					{
						if (Instance->IsIPInCIDR(SenderIP, Rule))
						{
							bBlocked = true;
							UE_LOG(LogSimpleUDP, Warning, TEXT("StartReceiveThread: [%s] rejected (blacklisted)"), *SenderIP);
							break;
						}
					}
				}

				if (!bBlocked)
				{
					AsyncTask(ENamedThreads::GameThread, [ReceiveChannel, SenderIP, SenderPort, Buffer]()
					{
						if (Instance)
						{
							const auto& CallbackMap = Instance->ReceiveCallbackMap;
							if (const TArray<FSimpleUDPMessageDelegate>* Callbacks = CallbackMap.Find(ReceiveChannel))
							{
								UE_LOG(LogSimpleUDP, Log, TEXT("StartReceiveThread: Dispatching message from [%s] to %d delegate(s) on channel [%s]"),
									*SenderIP, Callbacks->Num(), *ReceiveChannel.ToString());

								for (const auto& Callback : *Callbacks)
								{
									if (Callback.IsBound())
										Callback.Execute(SenderIP, SenderPort, Buffer);
								}
							}
							else
							{
								UE_LOG(LogSimpleUDP, Warning, TEXT("StartReceiveThread: No delegates registered for channel [%s]"), *ReceiveChannel.ToString());
							}
						}
					});
				}
			}
		}

		UE_LOG(LogSimpleUDP, Log, TEXT("StartReceiveThread: Stopped listening for channel [%s]"), *ReceiveChannel.ToString());
	});

	// ✨ 把句柄存进表，稍后等线程退出再销毁 Socket
	Instance->ReceiveThreadFutureMap.Add(ReceiveChannel, MoveTemp(ThreadFuture));
}

void USimpleUDPSubsystem::CheckCreateReceiveSocket(const FName ReceiveChannel)
{
	if (!Instance)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("CheckCreateReceiveSocket: Instance is null"));
		return;
	}

	TSharedPtr<FSocket>* SocketPtr = Instance->ReceiveSocketMap.Find(ReceiveChannel);
	if (SocketPtr && SocketPtr->IsValid())
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("CheckCreateReceiveSocket: Is valid socket found for channel [%s]"), *ReceiveChannel.ToString());
		return;
	}

	// 如果当前通道已有委托，则重新创建 socket（否则延迟到 Bind 时再建）
	if (Instance->ReceiveCallbackMap.Contains(ReceiveChannel))
	{
		if (const TSharedPtr<FSocket> Socket = USimpleUDPSubsystem::GetOrCreateReceiveSocket(ReceiveChannel); !Socket.IsValid())
		{
			UE_LOG(LogSimpleUDP, Error, TEXT("UpdateReceiveChannel: Failed to obtain socket for channel [%s]"), *ReceiveChannel.ToString());
		}
	}
	else
	{
		UE_LOG(LogSimpleUDP, Log, TEXT("UpdateReceiveChannel: Config applied for channel [%s], waiting for bind"), *ReceiveChannel.ToString());
	}
}

void USimpleUDPSubsystem::CheckCloseReceiveSocket(const FName ReceiveChannel)
{
	if (!Instance)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("CheckCloseReceiveSocket: Instance is null"));
		return;
	}

	TSharedPtr<FSocket>* SocketPtr = Instance->ReceiveSocketMap.Find(ReceiveChannel);
	if (!SocketPtr || !SocketPtr->IsValid())
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("CheckCloseReceiveSocket: No valid socket found for channel [%s]"), *ReceiveChannel.ToString());
		return;
	}

	const TArray<FSimpleUDPMessageDelegate>* CallbackArray = Instance->ReceiveCallbackMap.Find(ReceiveChannel);
	if (!CallbackArray || CallbackArray->IsEmpty())
	{
		UE_LOG(LogSimpleUDP, Log, TEXT("CheckCloseReceiveSocket: No delegates left on channel [%s], releasing socket"), *ReceiveChannel.ToString());

		Instance->ReceiveCallbackMap.Remove(ReceiveChannel);
		DestroyReceiveSocket(ReceiveChannel);
	}
	else
	{
		UE_LOG(LogSimpleUDP, Verbose, TEXT("CheckCloseReceiveSocket: Still has %d delegate(s) on channel [%s]"), CallbackArray->Num(), *ReceiveChannel.ToString());
	}
}

void USimpleUDPSubsystem::DestroyReceiveSocket(const FName ReceiveChannel)
{
	if (!Instance)
	{
		UE_LOG(LogSimpleUDP, Warning, TEXT("DestroyReceiveSocket: Instance is null"));
		return;
	}

	// 停止线程
	if (TSharedPtr<FThreadSafeBool>* FlagPtr = Instance->ReceiveThreadControlMap.Find(ReceiveChannel))
	{
		if (FlagPtr->IsValid())
		{
			(*FlagPtr)->AtomicSet(false);  // 通知线程退出
			Instance->ReceiveThreadControlMap.Remove(ReceiveChannel);
			UE_LOG(LogSimpleUDP, Log, TEXT("DestroyReceiveSocket: Exit flag set for channel [%s]"), *ReceiveChannel.ToString());
		}
	}

	// 2) 等待线程结束
	if (TFuture<void>* FuturePtr = Instance->ReceiveThreadFutureMap.Find(ReceiveChannel))
	{
		FuturePtr->Wait();                         // 阻塞直至 lambda 结束
		Instance->ReceiveThreadFutureMap.Remove(ReceiveChannel);
		UE_LOG(LogSimpleUDP, Log, TEXT("DestroyReceiveSocket: Thread joined for channel [%s]"), *ReceiveChannel.ToString());
	}

	// 关闭并销毁 Socket
	if (TSharedPtr<FSocket>* SocketPtr = Instance->ReceiveSocketMap.Find(ReceiveChannel))
	{
		if (SocketPtr->IsValid())
		{
			(*SocketPtr)->Close();
			SocketPtr->Reset();
			UE_LOG(LogSimpleUDP, Log, TEXT("DestroyReceiveSocket: Closed and destroyed socket for channel [%s]"), *ReceiveChannel.ToString());
		}
		Instance->ReceiveSocketMap.Remove(ReceiveChannel);
	}
}

