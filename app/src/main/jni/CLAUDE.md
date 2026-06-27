# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

生存子弹追踪 - Android Unity IL2CPP 游戏外挂模块（ImGui 菜单注入）。构建为 `libDarkTeam.so`，通过 Dobby inline hook 注入目标游戏进程，提供透视（ESP）、子弹追踪（自瞄）、武器修改等功能。

## 构建

```bash
# 在 jni 目录下执行
cd app/src/main/jni
ndk-build
```

- 目标架构：`arm64-v8a`（仅 64 位 ARM）
- 最低 API：`android-28`
- 编译标准：C++17
- 输出：`libs/arm64-v8a/libDarkTeam.so`

构建配置：[Android.mk](Android.mk)、[Application.mk](Application.mk)。Gradle 构建也配置了 `externalNativeBuild` 指向 `Android.mk`。

## 注入方式

1. **APK 注入**：将 `libDarkTeam.so` 放入目标 APK 的 `lib/arm64-v8a/` 目录，修改 `smali` 代码添加 `System.loadLibrary("DarkTeam")`，重编译签名
2. **Frida 注入**：使用 [load_so.js](load_so.js) 在运行时通过 Frida 调用 `dlopen` 加载

## 核心架构

项目是单文件为主的架构，所有游戏逻辑集中在 [hooks.h](hooks.h)（~1570 行），其他文件为工具库：

### 入口与生命周期

- **[main.cpp](main.cpp)**：`__attribute__((constructor))` 标记的 `lib_main()`，在库加载时自动执行。安装崩溃信号处理器（SIGSEGV/SIGABRT/SIGBUS/SIGILL）后，创建 `hack_thread`
- **`hack_thread()`** [hooks.h:1535](hooks.h#L1535)：启动流程：
  1. 调用 `Egl_hooks()` 钩取 EGL 交换缓冲和 Android 输入事件
  2. 等待 `libil2cpp.so` 加载完成 → `Il2CppAttach()` 附加到 IL2CPP 运行时
  3. `initializeFunctionPointers()` 解析所有需要的 IL2CPP 方法指针
  4. 二次 Hook EGL（`RTLD_DEFAULT` 兜底）→ 设置 `bInitDone = true`

### Hook 点

| Hook 函数 | 目标 | 用途 |
|---|---|---|
| `eglSwapBuffers` | `libEGL.so` | 每帧渲染 ImGui 菜单和 ESP 覆盖层 |
| `ANativeWindow_getWidth/Height` | `libandroid.so` | 捕获屏幕分辨率 |
| `Input::MotionEvent::copyFrom` | `libinput.so` | 捕获触摸事件传给 ImGui |
| `Bullet.OnCollisionEnter` | `Assembly-CSharp.dll` | 子弹碰撞（预留，当前原样放行） |
| `GameManager/PlayerControl.Update` | `Assembly-CSharp.dll` | 主循环回调，驱动武器修改、子弹追踪速度写入、玩家列表刷新 |

### 关键设计模式

**GL 线程安全（最重要！）**：GL 线程（`eglSwapBuffers` 回调）**绝不能调用 Unity API**，否则导致内存损坏崩溃。项目采用后台线程预计算方案：
- `Update()` 钩子中每帧计算所有玩家的 3D 位置 → 屏幕坐标，存入 `g_PlayersInfo`（带 `std::mutex` 保护）
- `DrawESP()` (GL 线程) 只读取预计算坐标，只做绘制调用
- 后台线程（`g_RefreshThread`）方案已注释禁用，当前全部在 `Update()` 中同步完成

**搜索降频**：`FindObjectsOfType` 是昂贵的全场景遍历。每 60 帧（约 1 秒）才执行一次搜索，其余帧只对已有缓存做轻量字段读写。

**ARM64 SIMD 寄存器问题**：Unity IL2CPP 的 `Vector3` 参数通过 ARM64 SIMD 寄存器传递（v0-v2），C++ 无法正确访问。`FireBullet` 的 Vector3 参数 Hook 因此被禁用，改用直接写 `Weapon.bulletSpeed` 字段（偏移 `0x3C`）+ 高速度值实现子弹追踪效果。

### 功能模块（均在 hooks.h）

- **透视 (ESP)**：方框、射线、距离、名字、骨骼、血量条，支持队友过滤
- **子弹追踪**：两种模式 — 最近 3D 距离 / 准星指向（屏幕距离），通过修改 `Weapon.bulletSpeed = 5000` 实现
- **武器修改**：无限子弹、无后坐力、高伤害、子弹速度（直接写内存偏移）
- **移动速度 / 玩家血量**：声明了接口但当前游戏中对应类不存在，实现已禁用

### 关键数据结构

- `PlayerInfo` [hooks.h:132](hooks.h#L132)：玩家信息（指针、名字、3D 位置、血量、队伍、预计算屏幕坐标）
- `BoneInfo` [hooks.h:146](hooks.h#L146)：骨骼信息（Transform 指针、3D 位置、预计算屏幕坐标）
- `Game_Screen` [hooks.h:50](hooks.h#L50)：屏幕尺寸
- `g_PlayersInfo` / `g_BoneInfos` / `g_CameraPosition`：带互斥锁保护的全局数据

### 依赖库

| 目录 | 用途 |
|---|---|
| `Dobby/` | 轻量级 ARM64 inline hook 框架 |
| `ImGui/` 及 `backends/` | UI 渲染（OpenGL ES 3 + Android 后端） |
| `KittyMemory/` | 内存补丁、备份、工具 |
| `Substrate/` | Cydia Substrate hook 框架（备用，当前未使用） |
| `ByNameModding/` | IL2CPP 运行时交互（类型查找、方法解析、字段偏移） |
| `Il2cpp/xdl/` | xDL — ELF 增强 `dlopen`/`dlsym` |
| `Unity/` | Unity 数学结构体（Vector2/3、Quaternion、Rect、Color） |

### 工具文件

- [timer.h](timer.h)：高精度 FPS 定时器和 CPU 亲和设置
- [Utils.h](Utils.h)：`/proc/self/maps` 解析、库基址查找、字符串转偏移
- [Drawing.h](Drawing.h)：ImGui `BackgroundDrawList` 绘制封装（线、框、文本、圆）
- [bools.h](bools.h)：所有功能开关的全局布尔变量
- [fix_bones.py](fix_bones.py)：Python 脚本，用于批量修改 `hooks.h` 中的 Hook 启用/禁用状态（通过字符串替换注释掉代码块）

## 代码风格

- 注释使用**简体中文**
- 功能开关使用 `bool` 全局变量（定义在 `bools.h`）
- 字符串偏移读取使用 `GET_METHOD(assembly, namespace, klass, method, paramCount)` 宏
- ARM64 函数指针需要 `ARM64_CALL`（即 `__attribute__((pcs("aapcs"))）`) 调用约定
- 游戏对象字段通过硬编码偏移量 + 类型强制转换读取（如 `*(int*)((uintptr_t)weapon + 0xC8)`）
