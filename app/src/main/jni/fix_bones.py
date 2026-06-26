with open('hooks.h', 'r', encoding='utf-8') as f:
    content = f.read()

# 1. Disable Update hook only
old1 = """    // Hook GameManager.Update（主循环回调，三级后备）
    void* updateAddr = GET_METHOD("Assembly-CSharp.dll", "", "GameManager", "Update", 0);
    if (!updateAddr) {
        updateAddr = GET_METHOD("Assembly-CSharp.dll", "", "NpcControl", "Update", 0);
    }
    if (!updateAddr) {
        updateAddr = GET_METHOD("Assembly-CSharp.dll", "", "GameLibrary", "Update", 0);
    }
    if (updateAddr && !old_Update) {
        DobbyHook(updateAddr, (void*)Update, (void**)&old_Update);
        HOOK_LOGI("✅ Update Hook: %p", updateAddr);
    } else if (!updateAddr) {
        HOOK_LOGE("❌ Update Hook 失败");
    }"""
new1 = """    // 【调试禁用以排查崩溃】
    HOOK_LOGI("Update Hook 禁用中...");
    /*
    void* updateAddr = GET_METHOD("Assembly-CSharp.dll", "", "GameManager", "Update", 0);
    if (!updateAddr) {
        updateAddr = GET_METHOD("Assembly-CSharp.dll", "", "NpcControl", "Update", 0);
    }
    if (!updateAddr) {
        updateAddr = GET_METHOD("Assembly-CSharp.dll", "", "GameLibrary", "Update", 0);
    }
    if (updateAddr && !old_Update) {
        DobbyHook(updateAddr, (void*)Update, (void**)&old_Update);
        HOOK_LOGI("✅ Update Hook: %p", updateAddr);
    }
    */"""
content = content.replace(old1, new1)

# 2. Disable FireBullet hooks
old2 = """    // Hook Weapon.FireBullet（dump：1参数 Vector3 _shootDir，来自Assembly-CSharp.dll Weapon类）
    void* fireBulletAddr = GET_METHOD("Assembly-CSharp.dll", "", "Weapon", "FireBullet", 1);
    if (fireBulletAddr && !old_FireBullet) {
        DobbyHook(fireBulletAddr, (void*)Hooked_FireBullet, (void**)&old_FireBullet);
        HOOK_LOGI("✅ Weapon.FireBullet: %p", fireBulletAddr);
    } else if (!fireBulletAddr) {
        HOOK_LOGE("❌ Weapon.FireBullet 未找到");
    }

    // Hook NpcControl.FireBullet（dump：1参数 Vector3，NPC射击入口）
    void* npcFireBulletAddr = GET_METHOD("Assembly-CSharp.dll", "", "NpcControl", "FireBullet", 1);
    if (npcFireBulletAddr && !old_NpcFireBullet) {
        DobbyHook(npcFireBulletAddr, (void*)Hooked_NpcFireBullet, (void**)&old_NpcFireBullet);
        HOOK_LOGI("✅ NpcControl.FireBullet: %p", npcFireBulletAddr);
    } else if (!npcFireBulletAddr) {
        HOOK_LOGE("❌ NpcControl.FireBullet 未找到");
    }"""
new2 = """    // 【调试禁用以排查崩溃】
    HOOK_LOGI("FireBullet Hook 禁用中...");
    /*
    void* fireBulletAddr = GET_METHOD("Assembly-CSharp.dll", "", "Weapon", "FireBullet", 1);
    if (fireBulletAddr && !old_FireBullet) {
        DobbyHook(fireBulletAddr, (void*)Hooked_FireBullet, (void**)&old_FireBullet);
    }
    void* npcFireBulletAddr = GET_METHOD("Assembly-CSharp.dll", "", "NpcControl", "FireBullet", 1);
    if (npcFireBulletAddr && !old_NpcFireBullet) {
        DobbyHook(npcFireBulletAddr, (void*)Hooked_NpcFireBullet, (void**)&old_NpcFireBullet);
    }
    */"""
content = content.replace(old2, new2)

# 3. Undo bone comment (keep bones working)
old3 = """        // 【调试】骨骼收集禁用以排查崩溃
        /*
        // 骨骼数据收集（过滤队友）"""
new3 = """        // 骨骼数据收集（过滤队友）"""
content = content.replace(old3, new3)

old4 = """            std::lock_guard<std::mutex> boneLock(g_BoneInfoMutex);
            g_BoneInfos = newBoneInfos;
        }
        */"""
new4 = """            std::lock_guard<std::mutex> boneLock(g_BoneInfoMutex);
            g_BoneInfos = newBoneInfos;
        }"""
content = content.replace(old4, new4)

with open('hooks.h', 'w', encoding='utf-8') as f:
    f.write(content)
print("Done: Update+FireBullet hooks disabled, bones kept enabled")
