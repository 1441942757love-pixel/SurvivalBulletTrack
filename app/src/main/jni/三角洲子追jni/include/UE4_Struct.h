#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <codecvt>
#include <Log.h>
#include <set>
using namespace std;

enum BoneEnum {
    Head = 31,       // 头
    Chest = 4,       // 胸
    Pelvis = 1,        // 盆骨
    Left_Shoulder = 34, // 左肩膀
    Right_Shoulder = 6, // 右肩膀
    Left_Elbow = 35,    // 左手肘
    Right_Elbow = 7,   // 右手肘
    Left_Wrist = 36,    // 左手腕
    Right_Wrist = 8,   // 右手腕
    Left_Thigh = 58,    // 左大腿
    Right_Thigh = 62,   // 右大腿
    Left_Knee = 59,     // 左膝盖
    Right_Knee = 63,    // 右膝盖
    Left_Ankle = 60,    // 左脚腕
    Right_Ankle = 64    // 右脚腕
};

static int bonelist[][2] = {
    {Head, Chest},
    {Chest, Pelvis},
    
    {Chest, Left_Shoulder},
    {Left_Shoulder, Left_Elbow},
    {Left_Elbow, Left_Wrist},
    
    {Chest, Right_Shoulder},
    {Right_Shoulder, Right_Elbow},
    {Right_Elbow, Right_Wrist},
    
    {Pelvis, Left_Thigh},
    {Left_Thigh, Left_Knee},
    {Left_Knee, Left_Ankle},
    
    {Pelvis, Right_Thigh},
    {Right_Thigh, Right_Knee},
    {Right_Knee, Right_Ankle},
};

static int bonecount = sizeof(bonelist) / sizeof(int[2]);

struct FLinearColor
{
    float    R;                        
    float    G;                                 
    float    B;
    float    A;                  
};

struct FVector {
    float X;
    float Y;
    float Z;
};

struct FVector2D{
    float X;
    float Y;
};

struct FRotator {
    float X;
    float Y;
    float R;
};

struct V4D {
    float X;
    float Y;
    float Z;
    float W;
};

struct FName
{
    union
    {
        struct
        {
            int32_t Number;
            int32_t ComparisonIndex;            
        };
    };
};

template<class T>
struct TArray
{
    friend struct FString;

public:
    inline TArray()
    {
        Data = nullptr;
        Count = Max = 0;
    };

    inline int Num() const
    {
        return Count;
    };

    inline T& operator[](int i)
    {
        return Data[i];
    };

    inline const T& operator[](int i) const
    {
        return Data[i];
    };

    inline bool IsValidIndex(int i) const
    {
        return i < Num();
    }

private:
    T* Data;
    int32_t Count;
    int32_t Max;
};

struct FString : private TArray<unsigned short>
{
    inline FString()
    {
    }

    FString(const std::wstring s)
    {
        Max = Count = !s.empty() ? (s.length() * 2) + 1 : 0;
        if (Count)
        {
            Data = (unsigned short *)(s.data());
        }
    }

    FString(const wchar_t *s) : FString(std::wstring(s)) {
    }

    FString(const wchar_t *s, int len) : FString(std::wstring(s, s + len)) {
    }

    FString(const std::string s)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring ws = converter.from_bytes(s);

        Max = Count = !ws.empty() ? ws.length() + 1 : 0;
        if (Count)
        {
            Data = new unsigned short[Count];
            for (size_t i = 0; i < Count; i++)
            {
                Data[i] = static_cast<unsigned short>(ws[i]);
            }
        }
    }

    FString(const char *s) : FString(std::string(s)) {
    }

    FString(const char *s, int len) : FString(std::string(s, s + len)) {
    }
    inline int Length() const
    {
        return Count;
    }

    inline bool IsValid() const
    {
        return Data != nullptr;
    }

    inline const wchar_t* ToWString() const
    {
        wchar_t *output = new wchar_t[Count + 1];

        for (int i = 0; i < Count; i++) {
            const char16_t uc = Data[i];
            if (uc - 0xd800u >= 2048u) {
                output[i] = uc;
            } else {
                if ((uc & 0xfffffc00) == 0xd800 && (uc & 0xfffffc00) == 0xdc00)
                    output[i] = (uc << 10) + Data[i] - 0x35fdc00;
                else
                    output[i] = L'?';
            }
        }

        output[Count] = 0;
        return output;
    }

    inline const char* ToString() const
    {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
        return convert.to_bytes(std::u16string(Data, Data + Count)).c_str();
    }
};
