#include <pthread.h>
#include <android/log.h>
#include <signal.h>
#include <unwind.h>
#include <dlfcn.h>
#include <string.h>
#include "hooks.h"

// 外部声明全局变量（实际定义在bools.h）
extern bool g_Initialized;
extern bool bInitDone;
extern int glWidth;
extern int glHeight;

#define CRASH_LOG_TAG "DarkTeam_Crash"

// ====== 崩溃信号捕获 ======
struct BacktraceState {
    void** current;
    void** end;
};

static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* ctx, void* arg) {
    BacktraceState* state = (BacktraceState*)arg;
    uintptr_t pc = _Unwind_GetIP(ctx);
    if (pc) {
        if (state->current == state->end) return _URC_END_OF_STACK;
        *state->current++ = (void*)pc;
    }
    return _URC_NO_REASON;
}

static void logBacktrace() {
    void* buffer[64];
    BacktraceState state = {buffer, buffer + 64};
    _Unwind_Backtrace(unwindCallback, &state);
    int count = state.current - buffer;

    __android_log_print(ANDROID_LOG_ERROR, CRASH_LOG_TAG, "=== CRASH BACKTRACE (%d frames) ===", count);
    for (int i = 0; i < count; i++) {
        Dl_info info;
        if (dladdr(buffer[i], &info) && info.dli_fname) {
            uintptr_t offset = (uintptr_t)buffer[i] - (uintptr_t)info.dli_fbase;
            const char* fname = strrchr(info.dli_fname, '/');
            fname = fname ? fname + 1 : info.dli_fname;
            __android_log_print(ANDROID_LOG_ERROR, CRASH_LOG_TAG,
                "  #%02d %s+0x%lx (%s)", i, fname, offset,
                info.dli_sname ? info.dli_sname : "?");
        } else {
            __android_log_print(ANDROID_LOG_ERROR, CRASH_LOG_TAG,
                "  #%02d %p", i, buffer[i]);
        }
    }
}

static void crashHandler(int sig, siginfo_t* info, void* ctx) {
    __android_log_print(ANDROID_LOG_ERROR, CRASH_LOG_TAG,
        "========================================");
    __android_log_print(ANDROID_LOG_ERROR, CRASH_LOG_TAG,
        "!!! CRASH: signal %d (SIG%s), fault addr: %p, code: %d !!!",
        sig,
        sig == SIGSEGV ? "SEGV" : sig == SIGABRT ? "ABRT" :
        sig == SIGBUS ? "BUS" : sig == SIGILL ? "ILL" : "???",
        info->si_addr, info->si_code);
    __android_log_print(ANDROID_LOG_ERROR, CRASH_LOG_TAG,
        "========================================");

    logBacktrace();

    __android_log_print(ANDROID_LOG_ERROR, CRASH_LOG_TAG,
        "========================================");

    // 恢复默认处理器，重新触发信号（让系统生成 tombstone）
    signal(sig, SIG_DFL);
    raise(sig);
}

static void installCrashHandler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = crashHandler;
    sa.sa_flags = SA_SIGINFO;  // 移除SA_ONSTACK：需要sigaltstack()前置，否则信号处理器静默失效
    sigemptyset(&sa.sa_mask);

    sigaction(SIGSEGV, &sa, nullptr);   // 段错误
    sigaction(SIGABRT, &sa, nullptr);   // abort()
    sigaction(SIGBUS, &sa, nullptr);    // 总线错误
    sigaction(SIGILL, &sa, nullptr);    // 非法指令

    __android_log_print(ANDROID_LOG_INFO, CRASH_LOG_TAG, "崩溃处理器已安装");
}
// ====== 崩溃信号捕获结束 ======

// 构造函数：启动钩子线程
extern "C" void __attribute__((constructor)) lib_main() {
    installCrashHandler();
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}
