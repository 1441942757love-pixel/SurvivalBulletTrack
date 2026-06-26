#include "UE4_Core.h"

using namespace std;
int glWidth = 0, glHeight = 0;
int screenWidth = -1, screenHeight = -1;
jobject DrawView;
ANativeWindow *GameWindow;
static ANativeWindow *g_NativeWindow = nullptr;
static EGLDisplay g_EglDisplay = EGL_NO_DISPLAY;
static EGLSurface g_EglSurface = EGL_NO_SURFACE;
static EGLContext g_EglContext = EGL_NO_CONTEXT;
bool isInitImGui = false;
int SizeX, SizeY;
TextureInfo Safe_Picture;
TextureInfo NoSafe_Picture;
void InitTexture() {
    Safe_Picture = CreateTexture(yesafe_icon, sizeof(yesafe_icon));
    NoSafe_Picture = CreateTexture(nosafe_icon, sizeof(nosafe_icon));
}
bool initHook = false;
ImColor 白色 = ImColor(255, 255, 255, 255);
ImColor 红色 = ImColor(255, 0, 0, 255);
ImColor 绿色 = ImColor(0, 255, 0, 255);
ImColor 黄色 = ImColor(255, 255, 0, 255);
ImColor 暗绿色 = ImColor(68, 170, 13, 255);
ImColor 暗红色 = ImColor(217, 0, 0, 255);
ImVec2 DrawText(float size, int x, int y, ImVec4 color, const char* str)
{
    auto textSize = ImGui::CountTextSize(NULL, str, size);  
    float centerX = x - textSize.x / 2;
    float centerY = y;
    ImGui::GetBackgroundDrawList()->AddText(NULL, size, ImVec2(centerX + 1, centerY), ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)), str);
    ImGui::GetBackgroundDrawList()->AddText(NULL, size, ImVec2(centerX - 0.1, centerY), ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)), str);
    ImGui::GetBackgroundDrawList()->AddText(NULL, size, ImVec2(centerX, centerY + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)), str);
    ImGui::GetBackgroundDrawList()->AddText(NULL, size, ImVec2(centerX, centerY - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)), str);
    ImGui::GetBackgroundDrawList()->AddText(NULL, size, ImVec2(centerX, centerY), ImGui::ColorConvertFloat4ToU32(color), str);    
    return textSize;
}

void DrawText(float size, float x, float y, ImColor color, ImColor color1, const char* str)
{
    ImGui::GetBackgroundDrawList()->AddText(NULL, size, ImVec2(x-0.1, y-0.1), color1, str);
    ImGui::GetBackgroundDrawList()->AddText(NULL, size, ImVec2(x+0.1, y+0.1), color1, str);
    ImGui::GetBackgroundDrawList()->AddText(NULL, size, ImVec2(x, y), color, str);
}

#include "SDK/Function.h"
uintptr_t World;
FVector CameraLoc = {0, 0, 0};
FVector TargetLoc = {0, 0, 0};
APlayerCameraManager* PlayerCameraManager;
APlayerController* PlayerController;
AActor* AcknowledgedPawn;
AActor* LockObj;
TArray<AActor* > ActorsToIgnore;

bool LineTrace(FVector End)
{
    HitResult out;
    if (World) 
    {        
        KismetSystemLibrary *gKismetSystemLibrary = (KismetSystemLibrary *)KismetSystemLibrary::StaticClass();
        bool bHit = gKismetSystemLibrary->LineTraceSingle(World, CameraLoc, End, 0, true, ActorsToIgnore, 0, out, true, {0,0,0,0}, {0,0,0,0}, 0.0f);
        if (bHit)
            return false;
        else
            return true;
    }
    return false;
}

