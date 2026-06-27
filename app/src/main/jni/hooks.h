#ifndef HOOKS_H
#define HOOKS_H

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <exception>

#define HOOK_LOG_TAG "DarkTeam"
#define HOOK_LOGI(...) __android_log_print(ANDROID_LOG_INFO, HOOK_LOG_TAG, __VA_ARGS__)
#define HOOK_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, HOOK_LOG_TAG, __VA_ARGS__)
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <future>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_android.h"
#include "imgui/fonts/GoogleSans.h"
#include "font.h"
#include "Unity/Vector3.hpp"  // 确保包含Vector3定义
#include "Unity/Vector2.hpp"
#include "Unity/Rect.h"
#include "Unity/Quaternion.h"
#include "Unity/Color.h"

#include "ByNameModding/Tools.h"
#include "ByNameModding/fake_dlfcn.h"
#include "ByNameModding/Il2Cpp.h"

#include "Utils.h"
#include "bools.h"
#include "Drawing.h"
#include "Dobby/dobby.h"
#include "timer.h"

#define libName "libil2cpp.so"
#define WEAPON_MUZZLE_OFFSET 0x158

struct Game_Screen {
    static int Width;
    static int Height;
    static int ScreenWidth;
    static int ScreenHeight;
    static bool setup;
};
int Game_Screen::Width = 0;
int Game_Screen::Height = 0;
int Game_Screen::ScreenWidth = 0;
int Game_Screen::ScreenHeight = 0;
bool Game_Screen::setup = false;

// 后台刷新线程（避免GL线程调用Unity API导致内存损坏崩溃）
static std::atomic<bool> g_RefreshRunning{false};
static std::thread g_RefreshThread;

// 全局宏定义
#if defined(__aarch64__)
#define ARM64_CALL __attribute__((pcs("aapcs")))
#define GET_METHOD(assembly, namespace, klass, method, paramCount) \
    (void*)(uintptr_t)Il2CppGetMethodOffset(assembly, namespace, klass, method, paramCount)
#else
#define ARM64_CALL
#define GET_METHOD(assembly, namespace, klass, method, paramCount) \
    (void*)(uintptr_t)Il2CppGetMethodOffset(assembly, namespace, klass, method, paramCount)
#endif

// 全局变量外部声明
extern bool g_Initialized;
extern bool bInitDone;
extern int glWidth;
extern int glHeight;

// 函数指针提前声明
void (*old_input)(void *event, void *exAb, void *exAc);
void hook_input(void *event, void *exAb, void *exAc);
int (*old_getWidth)(ANativeWindow* window);
int hook_getWidth(ANativeWindow* window);
int (*old_getHeight)(ANativeWindow* window);
int hook_getHeight(ANativeWindow* window);
void Egl_hooks();
void (*old_Update)(void * Update, void* method) = nullptr;
void Update(void * Update, void* method);
void ShowMenu();
bool IsTeammate(int playerTeamId);
struct PlayerInfo;
PlayerInfo FindClosestTarget(Vector3 origin);

	// FireBullet Hook（dump: FireBullet(UnityEngine.Vector3 _shootDir)）
    typedef void (ARM64_CALL *FireBullet_t)(void* weapon, Vector3 _shootDir, void* method);
    FireBullet_t old_FireBullet = nullptr;
    FireBullet_t old_NpcFireBullet = nullptr;
    // Bullet Hook
    typedef void (ARM64_CALL *BulletOnEnable_t)(void* bullet, void* method);
    BulletOnEnable_t old_BulletOnEnable = nullptr;
    void Hooked_BulletOnEnable(void* bullet, void* method);
    typedef void (ARM64_CALL *BulletOnCollision_t)(void* bullet, void* collision, void* method);
    BulletOnCollision_t old_BulletOnCollisionEnter = nullptr;
    void Hooked_BulletOnCollisionEnter(void* bullet, void* collision, void* method);
void initializeFunctionPointers();
void restartDrawing();
void ModifyWeapon(void* weapon);
// 【dump更新】MoveBehaviour/PlayerHealthManager类不存在，以下函数暂时保留声明但实现已禁用
void ModifyMoveSpeed(void* moveBehaviour);
void ModifyPlayerHealth(void* healthManager);
void ShowDrawSubMenu();
void ShowModSubMenu();
void GetPlayerHealthInfo(void* player, bool isNpc, float& currentHealth, float& maxHealth);
int GetPlayerTeamId(void* player);
int SafeReadTeamHash(void* obj, int teamOffset);
void DrawHealthBar(ImDrawList* draw, const Rect& playerRect, float currentHealth, float maxHealth, float distance, int screenWidth);
void DrawESP(ImDrawList *draw, int screenWidth, int screenHeight);
EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
void *hack_thread(void *);

#define SAFE_CALL(instance, func, ...) \
    if ((instance) && m_CachedPtr(instance) && Object_IsNativeObjectAlive(instance, nullptr)) { \
        func(instance, ##__VA_ARGS__); \
    }

// 数据结构定义
struct PlayerInfo {
    void* playerPtr;
    std::string name;
    Vector3 position;       // Transform根位置
    Vector3 headPos;        // 头部/胸部位置（chestBone，用于子弹追踪）
    float currentHealth;
    float maxHealth;
    int team;
    float distance;
    bool isNpc = false;
    bool hasLastPosition = false;
    Vector3 lastPosition = Vector3::Zero();
    Vector3 lastHeadPos = Vector3::Zero();
    Vector3 velocity = Vector3::Zero();
    // 【修复】预计算屏幕坐标（后台线程计算，GL线程只读，避免GL线程调用Unity API崩溃）
    ImVec2 screenFootPos = ImVec2(0,0);
    ImVec2 screenHeadPos = ImVec2(0,0);
    float screenDepth = -1.0f;   // Z值，<=0表示屏幕后方
};
struct BoneInfo {
    void* transform;
    Vector3 position;
    std::string name;
    ImVec2 screenPos = ImVec2(0,0);    // 预计算屏幕坐标
    float screenDepth = -1.0f;
};

// 预计算的相机数据（后台线程刷新）
static Vector3 g_CameraPosition = Vector3::Zero();
static std::mutex g_CameraMutex;

// 全局数据（带互斥锁）
std::mutex drawMutex; 
std::atomic<bool> g_RestartDrawing(false); 
std::mutex playersListMutex; 
std::map<int, BoneInfo> g_BoneInfos;  
std::mutex g_BoneInfoMutex;
std::vector<PlayerInfo> g_PlayersInfo; 
std::mutex g_PlayersInfoMutex;

// 线程池类
class ThreadPool {
public:
    ThreadPool(size_t numThreads) {
        start(numThreads);
    }

    ~ThreadPool() {
        stop();
    }

    template<class T>
    auto enqueue(T task) -> std::future<decltype(task())> {
        auto wrapper = std::make_shared<std::packaged_task<decltype(task())()>>(std::move(task));
        {
            std::unique_lock<std::mutex> lock{eventMutex};
            tasks.emplace([=] {
                (*wrapper)();
            });
        }
        eventVar.notify_one();
        return wrapper->get_future();
    }

private:
    std::vector<std::thread> threads;
    std::condition_variable eventVar;
    std::mutex eventMutex;
    bool stopping = false;
    std::queue<std::function<void()>> tasks;

    void start(size_t numThreads) {
        for (auto i = 0u; i < numThreads; ++i) {
            threads.emplace_back([=] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock{eventMutex};
                        eventVar.wait(lock, [=] { return stopping || !tasks.empty(); });
                        if (stopping && tasks.empty()) break;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    try { task(); } catch (...) {}
                }
            });
        }
    }

    void stop() noexcept {
        {
            std::unique_lock<std::mutex> lock{eventMutex};
            stopping = true;
        }
        eventVar.notify_all();
        for (auto &thread : threads) thread.join();
    }
};


// 终极防崩溃：验证指针是否在合法的 ARM64 堆内存地址空间内
bool IsValidHeapPointer(void* ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    // 过滤掉空指针、过小的地址，以及浮点数伪装的高位异常地址 (如 0x44d7a...)
    return addr > 0x1000000000 && addr < 0x00007FFFFFFFFFFF; 
}

bool IsLikelyIl2CppObjectPointer(void* ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    if (addr == 0) return false;
    // 常见 arm64 用户态/托管堆地址；过滤 0x3500000001 / 0x0030003300000002 这类打包值伪指针。
    return (addr >= 0x6000000000ULL && addr < 0x8000000000ULL) ||
           (addr >= 0xB000000000000000ULL && addr < 0xC000000000000000ULL);
}

bool IsLikelyNativePointer(void* ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    if (addr == 0) return false;
    // UnityEngine.Object.m_CachedPtr 是 native 指针，通常在低 canonical 用户态地址，不是托管堆地址。
    return addr >= 0x10000ULL && addr < 0x0000800000000000ULL;
}

// 终极防崩溃：利用 Linux 内核级安全读取，避免触发硬件 SEGV
bool SafeRead64(void* addr, uintptr_t* out_val) {
    if (!addr) return false;
    int pipefd[2];
    if (pipe(pipefd) == -1) return false;
    
    // 尝试将目标地址的内容写入管道
    // 如果 addr 所在内存页已被 GC 释放，内核会返回 -1 (EFAULT)，绝对不会触发应用层崩溃！
    ssize_t ret = write(pipefd[1], addr, sizeof(uintptr_t));
    if (ret == sizeof(uintptr_t)) {
        read(pipefd[0], out_val, sizeof(uintptr_t));
        close(pipefd[0]);
        close(pipefd[1]);
        return true;
    }
    
    close(pipefd[0]);
    close(pipefd[1]);
    return false;
}

bool SafeRead32(void* addr, uint32_t* out_val) {
    if (!out_val) return false;

    uintptr_t raw = 0;
    if (!SafeRead64(addr, &raw)) return false;

    *out_val = (uint32_t)(raw & 0xFFFFFFFFu);
    return true;
}

bool SafeReadInt32(void* addr, int* out_val) {
    if (!out_val) return false;

    uint32_t raw = 0;
    if (!SafeRead32(addr, &raw)) return false;

    *out_val = (int)raw;
    return true;
}

bool SafeReadFloat(void* addr, float* out_val) {
    if (!out_val) return false;

    uint32_t raw = 0;
    if (!SafeRead32(addr, &raw)) return false;

    float value = 0.0f;
    memcpy(&value, &raw, sizeof(float));
    if (!std::isfinite(value)) return false;

    *out_val = value;
    return true;
}

