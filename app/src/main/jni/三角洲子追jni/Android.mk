LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cpp .cc
LOCAL_MODULE    := Game

LOCAL_CFLAGS := -w -s -Wno-error=format-security -fvisibility=hidden
LOCAL_LDFLAGS += -Wl,--gc-sections,--strip-all,-llog,--strip-all
LOCAL_CPPFLAGS += -w -s -Wno-error=format-security -mllvm -aesSeed=DEADBEEFDEADBEEFDEADBEEFDEADBEEF -mllvm -sobf -mllvm -sub -mllvm -split -fvisibility=hidden -Werror -std=c++17  -g0 -O3 -mcpu=native
LOCAL_CPPFLAGS += -Wno-error=c++11-narrowing -fno-rtti -fno-exceptions -Wall -fomit-frame-pointer
LOCAL_CFLAGS += -ffunction-sections -fdata-sections


LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/MainLooper
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/InlineHook
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/ImGui
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/

FILE_LIST += $(wildcard $(LOCAL_PATH)/*.c*)
FILE_LIST += $(wildcard $(LOCAL_PATH)/include/ImGui/*.c*)
FILE_LIST += $(wildcard $(LOCAL_PATH)/include/InlineHook/*.c*)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)
LOCAL_STATIC_LIBRARIES=android_native_app_glue

LOCAL_CFLAGS += -w -std=c++17

LOCAL_LDLIBS := -lm -ldl -lz -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -lGLESv3


LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -lGLESv3
include $(BUILD_SHARED_LIBRARY)
$(call import-module,android/native_app_glue)

