#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include <string>

// The path to the file whose fopen calls we want to intercept
const char* DMI_PRODUCT_SERIAL = "/sys/class/dmi/id/product_serial";
const char* FAKE_SERIAL = "CHIMERA-USERLAND-SN\n";

typedef FILE* (*original_fopen_t)(const char*, const char*);

FILE* fopen(const char* path, const char* mode) {
    if (strcmp(path, DMI_PRODUCT_SERIAL) == 0) {
        // Create an in-memory file
        int fd = memfd_create("fake_serial", 0);
        if (fd == -1) {
            return NULL; // Could not create in-memory file
        }
        
        // Write our fake serial to it
        write(fd, FAKE_SERIAL, strlen(FAKE_SERIAL));
        lseek(fd, 0, SEEK_SET); // Rewind to the beginning
        
        // Open the in-memory file descriptor as a stream
        char fd_path[256];
        snprintf(fd_path, sizeof(fd_path), "/proc/self/fd/%d", fd);
        
        // We still need the original fopen to open the memfd path
        original_fopen_t original_fopen = (original_fopen_t)dlsym(RTLD_NEXT, "fopen");
        FILE* stream = original_fopen(fd_path, mode);
        
        close(fd); // The file stream now has its own reference
        return stream;
    }

    original_fopen_t original_fopen = (original_fopen_t)dlsym(RTLD_NEXT, "fopen");
    return original_fopen(path, mode);
} 