bool SafeReadVector3(void* addr, Vector3* out_val) {
    if (!out_val) return false;

    uintptr_t xyRaw = 0;
    uintptr_t zRaw = 0;
    if (!SafeRead64(addr, &xyRaw)) return false;
    if (!SafeRead64((void*)((uintptr_t)addr + 0x8), &zRaw)) return false;

    uint32_t xBits = (uint32_t)(xyRaw & 0xFFFFFFFFu);
    uint32_t yBits = (uint32_t)((xyRaw >> 32) & 0xFFFFFFFFu);
    uint32_t zBits = (uint32_t)(zRaw & 0xFFFFFFFFu);

    Vector3 value;
    memcpy(&value.X, &xBits, sizeof(float));
    memcpy(&value.Y, &yBits, sizeof(float));
    memcpy(&value.Z, &zBits, sizeof(float));
    if (!std::isfinite(value.X) || !std::isfinite(value.Y) || !std::isfinite(value.Z)) return false;

    *out_val = value;
    return true;
}

bool SafeReadObjectField(void* obj, uintptr_t offset, void** out_ptr) {
    if (!obj || !out_ptr) return false;

    uintptr_t rawPtr = 0;
    if (!SafeRead64((void*)((uintptr_t)obj + offset), &rawPtr)) return false;

    void* ptr = (void*)rawPtr;
    if (!IsLikelyIl2CppObjectPointer(ptr)) return false;

    uintptr_t probe = 0;
    if (!SafeRead64(ptr, &probe)) return false;

    *out_ptr = ptr;
    return true;
}

bool SafeWriteInt32Field(void* obj, uintptr_t offset, int value) {
    if (!obj) return false;

    void* addr = (void*)((uintptr_t)obj + offset);
    uintptr_t probe = 0;
    if (!SafeRead64(addr, &probe)) return false;

    *(int*)addr = value;
    return true;
}

bool SafeWriteFloatField(void* obj, uintptr_t offset, float value) {
    if (!obj || !std::isfinite(value)) return false;

    void* addr = (void*)((uintptr_t)obj + offset);
    uintptr_t probe = 0;
    if (!SafeRead64(addr, &probe)) return false;

    *(float*)addr = value;
    return true;
}

bool SafeWriteBoolField(void* obj, uintptr_t offset, bool value) {
    if (!obj) return false;

    void* addr = (void*)((uintptr_t)obj + offset);
    uintptr_t probe = 0;
    if (!SafeRead64(addr, &probe)) return false;

    *(uint8_t*)addr = value ? 1 : 0;
    return true;
}


ThreadPool pool(5);
void* g_GhostAIObject = nullptr;
void* g_AnimatorComponent = nullptr;
std::mutex g_GhostAIMutex;

// 功能开关变量
bool g_WeaponModEnabled = false;
bool g_InfiniteAmmo = false;
bool g_NoRecoil = false;
bool g_HighDamage = false;
bool g_NoFireCooldown = false;
float g_BulletSpeedValue = 500.0f;
int g_CanScopeValue = 50;
const int kBulletTrackSpeed = 1500;
const int kBulletTrackRange = 5000;

bool g_MoveSpeedModEnabled = false;
float g_WalkSpeedValue = 10.0f;   
float g_RunSpeedValue = 10.0f;    

bool g_PlayerHealthModEnabled = false;
float g_PlayerHealthValue = 8888.0f;
float g_HelmetHealthValue = 9999.0f;
float g_VestHealthValue = 9999.0f;
float g_HealthSliderValue = 9999.0f;

bool g_BulletTrackEnabled = false;
float g_BulletTrackMaxDist = 1200.0f;
bool g_BulletTrackLockHead = true;
int g_BulletTrackMode = 0;         // 0=最近距离, 1=准星指向
bool g_ShowTrackRange = true;
float g_TrackCircleRadius = 180.0f;
bool isTeamFilter = true;

std::vector<void*> g_MoveBehaviourObjects;
std::mutex g_MoveBehaviourMutex;
std::vector<void*> g_PlayerHealthManagers;
std::mutex g_PlayerHealthMutex;
std::vector<void*> g_WeaponObjects;
std::mutex g_WeaponMutex;
std::atomic<bool> g_SceneAlive(false);
std::atomic<long long> g_LastSceneAliveMs(0);
std::atomic<uintptr_t> g_LastMainCameraPtr(0);
std::atomic<int> g_LocalTeamHash(-1);

long long NowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

void ClearSceneCaches() {
    {
        std::lock_guard<std::mutex> lock(g_PlayersInfoMutex);
        g_PlayersInfo.clear();
    }
    {
        std::lock_guard<std::mutex> lock(g_BoneInfoMutex);
        g_BoneInfos.clear();
    }
    {
        std::lock_guard<std::mutex> lock(g_WeaponMutex);
        g_WeaponObjects.clear();
    }
    {
        std::lock_guard<std::mutex> lock(g_CameraMutex);
        g_CameraPosition = Vector3::Zero();
    }
    g_LocalTeamHash.store(-1, std::memory_order_release);
    g_LastMainCameraPtr.store(0, std::memory_order_release);
    g_SceneAlive.store(false, std::memory_order_release);
}

void MarkSceneAlive() {
    g_LastSceneAliveMs.store(NowMs(), std::memory_order_release);
    g_SceneAlive.store(true, std::memory_order_release);
}

bool IsSceneRenderAlive() {
    if (!g_SceneAlive.load(std::memory_order_acquire)) return false;

    long long lastAlive = g_LastSceneAliveMs.load(std::memory_order_acquire);
    if (lastAlive <= 0 || NowMs() - lastAlive > 750) {
        ClearSceneCaches();
        return false;
    }

    return true;
}

// 函数指针类型定义
#if defined(__aarch64__)
typedef Vector3 (ARM64_CALL *Camera_WorldToScreen_t)(void *camera, Vector3 position, void* method);
typedef void* (ARM64_CALL *GetTypeFunc_t)(MonoString*, void* method);
typedef MonoArray<void**>* (ARM64_CALL *FindObjectsFunc_t)(void*, void* method);
#else
typedef Vector3 (*Camera_WorldToScreen_t)(void *camera, Vector3 position, void* method);
typedef void* (*GetTypeFunc_t)(MonoString*, void* method);
typedef MonoArray<void**>* (*FindObjectsFunc_t)(void*, void* method);
#endif

typedef MonoString* (*Object_get_name_t)(void*, void* method);
Object_get_name_t Object_get_name = nullptr;
void (ARM64_CALL *Screen_SetResolution)(int width, int height, bool fullscreen, void* method);
// 【dump更新】CameraManager类不存在，使用Camera.get_main即可
void* (ARM64_CALL *Camera_get_main)(void* method);
void* (ARM64_CALL *Component_get_transform)(void *instance, void* method);
Vector3 (ARM64_CALL *Transform_get_position)(void *instance, void* method);
void (ARM64_CALL *Transform_set_position)(void *instance, Vector3 pos, void* method);
void* (ARM64_CALL *GameObject_get_transform)(void *instance, void* method);
Camera_WorldToScreen_t Camera_WorldToScreen = nullptr;
FindObjectsFunc_t Object_FindObjectsOfType = nullptr;
GetTypeFunc_t Type_GetTypeName = nullptr;
MonoArray<void**>* (ARM64_CALL *GameObject_FindGameObjectsWithTag)(MonoString*, void* method);
bool* (ARM64_CALL *Object_IsNativeObjectAlive)(void *object, void* method);
void* (ARM64_CALL *GameObject_get_gameObject)(void *original, void* method);
void* (ARM64_CALL *Object_Destroy)(void* obj, void* method);
Vector3 (ARM64_CALL *Physics_get_gravity)(void* method);

// 【dump更新】Player类不存在，health是NpcControl/PlayerControl的int32字段
void* (ARM64_CALL *Component_GetComponent)(void* component, void* type, void* method);
void* (ARM64_CALL *Component_GetComponentInParent)(void* component, void* type, void* method);
void* (ARM64_CALL *Collision_get_collider)(void* collision, void* method);
typedef void* (ARM64_CALL *GetBoneTransform_t)(void* animator, int boneId, void* method);
GetBoneTransform_t Animator_GetBoneTransform = nullptr;
// 【dump更新】Player.get_team不存在，team改为String字段通过偏移读取

bool m_CachedPtr(void *unity_obj) {
    if (!unity_obj) return false;
    if (!IsLikelyIl2CppObjectPointer(unity_obj)) return false;

    uintptr_t cachedPtr = 0;
    #if defined(__aarch64__)
    if (!SafeRead64((void*)((uintptr_t)unity_obj + 0x10), &cachedPtr)) return false;
    #else
    if (!SafeRead64((void*)((uintptr_t)unity_obj + 0x8), &cachedPtr)) return false;
    #endif

    if (!IsLikelyNativePointer((void*)cachedPtr)) return false;

    // 【核心修复】：防止 GC 复用导致的“伪造原生指针”崩溃！
    // 0x3000000039 这种垃圾值虽然通过了大小校验，但它是一块无法读取的无效内存。
    // 使用 SafeRead64 探路，如果是假指针直接返回 false！
    uintptr_t dummy = 0;
    if (!SafeRead64((void*)cachedPtr, &dummy)) return false;

    return Object_IsNativeObjectAlive && Object_IsNativeObjectAlive(unity_obj, nullptr);
}


uintptr_t GetAnimatorOffset(bool isNpc) {
    return isNpc ? 0xB8 : 0x100; // NpcControl._anim / PlayerControl._anim
}

uintptr_t GetHealthOffset(bool isNpc) {
    return isNpc ? 0x48 : 0x38; // NpcControl.health / PlayerControl.health
}

uintptr_t GetTeamOffset(bool isNpc) {
    return isNpc ? 0x38 : 0x50; // NpcControl.team / PlayerControl.team
}

bool IsLocalPlayerControl(void* player) {
    if (!m_CachedPtr(player)) return false;

    void* localCam = nullptr;
    // PlayerControl.localCam = 0x240
    return SafeReadObjectField(player, 0x240, &localCam) && m_CachedPtr(localCam);
}

void* GetAnimatorFromCharacter(void* character, bool isNpc) {
    if (!m_CachedPtr(character)) return nullptr;

    void* animator = nullptr;
    if (SafeReadObjectField(character, GetAnimatorOffset(isNpc), &animator) &&
        m_CachedPtr(animator)) {
        return animator;
    }

    return nullptr;
}

// 安全获取武器开火位置（dump: Weapon.firePoint 字段位于 0x108）
Vector3 Transform_getPosition(void *transform);
Vector3 GetWeaponFirePoint(void* weapon) {
    if (!m_CachedPtr(weapon)) {
        std::lock_guard<std::mutex> camLock(g_CameraMutex);
        return g_CameraPosition;
    }

    void* firePoint = nullptr;
    if (SafeReadObjectField(weapon, 0x108, &firePoint) && m_CachedPtr(firePoint)) {
        Vector3 pos = Transform_get_position(firePoint, nullptr);
        if (pos != Vector3::Zero()) return pos;
    }

    std::lock_guard<std::mutex> camLock(g_CameraMutex);
    return g_CameraPosition; 
}

