#include "Memory.h"

template <typename FunctionType>
inline FunctionType GetVirtualFunction(void* classInstance, int functionIndex)
{
    void** virtualTable = *reinterpret_cast<void***>(classInstance);
    return reinterpret_cast<FunctionType>(virtualTable[functionIndex]);
}

void ProcessEvent(void* objectInstance, void* function, void* parameters)
{
    using EventFunctionType = void(*)(void*, void*, void*);
    constexpr int ProcessEventIndex = 69;
    
    auto ProcessEventFunction = GetVirtualFunction<EventFunctionType>(objectInstance, ProcessEventIndex);
    ProcessEventFunction(objectInstance, function, parameters);
}

template <typename T>
static T StaticFindObject(const char * OrigInName){
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    std::u16string output = converter.from_bytes(OrigInName);
    return reinterpret_cast<T(*)(uintptr_t *, uintptr_t , const char16_t * , bool )>(GameBase + Offset::StaticFindObject)(nullptr,-1,output.c_str(),false);
}

void VirtHook(void** point, void* replace, void** backup, char recover)
{
    void* page_start = (void*)((uintptr_t)point - ((uintptr_t)point % 4096));   
    if (mprotect(page_start, 4096, PROT_READ | PROT_WRITE) == 0) 
    {       
        *backup = *point;
        *point = replace;      
        if (recover) {
            mprotect(page_start, 4096, PROT_READ);
        }
    }
}
