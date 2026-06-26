#pragma once
#include <string>    
#include <vector>  
using namespace std;  

android_app *g_App = nullptr;

// JNI配置
JavaVM* CurrentJavaVM = nullptr;


void JavaEnvDestructor(void*) {
    CurrentJavaVM->DetachCurrentThread();
}
struct DisplayMetrics {
    int widthPixels;
    int heightPixels;
    float density;
};
JNIEnv* GetJavaEnv() {
    static uintptr_t TlsSlot = 0;
    if (TlsSlot == 0) {
        pthread_key_create((pthread_key_t*)&TlsSlot, &JavaEnvDestructor);
    }
    auto* Env = (JNIEnv*)pthread_getspecific(TlsSlot);
    if (Env == nullptr) {
        CurrentJavaVM->GetEnv((void **)&Env, JNI_VERSION_1_6);
        jint AttachResult = CurrentJavaVM->AttachCurrentThread(&Env, nullptr);
        if (AttachResult == JNI_ERR) {
            return nullptr;
        }
        pthread_setspecific(TlsSlot, (void*)Env);
    }
    return Env;
}

DisplayMetrics GetScreenInfo() {
    DisplayMetrics result;
    JNIEnv* java_env = GetJavaEnv();
    jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
    jmethodID getSystemServiceMethod = java_env->GetMethodID(native_activity_clazz, xorstr_("getSystemService"), xorstr_("(Ljava/lang/String;)Ljava/lang/Object;"));

    jstring str = java_env->NewStringUTF(xorstr_("window"));
    jobject windowManager = java_env->CallObjectMethod(g_App->activity->clazz, getSystemServiceMethod, str);
    java_env->DeleteLocalRef(str);

    jclass PointClass = java_env->FindClass(xorstr_("android/graphics/Point"));
    jmethodID pointInit = java_env->GetMethodID(PointClass, xorstr_("<init>"), xorstr_("()V"));
    jobject point = java_env->NewObject(PointClass, pointInit);

    jclass windowManagerClass = java_env->FindClass(xorstr_("android/view/WindowManager"));
    jmethodID getDefaultDisplay = java_env->GetMethodID(windowManagerClass, xorstr_("getDefaultDisplay"), xorstr_("()Landroid/view/Display;"));
    jobject display = java_env->CallObjectMethod(windowManager, getDefaultDisplay);

    jclass displayClass = java_env->GetObjectClass(display);
    jmethodID getRealSize = java_env->GetMethodID(displayClass, xorstr_("getRealSize"), xorstr_("(Landroid/graphics/Point;)V"));
    java_env->CallVoidMethod(display, getRealSize, point);

    jfieldID xField = java_env->GetFieldID(PointClass, xorstr_("x"), xorstr_("I"));
    jint x = java_env->GetIntField(point, xField);

    jfieldID yField = java_env->GetFieldID(PointClass, xorstr_("y"), xorstr_("I"));
    jint y = java_env->GetIntField(point, yField);

    if (x > y) {
        result.widthPixels = x;
        result.heightPixels = y;
    } else {
        result.widthPixels = y;
        result.heightPixels = x;
    }

    java_env->DeleteLocalRef(native_activity_clazz);
    java_env->DeleteLocalRef(PointClass);
    java_env->DeleteLocalRef(point);
    java_env->DeleteLocalRef(windowManagerClass);
    java_env->DeleteLocalRef(display);
    java_env->DeleteLocalRef(displayClass);
    return result;
}


jobject createLayoutParams(int width, int height) {
    JNIEnv *env = GetJavaEnv();
    jclass lpClass = env->FindClass(xorstr_("android/view/WindowManager$LayoutParams"));
    jmethodID lpInit = env->GetMethodID(lpClass, xorstr_("<init>"), xorstr_("()V"));
    jobject lp = env->NewObject(lpClass, lpInit);

    jfieldID typeField = env->GetFieldID(lpClass, xorstr_("type"), xorstr_("I"));
    env->SetIntField(lp, typeField, 2);
    
    jfieldID flagsField = env->GetFieldID(lpClass, xorstr_("flags"), xorstr_("I"));
    env->SetIntField(lp, flagsField, 512 | 8 | 16 | 32 | 1024 | 256 | 0x01000000 | 1073741824); //824 隐藏窗口前flags:16778040 隐藏窗口后flags:16782136 8192

    jfieldID gravityField = env->GetFieldID(lpClass, xorstr_("gravity"), xorstr_("I"));
    env->SetIntField(lp, gravityField, 3 | 48);

    jfieldID formatField = env->GetFieldID(lpClass, xorstr_("format"), xorstr_("I"));
    env->SetIntField(lp, formatField, 1);

    jfieldID widthField = env->GetFieldID(lpClass, xorstr_("width"), xorstr_("I"));
    env->SetIntField(lp, widthField, width);

    jfieldID heightField = env->GetFieldID(lpClass, xorstr_("height"), xorstr_("I"));
    env->SetIntField(lp, heightField, height);

    env-> DeleteLocalRef(lpClass);
    return lp;
}

