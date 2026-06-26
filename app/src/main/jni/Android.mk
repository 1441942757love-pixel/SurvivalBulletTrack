LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE            := libdobby
LOCAL_SRC_FILES         := Dobby/libraries/$(TARGET_ARCH_ABI)/libdobby.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/dobby/
include $(PREBUILT_STATIC_LIBRARY)
LOCAL_LDFLAGS := $(LOCAL_PATH)/Dobby/armeabi-v7a/libdobby.a
include $(CLEAR_VARS)

LOCAL_MODULE    := DarkTeam

LOCAL_CFLAGS := -w -s -Wno-error=format-security -fvisibility=hidden -fpermissive -fexceptions
LOCAL_CPPFLAGS := -w -s -Wno-error=format-security -fvisibility=hidden -Werror -std=c++17
LOCAL_CPPFLAGS += -Wno-error=c++11-narrowing -fpermissive -Wall -fexceptions
LOCAL_LDFLAGS += -Wl,--gc-sections,--strip-all,-llog
LOCAL_LDLIBS := -llog -landroid -lGLESv2 -lEGL -lGLESv3
LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := libdobby

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/ImGui
LOCAL_C_INCLUDES += $(LOCAL_PATH)/ImGui/backends
FILE_LIST += $(wildcard $(LOCAL_PATH)/ImGui/*.c*)

LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_SRC_FILES := main.cpp \
    Il2cpp/xdl/xdl.c \
    Il2cpp/xdl/xdl_iterate.c \
    Il2cpp/xdl/xdl_linker.c \
    Il2cpp/xdl/xdl_lzma.c \
    Il2cpp/xdl/include/xdl.h \
    Il2cpp/xdl/xdl_util.c \
	ByNameModding/Tools.cpp \
    ByNameModding/fake_dlfcn.cpp \
    ByNameModding/Il2Cpp.cpp \
	Substrate/hde64.c \
	Substrate/SubstrateDebug.cpp \
	Substrate/SubstrateHook.cpp \
	Substrate/SubstratePosixMemory.cpp \
	Substrate/SymbolFinder.cpp \
    KittyMemory/KittyMemory.cpp \
	KittyMemory/MemoryPatch.cpp \
    KittyMemory/MemoryBackup.cpp \
    KittyMemory/KittyUtils.cpp \
    ImGui/imgui.cpp \
    ImGui/imgui_draw.cpp \
    ImGui/imgui_widgets.cpp \
    ImGui/imgui_tables.cpp \
    ImGui/backends/imgui_impl_opengl3.cpp \
    ImGui/backends/imgui_impl_android.cpp \

include $(BUILD_SHARED_LIBRARY)
