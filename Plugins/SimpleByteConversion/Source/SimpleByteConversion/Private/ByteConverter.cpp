// Copyright (c) 2025 mengzhishanghun. All rights reserved.



#include "ByteConverter.h"

#include "JsonObjectConverter.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/UnrealType.h"
#include "Misc/EngineVersionComparison.h"
#include "Internationalization/Regex.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

// === Int ===
void ByteConverter::IntToBytes(int32 Value, TArray<uint8>& OutBytes)
{
    OutBytes.SetNumUninitialized(sizeof(int32));
    FMemory::Memcpy(OutBytes.GetData(), &Value, sizeof(int32));
}

void ByteConverter::BytesToInt(const TArray<uint8>& InBytes, int32& OutValue)
{
    if (InBytes.Num() >= sizeof(int32))
    {
        FMemory::Memcpy(&OutValue, InBytes.GetData(), sizeof(int32));
    }
}

// === Int64 ===
void ByteConverter::Int64ToBytes(int64 Value, TArray<uint8>& OutBytes)
{
    OutBytes.SetNumUninitialized(sizeof(int64));
    FMemory::Memcpy(OutBytes.GetData(), &Value, sizeof(int64));
}

void ByteConverter::BytesToInt64(const TArray<uint8>& InBytes, int64& OutValue)
{
    if (InBytes.Num() >= sizeof(int64))
    {
        FMemory::Memcpy(&OutValue, InBytes.GetData(), sizeof(int64));
    }
}

// === Float ===
void ByteConverter::FloatToBytes(float Value, TArray<uint8>& OutBytes)
{
    OutBytes.SetNumUninitialized(sizeof(float));
    FMemory::Memcpy(OutBytes.GetData(), &Value, sizeof(float));
}

void ByteConverter::BytesToFloat(const TArray<uint8>& InBytes, float& OutValue)
{
    if (InBytes.Num() >= sizeof(float))
    {
        FMemory::Memcpy(&OutValue, InBytes.GetData(), sizeof(float));
    }
}

// === Double ===
void ByteConverter::DoubleToBytes(double Value, TArray<uint8>& OutBytes)
{
    OutBytes.SetNumUninitialized(sizeof(double));
    FMemory::Memcpy(OutBytes.GetData(), &Value, sizeof(double));
}

void ByteConverter::BytesToDouble(const TArray<uint8>& InBytes, double& OutValue)
{
    if (InBytes.Num() >= sizeof(double))
    {
        FMemory::Memcpy(&OutValue, InBytes.GetData(), sizeof(double));
    }
}

// === Bool ===
void ByteConverter::BoolToBytes(bool Value, TArray<uint8>& OutBytes)
{
    OutBytes.SetNumUninitialized(1);
    OutBytes[0] = Value ? 1 : 0;
}

void ByteConverter::BytesToBool(const TArray<uint8>& InBytes, bool& OutValue)
{
    if (InBytes.Num() >= 1)
    {
        OutValue = InBytes[0] != 0;
    }
}

// === String ===
void ByteConverter::StringToBytes(const FString& InString, TArray<uint8>& OutBytes)
{
    FTCHARToUTF8 Converter(*InString);
    const int32 UTF8Length = Converter.Length();

    OutBytes.Reset();
    OutBytes.Append(reinterpret_cast<const uint8*>(Converter.Get()), UTF8Length);
    OutBytes.Add(0); // Null terminator
}

void ByteConverter::BytesToString(const TArray<uint8>& InBytes, FString& OutString)
{
    if (InBytes.Num() == 0)
    {
        OutString.Empty();
        return;
    }

    // 拿到原始数据指针
    const uint8* RawData = InBytes.GetData();
    int32 RawLength = InBytes.Num();

    // 若已有 null terminator，直接转换，避免复制
    if (InBytes.Last() == 0)
    {
        OutString = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(RawData)));
        return;
    }

    // 否则复制 + 添加 null terminator
    TArray<uint8> SafeBuffer;
    SafeBuffer.Append(RawData, RawLength);
    SafeBuffer.Add(0); // 补一个 \0 保证安全

    OutString = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(SafeBuffer.GetData())));
}

