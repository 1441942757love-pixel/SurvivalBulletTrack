#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <sys/inotify.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/configuration.h>
#include <android/native_activity.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>
#include <pthread.h>
#include <iostream>
#include <cmath>
#include <random>
#include <cstdint>
#include <string>
#include <fstream>
#include <thread>
#include <codecvt>
#include <jni.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>
#include <include/imgui/imgui_internal.h>
#include <xorstr.hpp>
#include <UE4_Struct.h>
#include <Font.h>
#include <And64InlineHook.hpp>
#include <Log.h>
#include <JniHelper.h>
#include <MainLooper.cpp>
#include <Offsets.h>
#include <Assets/DrawImage.h>
#include <Assets/safe_icon.h>
#define PI 3.141592653589793238

std::random_device rand_device;
std::mt19937 engine(rand_device());
std::uniform_int_distribution<int> dist(0, 100);
uintptr_t GameBase, tersafe;
uintptr_t GetModule(const char* module_name)
{
    FILE *fp;
    uintptr_t addr = 0;
    char *pch;
    char filename[32];
    char line[1024];
    snprintf( filename, sizeof(filename), xorstr_("/proc/self/maps"));
    fp = fopen( filename, xorstr_("r"));

    if ( fp != NULL )
    {
        while ( fgets( line, sizeof(line), fp ) )
        {
            if ( strstr( line, module_name ) )
            {
                pch = strtok( line, xorstr_("-"));
                addr = strtoul( pch, NULL, 16 );
                if ( addr == 0x8000 )
                    addr = 0;
                break;
            }
        }
        fclose( fp ) ;
    }
    return addr;
}


    
void *my_memmove(void *dst, const void *src, size_t count) {
    if (src == nullptr || dst == nullptr) {
        return nullptr;
    }
    char *tmp_dst = (char *) dst;
    char *tmp_src = (char *) src;
    if (tmp_src == nullptr) {
        return nullptr;
    }
    if (tmp_dst + count < src || tmp_src + count < dst) {
        while (count--)
            *tmp_dst++ = *tmp_src++;
    } else {
        tmp_dst += count - 1;
        tmp_src += count - 1;
        while (count--)
            *tmp_dst-- = *tmp_src--;
    }

    return dst;
}

bool isMemoryTrap(uintptr_t addr)
{
	if (addr < 0x10000000 || addr > 0xFFFFFFFFFF || addr % 4 != 0) {
        return false;
	}
	int pageSize = sysconf(_SC_PAGESIZE); 
    void *pageStart = (void *)((uintptr_t)addr & ~(pageSize - 1)); 
    if (mprotect(pageStart, pageSize, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        return false;
    }
    mprotect(pageStart, pageSize, PROT_READ | PROT_WRITE | PROT_EXEC);
    return true;
}

bool Read(uintptr_t address, void *buffer, int size) {
    if (isMemoryTrap(address)) {
        return my_memmove(buffer, reinterpret_cast<void *>(address), size) != nullptr;
    }
    return false;
}

bool Write(uintptr_t address, void *buffer, int size) {
    if (isMemoryTrap(address)) {
        return my_memmove(reinterpret_cast<void *>(address), buffer, size) != nullptr;
    }
    return false;
}

template <typename T>
T Ram(uintptr_t Addr)
{
    T buf;
    Read(Addr, &buf, sizeof(T));
    return buf;
}

long int GetPointer(long int address) {
    return Ram<long int>(address);
}

int GetDword(long int address) {
    return Ram<int>(address);
}

float GetFloat(long int address) {
    return Ram<float>(address);
}

void ReadName(char* buf,uint64_t offset)
{
    unsigned short buf16[64] = { 0 };
    Read(offset, buf16, 28);
    unsigned short *pTempUTF16 = buf16;
    char *pTempUTF8 = buf;
    char *pUTF8End = pTempUTF8 + 32;
    while (pTempUTF16 < pTempUTF16 + 28)
    {
        if (*pTempUTF16 <= 0x007F && pTempUTF8 + 1 < pUTF8End)
        {
            *pTempUTF8++ = (char) * pTempUTF16;
        }
        else if (*pTempUTF16 >= 0x0080 && *pTempUTF16 <= 0x07FF && pTempUTF8 + 2 < pUTF8End)
        {
            *pTempUTF8++ = (*pTempUTF16 >> 6) | 0xC0;
            *pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
        }
        else if (*pTempUTF16 >= 0x0800 && *pTempUTF16 <= 0xFFFF && pTempUTF8 + 3 < pUTF8End)
        {
            *pTempUTF8++ = (*pTempUTF16 >> 12) | 0xE0;
            *pTempUTF8++ = ((*pTempUTF16 >> 6) & 0x3F) | 0x80;
            *pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
        }
        else
        {
            break;
        }
        pTempUTF16++;
    }
}

float GetDistance(FVector Self, FVector object)
{   
    return sqrtf(pow(Self.X - object.X, 2.0) + pow(Self.Y - object.Y, 2.0) + pow(Self.Z - object.Z, 2.0)) / 100.0f;
}

float CalcBottom(float outLeft_Ankle_Y, float outRight_Ankle_Y, float objScreen_w) 
{
    if (outLeft_Ankle_Y < outRight_Ankle_Y) {
        return outRight_Ankle_Y + objScreen_w / 10;
    } else {
        return outLeft_Ankle_Y + objScreen_w / 10;
    }
}
FRotator ToRotator(FVector local, FVector target) {
    FVector rotation;
    rotation.X = local.X - target.X;
    rotation.Y = local.Y - target.Y;
    rotation.Z = local.Z - target.Z;
 
    FRotator newViewAngle = {0, 0, 0}; 
    float hyp = sqrt(rotation.X * rotation.X + rotation.Y * rotation.Y);
    newViewAngle.X = -atan(rotation.Z / hyp) * (180.f / (float) M_PI);
    newViewAngle.Y = atan(rotation.Y / rotation.X) * (180.f / (float) M_PI);
    newViewAngle.R = (float) 0.f;
 
    if (rotation.X >= 0.f)
        newViewAngle.Y += 180.0f; 
    return newViewAngle;
}

bool WriteAddr(void *addr, void *buffer, size_t length) {
    unsigned long page_size = sysconf(_SC_PAGESIZE);
    unsigned long size = page_size * sizeof(uintptr_t);
    return mprotect((void *) ((uintptr_t) addr - ((uintptr_t) addr % page_size) - page_size), (size_t) size, PROT_EXEC | PROT_READ | PROT_WRITE) == 0 && memcpy(addr, buffer, length) != 0;
}
bool WriteInt(uintptr_t addr,int var) {
    return WriteAddr(reinterpret_cast<void*>(addr),reinterpret_cast<void*>(&var), sizeof(var));
}
bool WriteFloat(uintptr_t addr,float var) {
    return WriteAddr(reinterpret_cast<void*>(addr),reinterpret_cast<void*>(&var), sizeof(var));
}
bool WriteAddress(uintptr_t addr,uintptr_t var) {
    return WriteAddr(reinterpret_cast<void*>(addr),reinterpret_cast<void*>(&var), sizeof(var));
}

char GetByte(long a)
{
    a = a & (long)0xffffffffff;
    if (a > 0x10000000 && a < 0x8000000000)
    {
        return *(char*)(a);
    }
    return 0.0f;  
}