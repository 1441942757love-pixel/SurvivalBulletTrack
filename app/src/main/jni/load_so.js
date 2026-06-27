(function () {
    var srcPath = "/data/local/tmp/azhuruqi/libDarkTeam.so";
    var destPath = "/data/data/com.leaping.game.zdljb.m4399/files/libDarkTeam.so";
    var RTLD_NOW = 2;

    console.log("[*] Process arch: " + Process.arch);

    // 显式从 libc.so 获取函数
    var libc = Process.findModuleByName("libc.so");
    var fopen = new NativeFunction(Module.findExportByName("libc.so", "fopen"), "pointer", ["pointer", "pointer"]);
    var fread = new NativeFunction(Module.findExportByName("libc.so", "fread"), "int", ["pointer", "int", "int", "pointer"]);
    var fwrite = new NativeFunction(Module.findExportByName("libc.so", "fwrite"), "int", ["pointer", "int", "int", "pointer"]);
    var fclose = new NativeFunction(Module.findExportByName("libc.so", "fclose"), "int", ["pointer"]);

    // 确保目标目录存在
    var mkdir = new NativeFunction(Module.findExportByName("libc.so", "mkdir"), "int", ["pointer", "int"]);
    mkdir(Memory.allocUtf8String("/data/data/com.leaping.game.zdljb.m4399/files"), 448); // 0700

    // 原生拷贝
    var src = fopen(Memory.allocUtf8String(srcPath), Memory.allocUtf8String("rb"));
    if (src.isNull()) { console.log("[!] fopen src failed at: " + srcPath); return; }
    console.log("[*] src opened OK");

    var dst = fopen(Memory.allocUtf8String(destPath), Memory.allocUtf8String("wb"));
    if (dst.isNull()) { console.log("[!] fopen dst failed at: " + destPath); fclose(src); return; }
    console.log("[*] dst opened OK");

    var buf = Memory.alloc(8192);
    var total = 0, n;
    while ((n = fread(buf, 1, 8192, src)) > 0) {
        fwrite(buf, 1, n, dst);
        total += n;
    }
    fclose(src);
    fclose(dst);
    console.log("[*] copied " + total + " bytes to: " + destPath);

    // chmod 0755
    var chmod = new NativeFunction(Module.findExportByName("libc.so", "chmod"), "int", ["pointer", "int"]);
    chmod(Memory.allocUtf8String(destPath), 493);

    // dlopen
    var dlopen = new NativeFunction(Module.findExportByName(null, "dlopen"), "pointer", ["pointer", "int"]);
    var dlerror = new NativeFunction(Module.findExportByName(null, "dlerror"), "pointer", []);

    dlerror();
    var handle = dlopen(Memory.allocUtf8String(destPath), RTLD_NOW);
    console.log("[*] handle: " + handle);

    if (!handle.isNull()) {
        console.log("[+] libDarkTeam.so injected OK!");
    } else {
        console.log("[!] dlopen failed: " + dlerror().readCString());
    }
})();
