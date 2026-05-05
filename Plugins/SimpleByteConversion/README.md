
# 🎯 SimpleByteConversion Plugin for Unreal Engine

A lightweight and Blueprint-friendly plugin for Unreal Engine that enables efficient serialization of native types and structs to and from `TArray<uint8>`. It also provides JSON string conversion support with enhanced precision and naming compatibility.

---

## 📦 Overview

**SimpleByteConversion** enables you to:

- Convert `int32`, `float`, `bool`, `FString`, and other native types to and from byte arrays
- Serialize any Blueprint-exposed `USTRUCT` into:
  - **Byte arrays**
  - **JSON strings** (with high precision and original naming preserved)
- Use all features via Blueprints — **no C++ required**
- Ensure full cross-platform compatibility with UTF-8 encoding

---

## 🔧 Features

✅ **Native Type ↔ Byte Array Conversion**  
Supports bidirectional conversion for:
- `int32`, `int64`
- `float`, `double` (with no precision loss)
- `bool`
- `FString` (**UTF-8 encoded, supports multilingual data**)

✅ **Struct ↔ Byte Array Conversion**
- Compatible with any Blueprint-exposed `USTRUCT`
- Uses Unreal’s native `SerializeBin` for binary-accurate conversion

✅ **Struct ↔ JSON String Conversion**  
Compared to Unreal’s default `FJsonObjectConverter`, this plugin:
- **Preserves original field names** (avoids forced camelCase)
- **Maintains high float precision** (float: 9 digits, double: 17 digits)
- Ideal for human-readable formats, debugging, or network JSON protocols

✅ **Fully Blueprint-Compatible**
- All functions are exposed as Blueprint nodes — no C++ knowledge needed
- Clean category organization for easy use

✅ **Zero Dependencies**
- No third-party libraries required
- Lightweight and easy to integrate

---

## 📘 Documentation

Complete documentation available at:

https://github.com/mengzhishanghun/mengzhishanghun/blob/main/UEPlugins/SimpleByteConversion/README.md

---

## 📂 Dependencies

- No third-party dependencies

---

## 👨‍💻 Author

Author: **mengzhishanghun**  
Email: mengzhishanghun@outlook.com