Vector3 Transform_getPosition(void *transform) {  
    if (!m_CachedPtr(transform)) return Vector3::Zero();
    void* componentTransform = Component_get_transform(transform, nullptr);
    if (!m_CachedPtr(componentTransform)) return Vector3::Zero();
    return Transform_get_position(componentTransform, nullptr);
} 

Vector3 GameObject_getPosition(void *transform) {  
    if (!m_CachedPtr(transform)) return Vector3::Zero();
    void* gameObjectTransform = GameObject_get_transform(transform, nullptr);
    if (!m_CachedPtr(gameObjectTransform)) return Vector3::Zero();
    return Transform_get_position(gameObjectTransform, nullptr);
}

bool IsFiniteVector(const Vector3& v) {
    return std::isfinite(v.X) && std::isfinite(v.Y) && std::isfinite(v.Z);
}

Vector3 ClampVectorMagnitude(Vector3 v, float maxMagnitude) {
    float mag = Vector3::Magnitude(v);
    if (mag <= maxMagnitude || mag <= 0.001f) return v;
    return v * (maxMagnitude / mag);
}

void RestoreMotionCache(PlayerInfo& info, const std::vector<PlayerInfo>& previousPlayers) {
    for (const auto& oldInfo : previousPlayers) {
        if (oldInfo.playerPtr != info.playerPtr) continue;
        info.position = oldInfo.position;
        info.headPos = oldInfo.headPos;
        info.lastPosition = oldInfo.lastPosition;
        info.lastHeadPos = oldInfo.lastHeadPos;
        info.velocity = oldInfo.velocity;
        info.hasLastPosition = oldInfo.hasLastPosition;
        return;
    }
}

Vector3 ReadCachedPlayerVelocity(const PlayerInfo& info) {
    if (!m_CachedPtr(info.playerPtr)) return Vector3::Zero();

    uintptr_t velOffset = info.isNpc ? 0xD0 : 0x208; // NpcControl.myRigidVel / PlayerControl.myRigidVel
    Vector3 vel = Vector3::Zero();
    if (!SafeReadVector3((void*)((uintptr_t)info.playerPtr + velOffset), &vel)) return Vector3::Zero();

    float speed = Vector3::Magnitude(vel);
    if (speed > 0.01f && speed < 80.0f) return vel;

    return Vector3::Zero();
}

float GetTrackingBulletSpeed(void* weapon) {
    const float trackSpeed = (float)kBulletTrackSpeed;
    float speed = trackSpeed;

    if (!m_CachedPtr(weapon)) return speed;

    int currentSpeed = 0;
    if (SafeReadInt32((void*)((uintptr_t)weapon + 0x3C), &currentSpeed)) { // Weapon.bulletSpeed
        if (currentSpeed < kBulletTrackSpeed) {
            SafeWriteInt32Field(weapon, 0x3C, kBulletTrackSpeed);
            currentSpeed = kBulletTrackSpeed;
        }
        if (currentSpeed > 0 && currentSpeed < 20000) speed = (float)currentSpeed;
    }

    return speed;
}

void ApplyTrackingWeaponStats(void* weapon) {
    if (!m_CachedPtr(weapon)) return;

    int currentSpeed = 0;
    if (SafeReadInt32((void*)((uintptr_t)weapon + 0x3C), &currentSpeed) &&
        (currentSpeed <= 0 || currentSpeed < kBulletTrackSpeed)) {
        SafeWriteInt32Field(weapon, 0x3C, kBulletTrackSpeed);
    }

    int currentRange = 0;
    if (SafeReadInt32((void*)((uintptr_t)weapon + 0x40), &currentRange) &&
        (currentRange <= 0 || currentRange < kBulletTrackRange)) {
        SafeWriteInt32Field(weapon, 0x40, kBulletTrackRange);
    }
}

void ApplyTrackingBulletStats(void* bullet) {
    if (!m_CachedPtr(bullet)) return;

    float currentRange = 0.0f;
    if (SafeReadFloat((void*)((uintptr_t)bullet + 0x1C), &currentRange) &&
        (currentRange <= 0.0f || currentRange < (float)kBulletTrackRange)) {
        SafeWriteFloatField(bullet, 0x1C, (float)kBulletTrackRange);
    }

    float traveledDist = 0.0f;
    if (SafeReadFloat((void*)((uintptr_t)bullet + 0x20), &traveledDist) && traveledDist > 0.0f) {
        SafeWriteFloatField(bullet, 0x20, 0.0f);
    }
}

float SolveInterceptTime(Vector3 firePoint, Vector3 targetPos, Vector3 targetVelocity, float bulletSpeed) {
    if (bulletSpeed <= 1.0f) return 0.0f;

    Vector3 toTarget = targetPos - firePoint;
    float c = Vector3::Dot(toTarget, toTarget);
    if (c <= 0.001f) return 0.0f;

    float a = Vector3::Dot(targetVelocity, targetVelocity) - bulletSpeed * bulletSpeed;
    float b = 2.0f * Vector3::Dot(toTarget, targetVelocity);
    float t = 0.0f;

    if (fabsf(a) < 0.001f) {
        if (fabsf(b) > 0.001f) t = -c / b;
    } else {
        float discriminant = b * b - 4.0f * a * c;
        if (discriminant >= 0.0f) {
            float root = sqrtf(discriminant);
            float t1 = (-b - root) / (2.0f * a);
            float t2 = (-b + root) / (2.0f * a);
            if (t1 > 0.0f && t2 > 0.0f) t = fminf(t1, t2);
            else if (t1 > 0.0f) t = t1;
            else if (t2 > 0.0f) t = t2;
        }
    }

    float directTime = sqrtf(c) / bulletSpeed;
    if (t <= 0.0f) t = directTime;

    float maxPredictTime = fminf(fmaxf(directTime + 0.35f, 1.25f), 4.0f);
    return fminf(fmaxf(t, 0.0f), maxPredictTime);
}

Vector3 GetPhysicsGravityVector() {
    Vector3 gravity = Vector3(0.0f, -9.81f, 0.0f);

    try {
        if (Physics_get_gravity) {
            Vector3 runtimeGravity = Physics_get_gravity(nullptr);
            if (IsFiniteVector(runtimeGravity) && Vector3::Magnitude(runtimeGravity) > 0.001f) {
                gravity = runtimeGravity;
            }
        }
    } catch (...) {}

    return gravity;
}

Vector3 PredictAimPoint(Vector3 firePoint, const PlayerInfo& target, float bulletSpeed) {
    Vector3 targetPos = target.headPos;
    if (!g_BulletTrackLockHead) targetPos.Y -= 0.5f;

    Vector3 targetVelocity = ClampVectorMagnitude(target.velocity, 60.0f);
    float interceptTime = SolveInterceptTime(firePoint, targetPos, targetVelocity, bulletSpeed);
    Vector3 predicted = targetPos + targetVelocity * interceptTime;
    Vector3 gravity = GetPhysicsGravityVector();
    predicted = predicted - gravity * (0.5f * interceptTime * interceptTime);

    if (!IsFiniteVector(predicted)) return targetPos;
    return predicted;
}

void EnableAllESP() {
    isESP = true;
    isESPLine = true;
    isESPBox = true;
    isESPDistance = true;
    isESPObjectsCount = true;
    isESPName = true;
    g_ShowBones = true;
    isESPHealth = true;
    isTeamFilter = true;
}

void DisableAllESP() {
    isESP = false;
    isESPLine = false;
    isESPBox = false;
    isESPDistance = false;
    isESPObjectsCount = false;
    isESPName = false;
    g_ShowBones = false;
    isESPHealth = false;
    isTeamFilter = false;
}

// 武器修改
void ModifyWeapon(void* weapon) {
    if (!m_CachedPtr(weapon)) return;
    
    if (g_InfiniteAmmo) {
        SafeWriteInt32Field(weapon, 0xC4, 9999); // Weapon.ammo
        SafeWriteInt32Field(weapon, 0xC8, 9999); // Weapon.mag
    }

    if (g_NoRecoil) {
        SafeWriteFloatField(weapon, 0xA8, 0.0f); // Weapon.forces.X
        SafeWriteFloatField(weapon, 0xAC, 0.0f); // Weapon.forces.Y
        SafeWriteFloatField(weapon, 0xB0, 0.0f); // Weapon.forces.Z

        void* owner = nullptr;
        if (SafeReadObjectField(weapon, 0x148, &owner) && m_CachedPtr(owner)) { // Weapon._player
            SafeWriteFloatField(owner, 0x1E4, 0.0f); // PlayerControl.weaponKick.X
            SafeWriteFloatField(owner, 0x1E8, 0.0f); // PlayerControl.weaponKick.Y
            SafeWriteFloatField(owner, 0x1EC, 0.0f); // PlayerControl.weaponKick.Z
            SafeWriteFloatField(owner, 0x1F0, 0.0f); // PlayerControl.swayZ
            SafeWriteFloatField(owner, 0x1F8, 0.0f); // PlayerControl.recoilSpring.X
            SafeWriteFloatField(owner, 0x1FC, 0.0f); // PlayerControl.recoilSpring.Y
            SafeWriteFloatField(owner, 0x2A8, 0.0f); // PlayerControl.camShake
        }
    }

    if (g_HighDamage) {
        SafeWriteInt32Field(weapon, 0x44, 999); // Weapon.damage
    }

    if (g_NoFireCooldown) {
        SafeWriteFloatField(weapon, 0x88, 0.0f); // Weapon.overHeat
        SafeWriteBoolField(weapon, 0x8C, false); // Weapon.overHeated
        SafeWriteFloatField(weapon, 0xB8, 0.0f); // Weapon.fireCoolDown
    }

    SafeWriteInt32Field(weapon, 0x3C, (int)g_BulletSpeedValue); // Weapon.bulletSpeed
}

// 移动速度修改
void ModifyMoveSpeed(void* moveBehaviour) {
    if (!m_CachedPtr(moveBehaviour)) return;
    
    try {
        float* walkSpeedPtr = (float*)((uintptr_t)moveBehaviour + 0x30);
        if (walkSpeedPtr) *walkSpeedPtr = g_WalkSpeedValue;
        float* runSpeedPtr = (float*)((uintptr_t)moveBehaviour + 0x34);
        if (runSpeedPtr) *runSpeedPtr = g_RunSpeedValue;
    } catch (...) {}
}

