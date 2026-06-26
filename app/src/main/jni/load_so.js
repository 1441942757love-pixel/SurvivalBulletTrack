Java.perform(function () {
    var soPath = "/data/local/tmp/azhuruqi/libDarkTeam.so";
    var RTLD_NOW = 2;
    var dlopen = new NativeFunction(
        Module.findExportByName(null, "dlopen"),
        'pointer', ['pointer', 'int']
    );
    var ptrPath = Memory.allocUtf8String(soPath);
    var handle = dlopen(ptrPath, RTLD_NOW);
    console.log("[*] dlopen result: " + handle);
    if (!handle.isNull()) {
        console.log("[+] libDarkTeam.so injected OK!");
    } else {
        console.log("[!] dlopen failed!");
    }
});
