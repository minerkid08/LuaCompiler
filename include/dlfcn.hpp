#pragma once

#define RTLD_GLOBAL 0x100 /* do not hide entries in this module */
#define RTLD_LOCAL 0x000  /* hide entries in this module */

#define RTLD_LAZY 0x000 /* accept unresolved externs */
#define RTLD_NOW 0x001	/* abort if module has unresolved externs */

void* dlopen(const char* filename, int flag);
int dlclose(void* handle);

void* dlsym(void* handle, const char* name);

const char* dlerror(void);