// 玩家健康修改
void ModifyPlayerHealth(void* healthManager) {
    if (!m_CachedPtr(healthManager)) return;
    
    try {
        float* healthPtr = (float*)((uintptr_t)healthManager + 0x1C);
        if (healthPtr) *healthPtr = g_PlayerHealthValue;
        float* helmetHealthPtr = (float*)((uintptr_t)healthManager + 0x34);
        if (helmetHealthPtr) *helmetHealthPtr = g_HelmetHealthValue;
        float* vestHealthPtr = (float*)((uintptr_t)healthManager + 0x3C);
        if (vestHealthPtr) *vestHealthPtr = g_VestHealthValue;
        float* healthSliderPtr = (float*)((uintptr_t)healthManager + 0x88);
        if (healthSliderPtr) *healthSliderPtr = g_HealthSliderValue;
    } catch (...) {}
}

// 安全找最近目标（3D世界距离）
PlayerInfo FindClosestTarget(Vector3 origin) {
    PlayerInfo closest;
    closest.distance = g_BulletTrackMaxDist + 1.0f;
    closest.playerPtr = nullptr;
    closest.currentHealth = 0.0f;
    closest.position = Vector3::Zero();
    closest.headPos = Vector3::Zero();

    std::lock_guard<std::mutex> lock(g_PlayersInfoMutex);
    for (const auto& info : g_PlayersInfo) {
        if (!info.playerPtr || IsTeammate(info.team)) {
            continue;
        }
        Vector3 tp = (info.headPos != Vector3::Zero()) ? info.headPos : (info.position + Vector3(0, 1.6f, 0));
        float dist = Vector3::Distance(origin, tp);
        if (dist < closest.distance && dist <= g_BulletTrackMaxDist) {
            closest = info;
            closest.distance = dist;
            closest.headPos = tp;
        }
    }
    return closest;
}


// 准星指向目标查找（纯数据计算，0 崩溃风险）
PlayerInfo FindCrosshairTarget() {
    PlayerInfo closest;
    closest.distance = 999999.0f;
    closest.playerPtr = nullptr;
    closest.headPos = Vector3::Zero();

    std::lock_guard<std::mutex> lock(g_PlayersInfoMutex);
    ImVec2 screenCenter(Game_Screen::Width / 2.0f, Game_Screen::Height / 2.0f);

    for (const auto& info : g_PlayersInfo) {
        if (!info.playerPtr || IsTeammate(info.team)) continue;
        
        // 如果不在屏幕内（没有预计算深度），直接跳过
        if (info.screenDepth <= 0.f) continue;
        
        // 将预计算坐标换算为从屏幕上方计算的 Y 轴（与 ESP 绘制逻辑一致）
        ImVec2 screenPos(info.screenHeadPos.x, Game_Screen::Height - info.screenHeadPos.y);
        
        float screenDist = sqrtf((screenPos.x - screenCenter.x) * (screenPos.x - screenCenter.x) +
                                  (screenPos.y - screenCenter.y) * (screenPos.y - screenCenter.y));
        if (screenDist > g_TrackCircleRadius) continue;
        if (screenDist < closest.distance) {
            closest = info;
            closest.distance = screenDist;
        }
    }
    return closest;
}

// 子弹追踪Hook（dump: FireBullet(Vector3 _shootDir)，按值接收后显式调用原函数）
void Hooked_FireBullet(void* weapon, Vector3 _shootDir, void* method) {
    Vector3 finalShootDir = _shootDir;

    try {
        if (g_BulletTrackEnabled && m_CachedPtr(weapon)) {
            ApplyTrackingWeaponStats(weapon);
            Vector3 firePoint = GetWeaponFirePoint(weapon);

            if (firePoint != Vector3::Zero()) {
                PlayerInfo target;
                if (g_BulletTrackMode == 1) {
                    target = FindCrosshairTarget();
                } else {
                    target = FindClosestTarget(firePoint);
                }

                if (target.playerPtr && target.headPos != Vector3::Zero()) {
                    float bulletSpeed = GetTrackingBulletSpeed(weapon);
                    Vector3 aimPoint = PredictAimPoint(firePoint, target, bulletSpeed);
                    Vector3 dir = aimPoint - firePoint;

                    if (Vector3::Magnitude(dir) > 0.001f && IsFiniteVector(dir)) {
                        finalShootDir = Vector3::Normalized(dir);
                    }
                }
            }
        }
    } catch (...) {}

    if (old_FireBullet) old_FireBullet(weapon, finalShootDir, method);
}

// NpcControl.FireBullet Hook（NPC开火直接放行）
void Hooked_NpcFireBullet(void* npc, Vector3 _shootDir, void* method) {
    if (old_NpcFireBullet) old_NpcFireBullet(npc, _shootDir, method);
}

// Bullet.OnEnable Hook（不打Unity API — Vector3参数同样是寄存器问题）
void Hooked_BulletOnEnable(void* bullet, void* method) {
    if (old_BulletOnEnable) old_BulletOnEnable(bullet, method);

    if (g_BulletTrackEnabled) {
        ApplyTrackingBulletStats(bullet);
    }
}

void* FindCharacterFromCollision(void* collision, bool* outIsNpc) {
    if (!collision || !Collision_get_collider) return nullptr;
    if (!IsLikelyIl2CppObjectPointer(collision)) return nullptr;
    if (outIsNpc) *outIsNpc = false;

    try {
        void* collider = Collision_get_collider(collision, nullptr);
        if (!m_CachedPtr(collider)) return nullptr;

        static void* npcType = nullptr;
        static void* playerType = nullptr;
        if (!npcType) npcType = Type_GetTypeName(Il2CppString::CreateMonoString("NpcControl,Assembly-CSharp.dll"), nullptr);
        if (!playerType) playerType = Type_GetTypeName(Il2CppString::CreateMonoString("PlayerControl,Assembly-CSharp.dll"), nullptr);

        if (npcType) {
            void* npc = Component_GetComponentInParent ?
                Component_GetComponentInParent(collider, npcType, nullptr) :
                nullptr;
            if (m_CachedPtr(npc)) {
                if (outIsNpc) *outIsNpc = true;
                return npc;
            }
        }

        if (playerType) {
            void* player = Component_GetComponentInParent ?
                Component_GetComponentInParent(collider, playerType, nullptr) :
                nullptr;
            if (m_CachedPtr(player)) return player;
        }
    } catch (...) {}

    return nullptr;
}

void Hooked_BulletOnCollisionEnter(void* bullet, void* collision, void* method) {
    if (g_BulletTrackEnabled) {
        bool hitIsNpc = false;
        void* hitCharacter = FindCharacterFromCollision(collision, &hitIsNpc);
        
        // 只有在明确打中实体，且该实体是队友时，才拦截判定（实现子弹穿透队友）
        if (hitCharacter) {
            int hitTeam = SafeReadTeamHash(hitCharacter, GetTeamOffset(hitIsNpc));
            if (IsTeammate(hitTeam)) {
                return; // 直接返回，吞掉伤害判定
            }
        }
    }

    // 如果打中的是敌人，或者是墙壁掩体，都必须走原版结算！
    if (old_BulletOnCollisionEnter) {
        old_BulletOnCollisionEnter(bullet, collision, method);
    }
}

// UI子菜单：绘制功能
void ShowDrawSubMenu() {
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "📌 绘制功能");
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::Button("开启全部绘制", ImVec2(-1, 35))) EnableAllESP();
    if (ImGui::Button("关闭全部绘制", ImVec2(-1, 35))) DisableAllESP();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("基础绘制");
    ImGui::Checkbox("初始化绘制", &isESP);
    ImGui::Checkbox("显示射线", &isESPLine);
    ImGui::Checkbox("显示方框", &isESPBox);
    ImGui::Checkbox("显示距离", &isESPDistance);
    ImGui::Checkbox("显示敌人数量", &isESPObjectsCount);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("高级绘制");
    ImGui::Checkbox("显示玩家名称", &isESPName);
    ImGui::Checkbox("显示骨骼", &g_ShowBones);
    ImGui::Checkbox("显示血量条", &isESPHealth);
    ImGui::Checkbox("过滤队友", &isTeamFilter);
}

// UI子菜单：功能修改
void ShowModSubMenu() {
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "⚙️ 功能修改");
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f), "🔥 子弹追踪（安全模式）");
    ImGui::Checkbox("启用子弹追踪", &g_BulletTrackEnabled);
    if (g_BulletTrackEnabled) {
        ImGui::Indent(15);
        ImGui::SliderFloat("最大追踪距离", &g_BulletTrackMaxDist, 50.0f, 2000.0f, "%.0fm");
        ImGui::Checkbox("追踪头部（否则躯干）", &g_BulletTrackLockHead);
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "🎯 追踪模式");
        ImGui::RadioButton("最近距离（3D世界）", &g_BulletTrackMode, 0);
        ImGui::RadioButton("准星指向（屏幕中心）", &g_BulletTrackMode, 1);
        ImGui::Separator();
        ImGui::Checkbox("显示追踪范围圈", &g_ShowTrackRange);
        if (g_ShowTrackRange) {
            ImGui::SliderFloat("圆圈大小", &g_TrackCircleRadius, 30.0f, 600.0f, "%.0fpx");
        }
        ImGui::Unindent(15);
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("武器修改");
    ImGui::Checkbox("启用武器修改", &g_WeaponModEnabled);
    if (g_WeaponModEnabled) {
        ImGui::Indent(15);
        ImGui::Checkbox("无限子弹", &g_InfiniteAmmo);
        ImGui::Checkbox("无后坐力", &g_NoRecoil);
        ImGui::Checkbox("高伤害", &g_HighDamage);
        ImGui::Checkbox("无限制开火（无过热/冷却）", &g_NoFireCooldown);
        ImGui::SliderFloat("子弹速度", &g_BulletSpeedValue, 100.0f, 2000.0f, "%.0f");
        ImGui::InputInt("瞄准范围（固定）", &g_CanScopeValue, 0, 0);
        ImGui::TextDisabled("（按要求固定为50，不可修改）");
        ImGui::Unindent(15);
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("移动速度");
    ImGui::Checkbox("启用速度修改", &g_MoveSpeedModEnabled);
    if (g_MoveSpeedModEnabled) {
        ImGui::Indent(15);
        ImGui::SliderFloat("步行速度", &g_WalkSpeedValue, 1.0f, 50.0f, "%.1f");
        ImGui::SliderFloat("跑步速度", &g_RunSpeedValue, 1.0f, 50.0f, "%.1f");
        ImGui::Unindent(15);
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("玩家健康");
    ImGui::Checkbox("启用健康修改", &g_PlayerHealthModEnabled);
    if (g_PlayerHealthModEnabled) {
        ImGui::Indent(15);
        ImGui::SliderFloat("角色血量", &g_PlayerHealthValue, 100.0f, 9999.0f, "%.0f");
        ImGui::InputFloat("头盔健康（固定）", &g_HelmetHealthValue, 0, 0, "%.0f");
        ImGui::InputFloat("马甲健康（固定）", &g_VestHealthValue, 0, 0, "%.0f");
        ImGui::InputFloat("健康滑块（固定）", &g_HealthSliderValue, 0, 0, "%.0f");
        ImGui::TextDisabled("（按要求均固定为9999，不可修改）");
        ImGui::Unindent(15);
    }
}

// 主UI菜单
void ShowMenu() {
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.95f);
    
    if (ImGui::Begin("Unity Hook 菜单", 0, ImGuiWindowFlags_NoSavedSettings)) {
        float windowWidth = ImGui::GetContentRegionAvail().x;
        float leftWidth = windowWidth * 0.5f - 5;
        float rightWidth = windowWidth * 0.5f - 5;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.12f, 0.15f, 0.8f));
        ImGui::BeginChild("DrawMenu", ImVec2(leftWidth, 0), true);
        ShowDrawSubMenu();
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.12f, 0.1f, 0.8f));
        ImGui::BeginChild("ModMenu", ImVec2(rightWidth, 0), true);
        ShowModSubMenu();
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "当前FPS: %.1f | 分辨率: %dx%d", 
                          io.Framerate, Game_Screen::Width, Game_Screen::Height);
    }
    ImGui::End();
}

