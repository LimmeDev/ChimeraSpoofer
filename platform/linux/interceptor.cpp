#ifdef __linux__

#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>

// Original fopen function pointer
static FILE* (*original_fopen)(const char*, const char*) = NULL;

// Original uname function pointer
static int (*original_uname)(struct utsname*) = NULL;

extern "C" {

FILE* fopen(const char* pathname, const char* mode) {
    if (original_fopen == NULL) {
        original_fopen = (FILE* (*)(const char*, const char*))dlsym(RTLD_NEXT, "fopen");
    }

    // Intercept reads to DMI files to return fake serial numbers
    if (strcmp(pathname, "/sys/class/dmi/id/product_serial") == 0 ||
        strcmp(pathname, "/sys/class/dmi/id/board_serial") == 0 ||
        strcmp(pathname, "/sys/class/dmi/id/chassis_serial") == 0) {
        const char* fake_serial = "CHIMERA-SN-1337\n";
        // Use fmemopen to return a read-only memory buffer instead of a real file
        return fmemopen((void*)fake_serial, strlen(fake_serial), "r");
    }

    if (strcmp(pathname, "/sys/class/dmi/id/product_uuid") == 0) {
        const char* fake_uuid = "00000000-0000-0000-0000-000000000000\n";
        return fmemopen((void*)fake_uuid, strlen(fake_uuid), "r");
    }

    // If it's not a file we want to intercept, call the original fopen
    return original_fopen(pathname, mode);
}

int uname(struct utsname *buf) {
    if (original_uname == NULL) {
        original_uname = (int (*)(struct utsname*))dlsym(RTLD_NEXT, "uname");
    }

    // Call the original uname first to get the real system info
    int ret = original_uname(buf);
    
    // If the original call was successful, overwrite the release field
    if (ret == 0) {
        strcpy(buf->release, "5.4.0-chimera");
        strcpy(buf->version, "#1 SMP ChimeraOS");
    }

    return ret;
}

} // extern "C"

#endif // __linux__ 