jobject CreateDrawView(DisplayMetrics size) {
    JNIEnv *env = GetJavaEnv(); 
    auto Clazz = g_App->activity->clazz;   
    jclass ClazzName = env->GetObjectClass(Clazz);     
    jmethodID getSystemService = env->GetMethodID(ClazzName, xorstr_("getSystemService"), xorstr_("(Ljava/lang/String;)Ljava/lang/Object;"));
    jstring windowStr = env->NewStringUTF(xorstr_("window"));
    jobject windowManager = env->CallObjectMethod(Clazz, getSystemService, windowStr);
    env->DeleteLocalRef(windowStr);
    jclass windowManagerClass = env->GetObjectClass(windowManager);
    jobject lp = createLayoutParams(size.widthPixels, size.heightPixels);
    jclass surfaceViewClass = env->FindClass(xorstr_("android/view/SurfaceView"));
    jmethodID surfaceViewInit = env->GetMethodID(surfaceViewClass, xorstr_("<init>"), xorstr_("(Landroid/content/Context;)V"));
    jobject surfaceView = env->NewObject(surfaceViewClass, surfaceViewInit, g_App->activity->clazz);
    jmethodID addView = env->GetMethodID(windowManagerClass, xorstr_("addView"), xorstr_("(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V"));
    jmethodID getHolderMethod = env->GetMethodID(surfaceViewClass, xorstr_("getHolder"), xorstr_("()Landroid/view/SurfaceHolder;"));
    jobject surfaceHolder = env->CallObjectMethod(surfaceView, getHolderMethod);
    jclass surfaceHolderClass = env->GetObjectClass(surfaceHolder);
    jmethodID setZOrderOnTopMethod = env->GetMethodID(surfaceViewClass, xorstr_("setZOrderOnTop"), xorstr_("(Z)V"));
    env->CallVoidMethod(surfaceView, setZOrderOnTopMethod, true);
    jmethodID setFormatMethod = env->GetMethodID(surfaceHolderClass, xorstr_("setFormat"), xorstr_("(I)V"));
    env->CallVoidMethod(surfaceHolder, setFormatMethod, -2);
    env->CallVoidMethod(windowManager, addView, surfaceView, lp);
    env-> DeleteLocalRef(ClazzName);
    env-> DeleteLocalRef(windowManager);
    env-> DeleteLocalRef(surfaceHolderClass);
    env-> DeleteLocalRef(surfaceHolder);
    env-> DeleteLocalRef(surfaceHolderClass);
    return env->NewGlobalRef(surfaceView);
}

ANativeWindow* GetNativeWindow(jobject view) {
    jobject surface;
    ANativeWindow* nativeWindow = nullptr;
    JNIEnv *env = GetJavaEnv();   
    jclass surfaceViewClass = env->GetObjectClass(view);
    jmethodID getHolderMethod = env->GetMethodID(surfaceViewClass, xorstr_("getHolder"), xorstr_("()Landroid/view/SurfaceHolder;"));
    jobject surfaceHolder = env->CallObjectMethod(view, getHolderMethod);    
    if (surfaceHolder == nullptr) {
        env->DeleteLocalRef(surfaceViewClass);
        return 0;
    }
    jclass surfaceHolderClass = env->GetObjectClass(surfaceHolder);
    jmethodID getSurfaceMethod = env->GetMethodID(surfaceHolderClass, xorstr_("getSurface"), xorstr_("()Landroid/view/Surface;"));
    surface = env->CallObjectMethod(surfaceHolder, getSurfaceMethod);
    env->DeleteLocalRef(surfaceViewClass);
    env->DeleteLocalRef(surfaceHolderClass);
    nativeWindow = ANativeWindow_fromSurface(env, surface);
    return nativeWindow;
}