// 获取玩家血量信息（dump更新：health是NpcControl/PlayerControl的int32字段，偏移0x48/0x38）
void GetPlayerHealthInfo(void* player, bool isNpc, float& currentHealth, float& maxHealth) {
    currentHealth = 0.0f;
    maxHealth = 100.0f;
    
    if (!m_CachedPtr(player)) return;
    
    int health = 0;
    if (SafeReadInt32((void*)((uintptr_t)player + GetHealthOffset(isNpc)), &health) &&
        health > 0 && health <= 10000) {
        currentHealth = (float)health;
        maxHealth = 1000.0f;
        return;
    }
    
    if (currentHealth <= 0) {
        currentHealth = 100.0f;
        maxHealth = 100.0f;
    }
}

// 获取 PlayerControl 队伍哈希ID（PlayerControl.team = 0x50）
int GetPlayerTeamId(void* player) {
    return SafeReadTeamHash(player, GetTeamOffset(false));
}

// 检查是否为队友：只读主线程刷新出的本地队伍缓存，绘制线程禁止调用 Unity API。
bool IsTeammate(int playerTeamId) {
    if (!isTeamFilter) return false;

    int localTeamHash = g_LocalTeamHash.load(std::memory_order_acquire);
    if (localTeamHash < 0) return false;
    if (playerTeamId < 0) return false;
    return playerTeamId == localTeamHash;
}

// 【修复】playersList/get_type不再需要，RefreshPlayerList内部直接获取类型
void *camera = nullptr;


// 传入确定的偏移量，并使用严格的指针校验和纯内存数据提取
int SafeReadTeamHash(void* obj, int teamOffset) {
    if (!m_CachedPtr(obj)) return -1;
    
    void* strPtr = nullptr;
    if (!SafeReadObjectField(obj, (uintptr_t)teamOffset, &strPtr)) return -1;

    uintptr_t len64 = 0;
    if (!SafeRead64((void*)((uintptr_t)strPtr + 0x10), &len64)) return -1;
    
    // 取低 32 位作为字符串真实长度
    int len = (int)(len64 & 0xFFFFFFFF); 
    
    // 长度合法（1 到 32 字符），直接读取底层的 UTF-16 数据流做 Hash
    if (len > 0 && len < 32) {
        uintptr_t charsData = 0;
        // MonoString 实际字符数据存储在 +0x14 偏移处
        if (SafeRead64((void*)((uintptr_t)strPtr + 0x14), &charsData)) {
            // 将前 4 个 UTF-16 字符（8 字节）进行异或计算，生成极度安全的队伍 Hash！
            // 彻底告别 str->ToString() 带来的隐藏引擎级崩溃。
            return (int)((charsData ^ (charsData >> 16)) % 10000);
        }
    }
    
    return -1;
}


// 刷新玩家列表与坐标同步（全新优化主线程版）
void RefreshPlayerList(bool forceRefreshEntities) {
    static int frameCount = 0;
    frameCount++;

    // 1. 低频网络/实体搜索：每 60 帧（大约 1 秒）才重新用 FindObjectsOfType 搜索一次敌人，彻底解决主线程卡顿
    if (forceRefreshEntities || frameCount % 60 == 0) {
        std::lock_guard<std::mutex> infoLock(g_PlayersInfoMutex);
        std::vector<PlayerInfo> previousPlayers = g_PlayersInfo;
        g_PlayersInfo.clear();

        try {
            // --- 搜索 NpcControl（AI敌人） ---
            void* npcType = Type_GetTypeName(Il2CppString::CreateMonoString("NpcControl,Assembly-CSharp.dll"), nullptr);
            if (npcType) {
                MonoArray<void**>* npcList = Object_FindObjectsOfType(npcType, nullptr);
                if (npcList && npcList->getLength() > 0) {
                    for (int i = 0; i < npcList->getLength(); i++) {
                        void* obj = npcList->getPointer()[i];
                        if (!m_CachedPtr(obj)) continue;
                        PlayerInfo info;
                        info.playerPtr = obj;
                        info.isNpc = true;
                        info.team = SafeReadTeamHash(obj, GetTeamOffset(true));
                        info.currentHealth = 100.0f;   // 默认值，下一帧位置同步时更新为真实值
                        info.maxHealth = 100.0f;
                        RestoreMotionCache(info, previousPlayers);
                        if (Object_get_name) {
                            try { MonoString* ns = Object_get_name(obj, nullptr); if (ns) info.name = ns->ToString(); }
                            catch (...) { info.name = "NPC"; }
                        }
                        g_PlayersInfo.push_back(info);
                    }
                }
            }

            // --- 搜索 PlayerControl（真人玩家） ---
            void* playerType = Type_GetTypeName(Il2CppString::CreateMonoString("PlayerControl,Assembly-CSharp.dll"), nullptr);
            if (playerType) {
                MonoArray<void**>* playerList = Object_FindObjectsOfType(playerType, nullptr);
                if (playerList && playerList->getLength() > 0) {
                    int localTeamHash = -1;
                    for (int i = 0; i < playerList->getLength(); i++) {
                        void* obj = playerList->getPointer()[i];
                        if (!m_CachedPtr(obj) || !IsLocalPlayerControl(obj)) continue;

                        int teamHash = SafeReadTeamHash(obj, GetTeamOffset(false));
                        if (teamHash >= 0) {
                            localTeamHash = teamHash;
                            break;
                        }
                    }
                    g_LocalTeamHash.store(localTeamHash, std::memory_order_release);

                    for (int i = 0; i < playerList->getLength(); i++) {
                        void* obj = playerList->getPointer()[i];
                        if (!m_CachedPtr(obj)) continue;
                        int teamHash = SafeReadTeamHash(obj, GetTeamOffset(false));
                        if (IsLocalPlayerControl(obj)) continue;
                        if (localTeamHash >= 0 && teamHash == localTeamHash && isTeamFilter) continue;
                        PlayerInfo info;
                        info.playerPtr = obj;
                        info.isNpc = false;
                        info.team = teamHash;
                        info.currentHealth = 100.0f;   // 默认值
                        info.maxHealth = 100.0f;
                        RestoreMotionCache(info, previousPlayers);
                        if (Object_get_name) {
                            try { MonoString* ns = Object_get_name(obj, nullptr); if (ns) info.name = ns->ToString(); }
                            catch (...) { info.name = "Player"; }
                        }
                        g_PlayersInfo.push_back(info);
                    }
                }
            }

            // --- 收集骨骼指针缓存 ---
            if (g_ShowBones && Animator_GetBoneTransform) {
                std::map<int, BoneInfo> newBoneInfos;
                for (int idx = 0; idx < (int)g_PlayersInfo.size(); idx++) {
                    void* obj = g_PlayersInfo[idx].playerPtr;
                    if (!m_CachedPtr(obj)) continue;
                    if (isTeamFilter && IsTeammate(g_PlayersInfo[idx].team)) continue;
                    void* animator = GetAnimatorFromCharacter(obj, g_PlayersInfo[idx].isNpc);
                    if (!animator) continue;
                    
                    const std::vector<int> boneIds = {0,1,2,3,4,5,6,7,8,10,11,13,14,15,16,17,18};
                    for (int boneId : boneIds) {
                        try {
                            void* bt = Animator_GetBoneTransform(animator, boneId, nullptr);
                            if (m_CachedPtr(bt)) {
                                BoneInfo bi;
                                bi.transform = bt;
                                newBoneInfos[idx * 100 + boneId] = bi;
                            }
                        } catch (...) {}
                    }
                }
                std::lock_guard<std::mutex> boneLock(g_BoneInfoMutex);
                g_BoneInfos = newBoneInfos;
            }
        } catch (...) {}
    }

    // 2. 高频位置同步：以下部分【每一帧都会执行】！由于去掉了耗时的全图搜索，只做坐标转换，速度极快且100%安全安全
    static auto lastMotionSync = std::chrono::steady_clock::now();
    auto nowMotionSync = std::chrono::steady_clock::now();
    float deltaSeconds = std::chrono::duration<float>(nowMotionSync - lastMotionSync).count();
    lastMotionSync = nowMotionSync;
    if (deltaSeconds < 0.001f) deltaSeconds = 0.016f;
    if (deltaSeconds > 0.20f) deltaSeconds = 0.05f;

    try {
        void* cam = Camera_get_main(nullptr);
        if (m_CachedPtr(cam)) {
            Vector3 camPos = Transform_getPosition(Component_get_transform(cam, nullptr));
            {
                std::lock_guard<std::mutex> camLock(g_CameraMutex);
                g_CameraPosition = camPos;
            }

            std::lock_guard<std::mutex> infoLock2(g_PlayersInfoMutex);
            for (auto& info : g_PlayersInfo) {
                if (!m_CachedPtr(info.playerPtr)) {
                    info.screenDepth = -1.0f;
                    continue;
                }
                Vector3 previousPosition = info.position;
                Vector3 previousHeadPos = info.headPos;
                bool hadPreviousPosition = info.hasLastPosition;

                // 实时更新 3D 坐标与健康信息
                info.position = Transform_getPosition(info.playerPtr);
                GetPlayerHealthInfo(info.playerPtr, info.isNpc, info.currentHealth, info.maxHealth);
                
                // 获取最新精确头部 3D 坐标
                info.headPos = info.position + Vector3(0, 1.6f, 0); 
                if (Animator_GetBoneTransform) {
                    try {
                        void* anim = GetAnimatorFromCharacter(info.playerPtr, info.isNpc);
                        if (anim) {
                            void* headBone = Animator_GetBoneTransform(anim, 11, nullptr);
                            if (m_CachedPtr(headBone)) {
                                info.headPos = Transform_get_position(headBone, nullptr);
                            }
                        }
                    } catch (...) {}
                }

                Vector3 rigidVelocity = ReadCachedPlayerVelocity(info);
                if (rigidVelocity != Vector3::Zero()) {
                    info.velocity = rigidVelocity;
                } else if (hadPreviousPosition &&
                           previousPosition != Vector3::Zero() &&
                           previousHeadPos != Vector3::Zero()) {
                    Vector3 frameVelocity = (info.headPos - previousHeadPos) / deltaSeconds;
                    if (IsFiniteVector(frameVelocity)) {
                        info.velocity = ClampVectorMagnitude(frameVelocity, 60.0f);
                    }
                } else {
                    info.velocity = Vector3::Zero();
                }
                info.lastPosition = info.position;
                info.lastHeadPos = info.headPos;
                info.hasLastPosition = true;

                // 矩阵变换：计算最新的屏幕坐标
                auto sfp = Camera_WorldToScreen(cam, info.position, nullptr);
                auto shp = Camera_WorldToScreen(cam, info.headPos, nullptr);
                info.screenFootPos = ImVec2(sfp.X, sfp.Y);
                info.screenHeadPos = ImVec2(shp.X, shp.Y);
                info.screenDepth = (sfp.Z > 0.f && shp.Z > 0.f) ? sfp.Z : -1.0f;
            }

            // 实时更新所有骨骼的屏幕坐标
            if (g_ShowBones) {
                std::lock_guard<std::mutex> boneLock2(g_BoneInfoMutex);
                for (auto& [key, bi] : g_BoneInfos) {
                    if (m_CachedPtr(bi.transform)) {
                        bi.position = Transform_get_position(bi.transform, nullptr);
                        auto sp = Camera_WorldToScreen(cam, bi.position, nullptr);
                        bi.screenPos = ImVec2(sp.X, sp.Y);
                        bi.screenDepth = sp.Z;
                    }
                }
            }
        }
    } catch (...) {}
}

