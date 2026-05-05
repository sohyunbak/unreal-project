// Copyright (c) 2025 mengzhishanghun. All rights reserved.



#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimpleByteConversionBPLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLEBYTECONVERSION_API USimpleByteConversionBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	// === Int32 ===
	UFUNCTION(BlueprintPure, Category = "SimpleByteConversion|Int")
	static void IntToBytes(int32 Value, TArray<uint8>& Bytes);

	UFUNCTION(BlueprintPure, Category = "SimpleByteConversion|Int")
	static void BytesToInt(const TArray<uint8>& Bytes, int32& Value);

	// === Int64 ===
	UFUNCTION(BlueprintPure, Category = "SimpleByteConversion|Int64")
	static void Int64ToBytes(int64 Value, TArray<uint8>& Bytes);
	UFUNCTION(BlueprintPure, Category = "SimpleByteConversion|Int64")
	static void BytesToInt64(const TArray<uint8>& Bytes, int64& Value);

	// === Float ===
	UFUNCTION(BlueprintPure, Category = "SimpleByteConversion|Float")
	static void FloatToBytes(float Value, TArray<uint8>& Bytes);

	UFUNCTION(BlueprintPure, Category = "SimpleByteConversion|Float")
	static void BytesToFloat(const TArray<uint8>& Bytes, float& Value);

	// === Double ===
	UFUNCTION(BlueprintPure, Category = "SimpleByteConversion|Double")
	static void DoubleToBytes(double Value, TArray<uint8>& Bytes);

	UFUNCTION(BlueprintPure, Category = "SimpleByteConversion|Double")
	static void BytesToDouble(const TArray<uint8>& Bytes, double& Value);

	// === Bool ===
	UFUNCTION(BlueprintPure, Category = "SimpleByteConversion|Bool")
	static void BoolToBytes(bool Value, TArray<uint8>& Bytes);

	UFUNCTION(BlueprintPure, Category = "SimpleByteConversion|Bool")
	static void BytesToBool(const TArray<uint8>& Bytes, bool& Value);

	// === String ===
	UFUNCTION(BlueprintPure, Category = "SimpleByteConversion|String")
	static void StringToBytes(const FString& Value, TArray<uint8>& Bytes);

	UFUNCTION(BlueprintPure, Category = "SimpleByteConversion|String")
	static void BytesToString(const TArray<uint8>& Bytes, FString& Value);

	// === Struct ===
	UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Struct", DisplayName = "Struct To Bytes"), Category = "SimpleByteConversion|Struct")
	static void StructToBytes(const int32& Struct, TArray<uint8>& Bytes);
	DECLARE_FUNCTION(execStructToBytes);

	UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Struct", DisplayName = "Bytes To Struct"), Category = "SimpleByteConversion|Struct")
	static void BytesToStruct(const TArray<uint8>& Bytes, int32& Struct);
	DECLARE_FUNCTION(execBytesToStruct);

	UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Struct", DisplayName = "Struct To JsonString"), Category = "SimpleByteConversion|Struct")
	static void StructToJsonString(const int32& Struct, FString& JsonString);
	DECLARE_FUNCTION(execStructToJsonString);

	UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Struct", DisplayName = "JsonString To Struct"), Category = "SimpleByteConversion|Struct")
	static void JsonStringToStruct(const FString& JsonString, int32& Struct);
	DECLARE_FUNCTION(execJsonStringToStruct);
};
