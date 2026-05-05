// Copyright (c) 2025 mengzhishanghun. All rights reserved.



#pragma once

#include "CoreMinimal.h"

class SIMPLEBYTECONVERSION_API ByteConverter
{
public:

	// === Int ===
	static void IntToBytes(int32 Value, TArray<uint8>& OutBytes);
	static void BytesToInt(const TArray<uint8>& InBytes, int32& OutValue);

	// === Int64 ===
	static void Int64ToBytes(int64 Value, TArray<uint8>& OutBytes);
	static void BytesToInt64(const TArray<uint8>& InBytes, int64& OutValue);

	// === Float ===
	static void FloatToBytes(float Value, TArray<uint8>& OutBytes);
	static void BytesToFloat(const TArray<uint8>& InBytes, float& OutValue);

	// === Double ===
	static void DoubleToBytes(double Value, TArray<uint8>& OutBytes);
	static void BytesToDouble(const TArray<uint8>& InBytes, double& OutValue);

	// === Bool ===
	static void BoolToBytes(bool Value, TArray<uint8>& OutBytes);
	static void BytesToBool(const TArray<uint8>& InBytes, bool& OutValue);

	// === String ===
	static void StringToBytes(const FString& InString, TArray<uint8>& OutBytes);
	static void BytesToString(const TArray<uint8>& InBytes, FString& OutString);

	// === Struct ===
	static void StructToBytes(void* StructPtr, const UStruct* StructType, TArray<uint8>& OutBytes);
	static void BytesToStruct(const TArray<uint8>& InBytes, void* OutStructPtr, const UStruct* StructType);

	static void StructToJsonString(const void* StructPtr, const UStruct* StructType, FString& OutJsonString);
	static void JsonStringToStruct(const FString& InJsonString, void* OutStructPtr, const UStruct* StructType);
};