// 在原有的 Update 函数前添加此函数
void SyncESPDataOnMainThread() {
    static int frameCount = 0;
    frameCount++;

    void* cam = Camera_get_main(nullptr);
    if (!m_CachedPtr(cam)) return;

    // 【优化 1】：每 60 帧（约 1 秒）执行一次耗时的遍历，避免掉帧卡顿
    if (frameCount % 60 == 0) {
        std::vector<PlayerInfo> tempPlayers;
        
        // 搜索 NpcControl (这里可以放你原来的搜索逻辑)
        void* npcType = Type_GetTypeName(Il2CppString::CreateMonoString("NpcControl,Assembly-CSharp.dll"), nullptr);
        if (npcType) {
            MonoArray<void**>* npcList = Object_FindObjectsOfType(npcType, nullptr);
            if (npcList && npcList->getLength() > 0) {
                for (int i = 0; i < npcList->getLength(); i++) {
                    void* obj = npcList->getPointer()[i];
                    if (!m_CachedPtr(obj)) continue;
                    PlayerInfo info;
                    info.playerPtr = obj;
                    info.isNpc = true;
                    info.team = SafeReadTeamHash(obj, GetTeamOffset(true));
                    if (Object_get_name) {
                        try { MonoString* ns = Object_get_name(obj, nullptr); if (ns) info.name = ns->ToString(); }
                        catch (...) { info.name = "NPC"; }
                    }
                    tempPlayers.push_back(info);
                }
            }
        }
        
        // （你可以将搜索 PlayerControl 的逻辑也补在这里，存入 tempPlayers）

        // 安全地替换全局列表
        std::lock_guard<std::mutex> infoLock(g_PlayersInfoMutex);
        g_PlayersInfo = tempPlayers;
    }

    // 【优化 2】：每帧实时更新 3D 和 屏幕坐标（运行在主线程，0 延迟无残影！）
    Vector3 camPos = Transform_getPosition(Component_get_transform(cam, nullptr));
    {
        std::lock_guard<std::mutex> camLock(g_CameraMutex);
        g_CameraPosition = camPos;
    }

    std::lock_guard<std::mutex> infoLock(g_PlayersInfoMutex);
    for (auto& info : g_PlayersInfo) {
        if (!m_CachedPtr(info.playerPtr)) {
            info.screenDepth = -1.0f; // 标记为无效，GL 线程不绘制
            continue;
        }

        // 实时获取血量与 3D 位置
        GetPlayerHealthInfo(info.playerPtr, info.isNpc, info.currentHealth, info.maxHealth);
        info.position = Transform_getPosition(info.playerPtr);
        
        // 获取头部 3D 坐标
        info.headPos = info.position + Vector3(0, 1.6f, 0);
        if (Animator_GetBoneTransform) {
            try {
                void* anim = GetAnimatorFromCharacter(info.playerPtr, info.isNpc);
                if (m_CachedPtr(anim)) {
                    void* headBone = Animator_GetBoneTransform(anim, 11, nullptr);
                    if (m_CachedPtr(headBone)) info.headPos = Transform_get_position(headBone, nullptr);
                }
            } catch (...) {}
        }

        // 实时转换为屏幕坐标
        auto sfp = Camera_WorldToScreen(cam, info.position, nullptr);
        auto shp = Camera_WorldToScreen(cam, info.headPos, nullptr);
        info.screenFootPos = ImVec2(sfp.X, sfp.Y);
        info.screenHeadPos = ImVec2(shp.X, shp.Y);
        info.screenDepth = (sfp.Z > 0.f && shp.Z > 0.f) ? sfp.Z : -1.0f;
    }
    
    // 如果开启了骨骼，也可以在这里用同样的方法遍历缓存的骨骼指针并做 WorldToScreen
}




// Update函数（主线程帧回调）
void Update(void * instance, void* method) {
    void* cam = nullptr;
    if (Camera_get_main) {
        try {
            cam = Camera_get_main(nullptr);
        } catch (...) {
            cam = nullptr;
        }
    }

    if (!m_CachedPtr(cam)) {
        ClearSceneCaches();
        if (old_Update) old_Update(instance, method);
        return;
    }

    bool sceneChanged = false;
    uintptr_t camPtr = (uintptr_t)cam;
    uintptr_t lastCamPtr = g_LastMainCameraPtr.load(std::memory_order_acquire);
    if (lastCamPtr != camPtr) {
        ClearSceneCaches();
        g_LastMainCameraPtr.store(camPtr, std::memory_order_release);
        sceneChanged = true;
    }

    MarkSceneAlive();

    if (isESP || g_BulletTrackEnabled) {
        static bool firstUpdate = true;
        if (firstUpdate || sceneChanged) {
            RefreshPlayerList(true);
            firstUpdate = false;
        } else {
            RefreshPlayerList(false);
        }
    }

    // 武器修改 + 子弹追踪速度（每60帧搜索一次武器列表，符合降频铁律）
    if (g_WeaponModEnabled || g_BulletTrackEnabled) {
        static int weaponSearchFrame = 0;
        weaponSearchFrame++;
        if (weaponSearchFrame % 60 == 0) {
            void* weaponType = Type_GetTypeName(Il2CppString::CreateMonoString("Weapon,Assembly-CSharp.dll"), nullptr);
            if (weaponType) {
                MonoArray<void**>* weaponList = Object_FindObjectsOfType(weaponType, nullptr);
                if (weaponList && weaponList->getLength() > 0) {
                    std::lock_guard<std::mutex> weaponLock(g_WeaponMutex);
                    g_WeaponObjects.clear();
                    for(int i = 0; i < weaponList->getLength(); i++) {
                        void* weapon = weaponList->getPointer()[i];
                        if(m_CachedPtr(weapon)) {
                            g_WeaponObjects.push_back(weapon);
                        }
                    }
                }
            }
        }
        // 每帧对已缓存的武器写入修改（安全：只写 int/float 值字段，不调 Unity API）
        {
            std::lock_guard<std::mutex> weaponLock(g_WeaponMutex);
            for (void* weapon : g_WeaponObjects) {
                if (!m_CachedPtr(weapon)) continue;
                if (g_WeaponModEnabled) ModifyWeapon(weapon);
                if (g_BulletTrackEnabled) {
                    ApplyTrackingWeaponStats(weapon);
                }
            }
        }
    }

    // 调用原函数
    if (old_Update) old_Update(instance, method);
}

// 绘制血量条
void DrawHealthBar(ImDrawList* draw, const Rect& playerRect, float currentHealth, float maxHealth, float distance, int screenWidth) {
    if (maxHealth <= 0) return;
    
    float healthPercentage = currentHealth / maxHealth;
    healthPercentage = std::max(0.0f, std::min(1.0f, healthPercentage));
    
    float barWidth = 4.0f;
    float barHeight = playerRect.height;
    float barX, barY;
    
    if (playerRect.x + playerRect.width/2 > screenWidth / 2) {
        barX = playerRect.x - 8.0f;
    } else {
        barX = playerRect.x + playerRect.width + 4.0f;
    }
    barY = playerRect.y;
    
    if (distance > 20.0f) {
        float scale = 1.0f - (distance - 20.0f) / 100.0f;
        scale = std::max(0.3f, scale);
        barWidth *= scale;
        barHeight *= scale;
        barY = playerRect.y + (playerRect.height - barHeight) / 2.0f;
    }
    
    ImColor bgColor = ImColor(255, 0, 0, 180);
    draw->AddRectFilled(
        ImVec2(barX, barY),
        ImVec2(barX + barWidth, barY + barHeight),
        bgColor
    );
    
    ImColor healthColor;
    if (healthPercentage > 0.7f) {
        healthColor = ImColor(0, 255, 0, 220);
    } else if (healthPercentage > 0.3f) {
        healthColor = ImColor(255, 255, 0, 220);
    } else {
        healthColor = ImColor(255, 0, 0, 220);
    }
    
    float healthHeight = barHeight * healthPercentage;
    if (healthHeight > 0) {
        draw->AddRectFilled(
            ImVec2(barX, barY + barHeight - healthHeight),
            ImVec2(barX + barWidth, barY + barHeight),
            healthColor
        );
    }
    
    if (distance < 30.0f) {
        char healthText[20];
        snprintf(healthText, sizeof(healthText), "%.0f", currentHealth);
        
        float textX, textY;
        if (playerRect.x + playerRect.width/2 > screenWidth / 2) {
            textX = barX - 20.0f;
        } else {
            textX = barX + barWidth + 5.0f;
        }
        textY = barY + barHeight / 2.0f;
        
        Drawing::DrawText(10, 
            ImVec2(textX, textY), 
            ImColor(255, 255, 255, 255), 
            healthText,
            true
        );
    }
}

