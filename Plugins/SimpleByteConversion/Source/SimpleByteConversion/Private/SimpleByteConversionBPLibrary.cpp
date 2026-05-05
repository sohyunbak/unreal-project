// Copyright (c) 2025 mengzhishanghun. All rights reserved.



#include "SimpleByteConversionBPLibrary.h"
#include "ByteConverter.h"
#include "UObject/UnrealType.h"

// === Int ===
void USimpleByteConversionBPLibrary::IntToBytes(int32 Value, TArray<uint8>& Bytes)
{
    ByteConverter::IntToBytes(Value, Bytes);
}

void USimpleByteConversionBPLibrary::BytesToInt(const TArray<uint8>& Bytes, int32& Value)
{
    ByteConverter::BytesToInt(Bytes, Value);
}

void USimpleByteConversionBPLibrary::Int64ToBytes(int64 Value, TArray<uint8>& Bytes)
{
   ByteConverter::Int64ToBytes(Value, Bytes);
}

void USimpleByteConversionBPLibrary::BytesToInt64(const TArray<uint8>& Bytes, int64& Value)
{
    ByteConverter::BytesToInt64(Bytes, Value);
}

// === Float ===
void USimpleByteConversionBPLibrary::FloatToBytes(float Value, TArray<uint8>& Bytes)
{
    ByteConverter::FloatToBytes(Value, Bytes);
}

void USimpleByteConversionBPLibrary::BytesToFloat(const TArray<uint8>& Bytes, float& Value)
{
    ByteConverter::BytesToFloat(Bytes, Value);
}

// === Double ===
void USimpleByteConversionBPLibrary::DoubleToBytes(double Value, TArray<uint8>& Bytes)
{
    ByteConverter::DoubleToBytes(Value, Bytes);
}

void USimpleByteConversionBPLibrary::BytesToDouble(const TArray<uint8>& Bytes, double& Value)
{
    ByteConverter::BytesToDouble(Bytes, Value);
}

// === Bool ===
void USimpleByteConversionBPLibrary::BoolToBytes(bool Value, TArray<uint8>& Bytes)
{
    ByteConverter::BoolToBytes(Value, Bytes);
}

void USimpleByteConversionBPLibrary::BytesToBool(const TArray<uint8>& Bytes, bool& Value)
{
    ByteConverter::BytesToBool(Bytes, Value);
}

// === String ===
void USimpleByteConversionBPLibrary::StringToBytes(const FString& Value, TArray<uint8>& Bytes)
{
    ByteConverter::StringToBytes(Value, Bytes);
}

void USimpleByteConversionBPLibrary::BytesToString(const TArray<uint8>& Bytes, FString& Value)
{
    ByteConverter::BytesToString(Bytes, Value);
}

// === Struct ===
DEFINE_FUNCTION(USimpleByteConversionBPLibrary::execStructToBytes)
{
    Stack.StepCompiledIn<FStructProperty>(nullptr);
    const FStructProperty* StructProperty = CastFieldChecked<FStructProperty>(Stack.MostRecentProperty);
    if (!StructProperty) return;

    void* StructPtr = Stack.MostRecentPropertyAddress;

    P_GET_TARRAY_REF(uint8, Bytes);
    P_FINISH;
    P_NATIVE_BEGIN;
        ByteConverter::StructToBytes(StructPtr, StructProperty->Struct, Bytes);
    P_NATIVE_END;
}

DEFINE_FUNCTION(USimpleByteConversionBPLibrary::execBytesToStruct)
{
    P_GET_TARRAY(uint8, Bytes);

    Stack.StepCompiledIn<FStructProperty>(nullptr);
    const FStructProperty* StructProperty = CastFieldChecked<FStructProperty>(Stack.MostRecentProperty);
    if (!StructProperty) return;

    void* StructPtr = Stack.MostRecentPropertyAddress;

    P_FINISH;
    P_NATIVE_BEGIN;
        ByteConverter::BytesToStruct(Bytes, StructPtr, StructProperty->Struct);
    P_NATIVE_END;
}

DEFINE_FUNCTION(USimpleByteConversionBPLibrary::execStructToJsonString)
{
    Stack.StepCompiledIn<FStructProperty>(nullptr);
    const FStructProperty* StructProperty = CastFieldChecked<FStructProperty>(Stack.MostRecentProperty);
    if (!StructProperty) return;

    const void* StructPtr = Stack.MostRecentPropertyAddress;

    P_GET_PROPERTY_REF(FStrProperty, JsonString);
    P_FINISH;
    P_NATIVE_BEGIN;
        ByteConverter::StructToJsonString(StructPtr, StructProperty->Struct, JsonString);
    P_NATIVE_END;
}

DEFINE_FUNCTION(USimpleByteConversionBPLibrary::execJsonStringToStruct)
{
    P_GET_PROPERTY(FStrProperty, JsonString);

    Stack.StepCompiledIn<FStructProperty>(nullptr);
    const FStructProperty* StructProperty = CastFieldChecked<FStructProperty>(Stack.MostRecentProperty);
    if (!StructProperty) return;

    void* StructPtr = Stack.MostRecentPropertyAddress;

    P_FINISH;
    P_NATIVE_BEGIN;
        ByteConverter::JsonStringToStruct(JsonString, StructPtr, StructProperty->Struct);
    P_NATIVE_END;
}