bool WorldToScreen(FVector HeadPos, FVector2D& oScreen) {
    if (!PlayerController->ProjectWorldLocationToScreen(HeadPos, oScreen, false)) {
        return false;
    }
    int drawSizeX = screenWidth;
    int drawSizeY = screenHeight;
    oScreen.X = (oScreen.X / SizeX) * drawSizeX;
    oScreen.Y = (oScreen.Y / SizeY) * drawSizeY;
    return true; 
}
bool BoneToScreen(const class FVector& WorldLocation, FVector2D* Rect)
{
	FVector2D ScreenView;
    if (!WorldToScreen(WorldLocation, ScreenView)) {
        return false;
    }
    Rect->X = ScreenView.X;
	Rect->Y = ScreenView.Y;
	return true;
}

bool TempLockVisble = false;
FVector TempLockPos, TrackLockPos;
bool BoneToScreen_LineOfSight(const class FVector& WorldLocation, FVector* Rect)
{
    FVector2D ScreenView;	
    if (!WorldToScreen(WorldLocation, ScreenView)) {
        return false;
    }
    Rect->X = ScreenView.X;
    Rect->Y = ScreenView.Y;
    float is = LineTrace(WorldLocation) ? 1.0f : 0.0f;
    Rect->Z = is;
    if (is == 1.0f && !TempLockVisble) {
        TempLockPos = WorldLocation;
        TempLockVisble = true;
    }
    return true;
}

static bool _WorldToScreen(FVector WorldLocation, FVector *pOut) {
    FVector2D Location;
    WorldToScreen(WorldLocation, Location);
    pOut->X = Location.X, pOut->Y = Location.Y;
    return false;
}
bool WorldToScreen(FVector WorldLocation, FVector *pOut) {
    FVector screen;
    _WorldToScreen(WorldLocation, &screen);

    FVector world2 = {WorldLocation.X, WorldLocation.Y,WorldLocation.Z + 165};
    FVector screen2;
    _WorldToScreen(world2, &screen2);

    float h = screen.Y - screen2.Y; 
    float w = h / 1.5f; 
    pOut->X = screen.X; 
    pOut->Y = screen.Y; 
    pOut->Z = w;
    return true;
}

FVector GetBoneLocation(USkeletalMeshComponent *Mesh, int Id)
{    
    return Mesh->GetBoneLocation(Mesh->GetBoneName(Id), 0);                    
}

struct Config {
    bool 方框;
    bool 射线;
    bool 骨骼;
    bool 血量;
    bool 信息;
    bool 英雄;
    bool 手持;
    bool 护甲;
    bool 人数;
    bool 战场;    
    bool 盒子;
    bool 隐藏人机;

    bool 子弹追踪;
    float 追踪概率 = 100.0;
    float 锁定范围 = 250.0;
    bool 启用预判;
    bool 忽略人机;
    bool 忽略倒地;    
    
    bool 开火自瞄;
    bool 自动压枪;
    float 压枪力度 = 0.2f;
    int 拉枪速度 = 3;
        
    bool 子弹瞬击;
    bool 范围;
    float 头范围 = 3.0f;
    float 身范围 = 3.0f;    
    int 最低过滤价值 = 10000;
    int Index01 = 0;
    
    bool 物资;
    ImColor 盒子颜色 = ImColor(255,255,255,255);
} Settings;

const char* GetHeroName(int yx) {
    static const std::unordered_map<int, std::string> 英雄表 = {
        {2100654105, "威龙"},
        {2100654106, "骇爪"},
        {2100654107, "蜂医"},
        {2100654108, "露娜"},
        {2100654109, "牧羊人"},
        {2100654110, "红狼"},
        {2100654115, "乌鲁鲁"},
        {2100654116, "蛊"},
        {2100654117, "深蓝"},
    };

    auto it = 英雄表.find(yx);
    return it != 英雄表.end() ? it->second.c_str() : "未知干员";
}

std::string extractDigits(int number, int start, int end) {
    std::string str = std::to_string(number);
    if (str[0] == '-') {
        str = str.substr(1);
    }
    if (start < 0 || end >= str.size() || start > end) {
        return "";
    }
    return str.substr(start, end - start + 1);
}