// 绘制ESP（【修复】不再调用Unity API，全部使用后台线程预计算的数据）
void DrawESP(ImDrawList *draw, int screenWidth, int screenHeight) {
    std::lock_guard<std::mutex> guard(drawMutex);
    if (!bInitDone) return;
    if (!IsSceneRenderAlive()) return;

    try {
        {
            std::lock_guard<std::mutex> infoLock(g_PlayersInfoMutex);
            if(isESPObjectsCount) {
                int enemyCount = 0;
                for(const auto& playerInfo : g_PlayersInfo) {
                    if(!IsTeammate(playerInfo.team)) enemyCount++;
                }
                std::string playerCount = "敌人数量: " + std::to_string(enemyCount);
                Drawing::DrawText(30, ImVec2(screenWidth / 2, 50),
                                ImColor(255,255,255,255), playerCount.c_str());
            }

            if(isESP) {
                // 从缓存获取相机位置（后台线程更新，GL线程只读）
                Vector3 cameraPos;
                {
                    std::lock_guard<std::mutex> camLock(g_CameraMutex);
                    cameraPos = g_CameraPosition;
                }
                bool haveCamera = (cameraPos != Vector3::Zero());

                for(const auto& playerInfo : g_PlayersInfo) {
                    if(IsTeammate(playerInfo.team)) continue;

                    // 使用预计算的屏幕坐标（后台线程已计算）
                    if(playerInfo.screenDepth <= 0.f) continue;
                    ImVec2 sfp = playerInfo.screenFootPos;
                    ImVec2 shp = playerInfo.screenHeadPos;
                    if(std::isnan(sfp.x) || std::isnan(sfp.y) || std::isnan(shp.x) || std::isnan(shp.y)) continue;

                    float boxHeight = fabs(shp.y - sfp.y);
                    float boxWidth = boxHeight * 0.60f;

                    Rect playerRect(
                        shp.x - (boxWidth / 2),
                        (screenHeight - shp.y),
                        boxWidth, boxHeight
                    );

                    if(isESPLine) {
                        draw->AddLine(
                            ImVec2(screenWidth / 2, screenHeight),
                            ImVec2(sfp.x, screenHeight - sfp.y),
                            ImColor(255,0,0), 1
                        );
                    }

                    if(isESPBox) {
                        Drawing::DrawBox(playerRect, 1, ImColor(255,0,0));

                        if(isESPName && !playerInfo.name.empty()) {
                            float namePosX = playerRect.x + (playerRect.width / 2);
                            float namePosY = playerRect.y - 20.0f;

                            Drawing::DrawText(15,
                                ImVec2(namePosX, namePosY),
                                ImColor(255, 255, 255, 255),
                                playerInfo.name.c_str(),
                                true
                            );
                        }
                    }

                    if(isESPHealth && playerInfo.maxHealth > 0 && haveCamera) {
                        float distance = Vector3::Distance(cameraPos, playerInfo.position);
                        DrawHealthBar(draw, playerRect, playerInfo.currentHealth, playerInfo.maxHealth, distance, screenWidth);
                    }

                    if (g_ShowBones) {
                        // 骨骼每5帧渲染一次
                        static int boneFrameSkip = 0;
                        bool doBoneRender = (++boneFrameSkip >= 5);
                        if (doBoneRender) {
                            boneFrameSkip = 0;
                            std::lock_guard<std::mutex> boneLock(g_BoneInfoMutex);
                            std::map<int, std::map<int, ImVec2>> objectBonesMap;

                            for (const auto& [boneKey, boneInfo] : g_BoneInfos) {
                                int objectIndex = boneKey / 100;
                                int boneId = boneKey % 100;
                                // 使用预计算的骨骼屏幕坐标
                                if (boneInfo.screenDepth > 0.f &&
                                    !std::isnan(boneInfo.screenPos.x) && !std::isnan(boneInfo.screenPos.y)) {
                                    objectBonesMap[objectIndex][boneId] =
                                        ImVec2(boneInfo.screenPos.x, screenHeight - boneInfo.screenPos.y);
                                }
                            }

                            const std::vector<std::pair<int, int>> boneConnections = {
                                {8, 10}, {8, 14}, {14, 16}, {16, 18},
                                {8, 13}, {13, 15}, {15, 17}, {0, 8},
                                {0, 1}, {1, 3}, {3, 5}, {0, 2}, {2, 4}, {4, 6}
                            };

                            for (const auto& [objectIndex, bonePositions] : objectBonesMap) {
                                for (const auto& [parentId, childId] : boneConnections) {
                                    if (bonePositions.count(parentId) && bonePositions.count(childId)) {
                                        draw->AddLine(
                                            bonePositions.at(parentId),
                                            bonePositions.at(childId),
                                            ImColor(0, 255, 0, 255),
                                            2.0f
                                        );
                                    }
                                }
                            }
                        }  // end if(doBoneRender)
                    }

                    if(isESPDistance && haveCamera) {
                        char extra[30];
                        float distance = Vector3::Distance(cameraPos, playerInfo.position);
                        snprintf(extra, sizeof(extra), "%0.0f m", distance);
                        Drawing::DrawText(15,
                            ImVec2(playerRect.x + (playerRect.width / 2),
                            playerRect.y + playerRect.height + 10.0f),
                            ImColor(255,255,255,255), extra
                        );
                    }
                }

                // 子弹追踪线：准星到被追踪敌人头部（使用预计算屏幕坐标）
                if (g_BulletTrackEnabled) {
                    ImVec2 crosshair(screenWidth / 2.0f, screenHeight / 2.0f);
                    ImVec2 trackedHead;
                    bool found = false;

                    if (g_BulletTrackMode == 1) {
                        // 准星模式：屏幕中心最近的敌人
                        float minScrDist = 999999.0f;
                        for (const auto& info : g_PlayersInfo) {
                            if (IsTeammate(info.team)) continue;
                            if (info.screenDepth <= 0.f) continue;
                            ImVec2 hs(info.screenHeadPos.x, screenHeight - info.screenHeadPos.y);
                            float scrDist = sqrtf((hs.x-crosshair.x)*(hs.x-crosshair.x)+(hs.y-crosshair.y)*(hs.y-crosshair.y));
                            if (scrDist > g_TrackCircleRadius) continue;
                            if (scrDist < minScrDist) {
                                minScrDist = scrDist;
                                trackedHead = hs;
                                found = true;
                            }
                        }
                    } else if (haveCamera) {
                        // 最近距离模式：3D世界距离最近的敌人
                        float minDist = g_BulletTrackMaxDist + 1.0f;
                        for (const auto& info : g_PlayersInfo) {
                            if (IsTeammate(info.team)) continue;
                            if (info.screenDepth <= 0.f) continue;
                            float d = Vector3::Distance(cameraPos, info.position);
                            if (d < minDist && d <= g_BulletTrackMaxDist) {
                                minDist = d;
                                trackedHead = ImVec2(info.screenHeadPos.x, screenHeight - info.screenHeadPos.y);
                                found = true;
                            }
                        }
                    }
                    if (found) {
                        draw->AddLine(crosshair, trackedHead, ImColor(255,255,255,220), 1.5f);
                    }
                }
            }
        }
    } catch (...) {}
}

// EGL交换缓冲钩子（修复版：字体内存泄漏、死锁、双重NewFrame）
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &Game_Screen::Width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &Game_Screen::Height);

    glWidth = Game_Screen::Width;
    glHeight = Game_Screen::Height;
    
    try {
        if (!g_Initialized) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)glWidth, (float)glHeight);
            io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f); 
            ImGui::StyleColorsDark();
            
            ImGui::GetStyle().WindowRounding = 8.0f;
            ImGui::GetStyle().ChildRounding = 8.0f;
            ImGui::GetStyle().FrameRounding = 4.0f;
            ImGui::GetStyle().ScaleAllSizes(3.0f);
            
            // 【修复】字体只在初始化时加载一次，避免每帧内存泄漏
            ImFontConfig font_cfg;
            font_cfg.SizePixels = 22.0f;
            io.Fonts->AddFontFromMemoryTTF((void *)font_v, font_v_size, 28.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
            io.Fonts->AddFontDefault(&font_cfg);
            
            ImGui_ImplOpenGL3_Init("#version 100");
            g_Initialized = true;
        }

        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)glWidth, (float)glHeight);
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        ShowMenu();

        // 子弹追踪范围圈（白色圆圈在屏幕中心）
        if (g_BulletTrackEnabled && g_ShowTrackRange && IsSceneRenderAlive()) {
            ImVec2 center(glWidth / 2.0f, glHeight / 2.0f);
            ImGui::GetBackgroundDrawList()->AddCircle(
                center, g_TrackCircleRadius, ImColor(255, 255, 255, 180), 64, 2.0f);
        }

        if (isESP) {
            // 【修复】后台线程刷新数据，避免GL线程调用Unity API导致内存损坏崩溃
            /*if (!g_RefreshRunning) {
                g_RefreshRunning = true;
                g_RefreshThread = std::thread([]() {
                    Il2CppAttach();  // 新线程必须附加到IL2CPP运行时
                    bool first = true;
                    while (g_RefreshRunning) {
                        try {
                            RefreshPlayerList(first || g_ShowBones);
                            first = false;
                        } catch (...) {}
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
                });
                g_RefreshThread.detach();
            }*/
            DrawESP(ImGui::GetBackgroundDrawList(), glWidth, glHeight);
        }
        
        ImGui::EndFrame();
        ImGui::Render();
        glViewport(0, 0, glWidth, glHeight);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (old_eglSwapBuffers) return old_eglSwapBuffers(dpy, surface);
    } catch (...) {
        g_RestartDrawing = true;
    }
    return EGL_TRUE;
}