// ---------- Struct → Bytes ----------
void ByteConverter::StructToBytes(void* StructPtr,const UStruct* StructType,TArray<uint8>& OutBytes)
{
    if (!StructPtr || !StructType)
    {
        UE_LOG(LogTemp, Warning, TEXT("StructToBytes invalid input"));
        return;
    }

    OutBytes.Reset();
    FMemoryWriter Writer(OutBytes);

    StructType->SerializeBin(Writer, StructPtr);
    UE_LOG(LogTemp, Warning, TEXT("StructToBytes done. ByteSize = %d"), OutBytes.Num());
}

// ---------- Bytes → Struct ----------
void ByteConverter::BytesToStruct(const TArray<uint8>& InBytes,
                                  void* OutStructPtr, const UStruct* StructType)
{
    if (!OutStructPtr || !StructType || InBytes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("BytesToStruct invalid input"));
        return;
    }

    FMemoryReader Reader(InBytes);

    StructType->SerializeBin(Reader, OutStructPtr);
}

// 依赖：Json, JsonUtilities
template <typename CharT>
struct FExactPrintPolicy : TCondensedJsonPrintPolicy<CharT>
{
    static void WriteFloat (FArchive* Stream, float  V)
    {
        FString S = FString::Printf(TEXT("%.9f"), V);
        TCondensedJsonPrintPolicy<CharT>::WriteString(Stream, S);
    }
    static void WriteDouble(FArchive* Stream, double V)
    {
        FString S = FString::Printf(TEXT("%.17g"), V);
        TCondensedJsonPrintPolicy<CharT>::WriteString(Stream, S);
    }
};

FString SanitizeJsonFieldName(const FString& InName)
{
    // 匹配结尾的 "_数字_十六进制" 部分
    const FRegexPattern Pattern(TEXT("_(\\d+)_([A-F0-9]{6,})$"));
    FRegexMatcher Matcher(Pattern, InName);

    if (Matcher.FindNext())
    {
        int32 MatchStart = Matcher.GetMatchBeginning();
        return InName.Left(MatchStart);  // 截取干净部分
    }

    return InName; // 没匹配到则返回原名
}

// ---------- Struct → JsonString ----------
void ByteConverter::StructToJsonString(const void* StructPtr, const UStruct* StructType, FString& OutJsonString)
{
    if(!StructPtr||!StructType) return;
    TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
    for(TFieldIterator<FProperty>It(StructType);It;++It)
    {
        FProperty* P=*It;
        const void* Val=P->ContainerPtrToValuePtr<void>(StructPtr);
        TSharedPtr<FJsonValue> J=FJsonObjectConverter::UPropertyToJsonValue(
#if UE_VERSION_NEWER_THAN(5,4,0)
            P,Val,0,0,nullptr,nullptr,EJsonObjectConversionFlags::None
#else
            P,Val,0,0
#endif
        );

        if (J) Obj->SetField(SanitizeJsonFieldName(P->GetName()),J);          // 保留大小写
    }
    auto Writer=TJsonWriterFactory<TCHAR,FExactPrintPolicy<TCHAR>>::Create(&OutJsonString);
    FJsonSerializer::Serialize(Obj,Writer);           // 高精度输出
}

// ---------- JsonString → Struct ----------
void ByteConverter::JsonStringToStruct(const FString& InJsonString, void* OutStructPtr,
    const UStruct* StructType)
{
    if (!OutStructPtr || !StructType || InJsonString.IsEmpty()) return;
    TSharedPtr<FJsonObject> Obj;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(InJsonString), Obj) || !Obj)
    {
        UE_LOG(LogTemp, Warning, TEXT("JSON parse failed: %s"), *InJsonString);
        return;
    }

    for (TFieldIterator<FProperty> It(StructType); It; ++It)
    {
        FProperty* Prop = *It;
        FString FieldName = SanitizeJsonFieldName(Prop->GetName());
        if (const TSharedPtr<FJsonValue>* J = Obj->Values.Find(FieldName))
        {
            if (!FJsonObjectConverter::JsonValueToUProperty(
                    *J, Prop, Prop->ContainerPtrToValuePtr<void>(OutStructPtr), 0, 0))
            {
                UE_LOG(LogTemp, Warning,
                       TEXT("Key %s found but failed to import"), *FieldName);
            }
        }
        else
        {
            UE_LOG(LogTemp, Verbose,
                   TEXT("Key %s not present in JSON, keep default"), *FieldName);
        }
    }
}