int GetArmorLeveL(int id) {
    std::unordered_map<std::string, int> levelMap = {
        {"900", 1}, {"899", 2}, {"898", 3}, {"897", 4}, {"896", 5}, {"895", 6}
    };

    std::string val = extractDigits(id, 4, 6);
    return levelMap.count(val) ? levelMap[val] : 0;
}

const char* GetWeapon(int weaponID) {
    static const std::unordered_map<int, std::string> WeaponTable = {
        // 突击步枪
        {830130856, "K437突击步枪"},
        {830130854, "腾龙突击步枪"},
        {830130853, "AC Val突击步枪"},
        {830130847, "CAR-15突击步枪"},
        {830130840, "PTR-32突击步枪"},
        {830130839, "G3战斗步枪"},
        {830130837, "SCAR-H战斗步枪"},
        {830130834, "AK-12突击步枪"},
        {830130833, "SG552突击步枪"},
        {830130832, "M7战斗步枪"},
        {830130831, "AUG突击步枪"},
        {830130830, "M16A4突击步枪"},
        {830130829, "K416突击步枪"},
        {830130828, "ASh-12战斗步枪"},
        {830130826, "AKS-74U突击步枪"},
        {830130824, "QBZ95-1突击步枪"},
        {830130822, "AKM突击步枪"},
        {830130817, "M4A4突击步枪"},
        // 冲锋枪
        {840130827, "QCQ171冲锋枪"},
        {840130826, "MP7冲锋枪"},
        {840130825, "勇士冲锋枪"},
        {840130824, "SR-3M紧凑突击步枪"},
        {840130822, "SMG-45冲锋枪"},
        {840130821, "野牛冲锋枪"},
        {840130820, "UZI冲锋枪"},
        {840130819, "Vector冲锋枪"},
        {840130818, "P90冲锋枪"},
        {840130817, "MP5冲锋枪"},
        // 霰弹枪
        {850130821, "725双管霰弹枪"},
        {850130820, "M870霰弹枪"},
        {850130818, "S12K霰弹枪"},
        {850130817, "M1014霰弹枪"},
        // 机枪
        {860130824, "机枪"},
        {860130820, "QJB201轻机枪"},
        {860130819, "M250通用机枪"},
        {860130818, "M249轻机枪"},
        {860130817, "PKM通用机枪"},
        // 射手步枪
        {870130847, "PSG-1射手步枪"},
        {870130824, "SR9射手步枪"},
        {870130823, "SR-25射手步枪"},
        {870130822, "SKS射手步枪"},
        {870130821, "M14射手步枪"},
        {870130820, "SVD狙击步枪"},
        {870130819, "VSS射手步枪"},
        {870130818, "Mini-14射手步枪"},
        // 狙击步枪
        {880130827, "AWM狙击步枪"},
        {880130825, "M700狙击步枪"},
        {880130824, "R93狙击步枪"},
        {880130823, "SV-98狙击枪"},
        // 手枪 & 特殊武器
        {890130846, "医疗枪"},
        {890130849, "M1911手枪"},
        {890130826, "G17手枪"},
        {890130822, "93R手枪"},
        {890130821, "G18手枪"},
        {890130820, "沙漠之鹰"},
        {890130819, ".357左轮"},
        {890130818, "QSZ92G手枪"},
        // 近战武器
        {920130818, "刀"},
        {920130825, "小刀"},
        // 弓 & 特殊武器
        {950130817, "雷电弓"},
        {950130818, "探测弓"},
        {900130827, "击倒炮"},
        {900130826, "手炮"},
        {900130825, "追踪弹"},
        {900130823, "火箭筒"},
    };
    // 查找武器
    auto it = WeaponTable.find(weaponID);
    return it != WeaponTable.end() ? it->second.c_str() : "未知武器";
}