// 函数指针初始化
void initializeFunctionPointers() {
    Object_Destroy = (void* (ARM64_CALL *)(void*, void*))GET_METHOD("UnityEngine.dll", "UnityEngine", "Object", "Destroy", 1);
    Camera_WorldToScreen = (Camera_WorldToScreen_t)GET_METHOD("UnityEngine.dll", "UnityEngine", "Camera", "WorldToScreenPoint", 1);
    Component_get_transform = (void* (ARM64_CALL *)(void*, void*))GET_METHOD("UnityEngine.dll", "UnityEngine", "Component", "get_transform", 0);
    Transform_get_position = (Vector3 (ARM64_CALL *)(void*, void*))GET_METHOD("UnityEngine.dll", "UnityEngine", "Transform", "get_position", 0);
    Transform_set_position = (void (ARM64_CALL *)(void*, Vector3, void*))GET_METHOD("UnityEngine.dll", "UnityEngine", "Transform", "set_position", 1);
    Object_FindObjectsOfType = (FindObjectsFunc_t)GET_METHOD("UnityEngine.dll", "UnityEngine", "Object", "FindObjectsOfType", 1);    
    Type_GetTypeName = (GetTypeFunc_t)GET_METHOD("mscorlib.dll", "System", "Type", "GetType", 1);           
    GameObject_get_transform = (void* (ARM64_CALL *)(void*, void*))GET_METHOD("UnityEngine.dll", "UnityEngine", "GameObject", "get_transform", 0);
    Object_IsNativeObjectAlive = (bool* (ARM64_CALL *)(void*, void*))GET_METHOD("UnityEngine.dll", "UnityEngine", "Object", "IsNativeObjectAlive", 1);  
    GameObject_FindGameObjectsWithTag = (MonoArray<void**>* (ARM64_CALL *)(MonoString*, void*))GET_METHOD("UnityEngine.dll", "UnityEngine", "GameObject", "FindGameObjectsWithTag", 1);
    Camera_get_main = (void* (ARM64_CALL *)(void*))GET_METHOD("UnityEngine.dll", "UnityEngine", "Camera", "get_main", 0);
    Physics_get_gravity = (Vector3 (ARM64_CALL *)(void*))GET_METHOD("UnityEngine.PhysicsModule.dll", "UnityEngine", "Physics", "get_gravity", 0);
    
    // 【修复】GameObject.get_gameObject → 属性名可能是 gameObject 不带 get_ 前缀
    GameObject_get_gameObject = (void* (ARM64_CALL *)(void*, void*))GET_METHOD("UnityEngine.dll", "UnityEngine", "GameObject", "gameObject", 0);
    if (!GameObject_get_gameObject) {
        GameObject_get_gameObject = (void* (ARM64_CALL *)(void*, void*))GET_METHOD("UnityEngine.dll", "UnityEngine", "GameObject", "get_gameObject", 0);
    }
    
    // 【修复】Screen.SetResolution 参数数量修正：3个参数 (int, int, bool)
    Screen_SetResolution = (void (ARM64_CALL *)(int, int, bool, void*))GET_METHOD("UnityEngine.dll", "UnityEngine", "Screen", "SetResolution", 3);
    Object_get_name = (Object_get_name_t)GET_METHOD("UnityEngine.CoreModule.dll", "UnityEngine", "Object", "get_name", 0);
    
    // 【dump更新】Player类不存在，health和team是NpcControl/PlayerControl的字段，已通过偏移读取
    Component_GetComponent = (void* (ARM64_CALL *)(void*, void*, void*))GET_METHOD("UnityEngine.CoreModule.dll", "UnityEngine", "Component", "GetComponent", 1);
    Component_GetComponentInParent = (void* (ARM64_CALL *)(void*, void*, void*))GET_METHOD("UnityEngine.CoreModule.dll", "UnityEngine", "Component", "GetComponentInParent", 1);
    Collision_get_collider = (void* (ARM64_CALL *)(void*, void*))GET_METHOD("UnityEngine.PhysicsModule.dll", "UnityEngine", "Collision", "get_collider", 0);
    Animator_GetBoneTransform = (GetBoneTransform_t)GET_METHOD("UnityEngine.AnimationModule.dll", "UnityEngine", "Animator", "GetBoneTransform", 1);
    HOOK_LOGI("Collision.get_collider: %p", Collision_get_collider);
    HOOK_LOGI("Component.GetComponentInParent: %p", Component_GetComponentInParent);
    HOOK_LOGI("Animator.GetBoneTransform: %p", Animator_GetBoneTransform);
    
    // ======= 挂载 Weapon.FireBullet：修改射击方向并显式调用原函数 =======
    void* fireBulletAddr = GET_METHOD("Assembly-CSharp.dll", "", "Weapon", "FireBullet", 1);
    if (fireBulletAddr && !old_FireBullet) {
        DobbyHook(fireBulletAddr, (void*)Hooked_FireBullet, (void**)&old_FireBullet);
        HOOK_LOGI("✅ Weapon.FireBullet 挂载成功: %p", fireBulletAddr);
    } else if (!fireBulletAddr) {
        HOOK_LOGE("❌ Weapon.FireBullet 挂载失败");
    }

    /*
    // NPC开火不参与子弹追踪，通常不需要挂载。
    void* npcFireBulletAddr = GET_METHOD("Assembly-CSharp.dll", "", "NpcControl", "FireBullet", 1);
    if (npcFireBulletAddr && !old_NpcFireBullet) {
        DobbyHook(npcFireBulletAddr, (void*)Hooked_NpcFireBullet, (void**)&old_NpcFireBullet);
        HOOK_LOGI("✅ NpcControl.FireBullet 挂载成功: %p", npcFireBulletAddr);
    }
    */

    // ======= 挂载 Bullet.OnEnable：扩大 Bullet.range，避免远距离被 traveledDist 截断 =======
    void* bulletEnableAddr = GET_METHOD("Assembly-CSharp.dll", "", "Bullet", "OnEnable", 0);
    if (bulletEnableAddr && !old_BulletOnEnable) {
        DobbyHook(bulletEnableAddr, (void*)Hooked_BulletOnEnable, (void**)&old_BulletOnEnable);
        HOOK_LOGI("✅ Bullet.OnEnable 挂载成功: %p", bulletEnableAddr);
    } else if (!bulletEnableAddr) {
        HOOK_LOGE("❌ Bullet.OnEnable 挂载失败");
    }

    // ======= 挂载 Bullet.OnCollisionEnter（穿透墙壁：非敌人碰撞不触发，子弹穿透继续飞） =======
    void* bulletColAddr = GET_METHOD("Assembly-CSharp.dll", "", "Bullet", "OnCollisionEnter", 1);
    if (bulletColAddr && !old_BulletOnCollisionEnter) {
        DobbyHook(bulletColAddr, (void*)Hooked_BulletOnCollisionEnter, (void**)&old_BulletOnCollisionEnter);
        HOOK_LOGI("✅ Bullet.OnCollisionEnter 挂载成功: %p", bulletColAddr);
    } else {
        HOOK_LOGE("❌ Bullet.OnCollisionEnter 挂载失败");
    }

    // ======= 挂载主线程 Update 钩子（主选PlayerControl，备选GameManager/NpcControl/GameLibrary） =======
    void* updateAddr = GET_METHOD("Assembly-CSharp.dll", "", "PlayerControl", "Update", 0);
    if (!updateAddr) {
        updateAddr = GET_METHOD("Assembly-CSharp.dll", "", "NpcControl", "Update", 0);
    }
    if (!updateAddr) {
        updateAddr = GET_METHOD("Assembly-CSharp.dll", "", "GameManager", "Update", 0);
    }
    if (!updateAddr) {
        updateAddr = GET_METHOD("Assembly-CSharp.dll", "", "GameLibrary", "Update", 0);
    }
    if (updateAddr && !old_Update) {
        DobbyHook(updateAddr, (void*)Update, (void**)&old_Update);
        HOOK_LOGI("✅ Update Hook 挂载成功: %p", updateAddr);
    } else {
        HOOK_LOGE("❌ Update Hook 挂载失败，所有备选方法均未找到");
    }
}

// 重启绘制线程
void restartDrawing() {
    while (true) {
        if (g_RestartDrawing) {
            std::lock_guard<std::mutex> guard(drawMutex);
            g_Initialized = false;
            g_RestartDrawing = false;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// 输入钩子
void hook_input(void *event, void *exAb, void *exAc) {
    if (old_input) old_input(event, exAb, exAc);
    ImGui_ImplAndroid_HandleTouchEvent((AInputEvent *)event, {
        (float)Game_Screen::ScreenWidth / (float)Game_Screen::Width, 
        (float)Game_Screen::ScreenHeight / (float)Game_Screen::Height
    });
}

// 窗口宽度钩子
int hook_getWidth(ANativeWindow* window) {
    if (old_getWidth) Game_Screen::ScreenWidth = old_getWidth(window);
    return old_getWidth ? old_getWidth(window) : 0;
}

// 窗口高度钩子
int hook_getHeight(ANativeWindow* window) {
    if (old_getHeight) Game_Screen::ScreenHeight = old_getHeight(window);
    return old_getHeight ? old_getHeight(window) : 0;
}

// EGL钩子初始化
void Egl_hooks() {
    // Hook窗口大小
    void* getWidthAddr = dlsym(dlopen("libandroid.so", RTLD_NOW), "ANativeWindow_getWidth");
    if (getWidthAddr && !old_getWidth) {
        DobbyHook(getWidthAddr, (void*)hook_getWidth, (void**)&old_getWidth);
    }
    void* getHeightAddr = dlsym(dlopen("libandroid.so", RTLD_NOW), "ANativeWindow_getHeight");
    if (getHeightAddr && !old_getHeight) {
        DobbyHook(getHeightAddr, (void*)hook_getHeight, (void**)&old_getHeight);
    }
    
    // Hook输入
    void *sym_input = DobbySymbolResolver("/system/lib/libinput.so", "_ZN7android11MotionEvent8copyFromEPKS0_b");
    if (sym_input && !old_input) {
        DobbyHook(sym_input, (void*)hook_input, (void**)&old_input);
    }
    
    // Hook EGL交换缓冲
    void *sym_Egl = DobbySymbolResolver("/system/lib/libEGL.so", "eglSwapBuffers");
    if (sym_Egl && !old_eglSwapBuffers) {
        DobbyHook(sym_Egl, (void*)hook_eglSwapBuffers, (void**)&old_eglSwapBuffers);
    }
}

// 钩子主线程
void *hack_thread(void *) {
    HOOK_LOGI("hack_thread 启动，开始Hook EGL...");
    Egl_hooks();
    std::thread(restartDrawing).detach();

    // 等待libil2cpp加载完成
    HOOK_LOGI("等待 %s 加载...", libName);
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!isLibraryLoaded(libName));
    HOOK_LOGI("%s 已加载！", libName);

    Il2CppAttach();
    HOOK_LOGI("Il2CppAttach 完成");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    HOOK_LOGI("开始解析函数指针...");
    initializeFunctionPointers();
    HOOK_LOGI("函数指针解析完成");

    // 二次Hook EGL（增强：使用RTLD_DEFAULT兜底）
    pool.enqueue([] {
        auto p_eglSwapBuffers = (uintptr_t)dlsym(RTLD_NEXT, "eglSwapBuffers");
        if (!p_eglSwapBuffers) {
            p_eglSwapBuffers = (uintptr_t)dlsym(RTLD_DEFAULT, "eglSwapBuffers");
        }
        if (p_eglSwapBuffers && !old_eglSwapBuffers) {
            DobbyHook((void *)p_eglSwapBuffers, (void *)hook_eglSwapBuffers, (void **)&old_eglSwapBuffers);  
        }
        bInitDone = true;
    });
    
    return nullptr;
}

#endif
