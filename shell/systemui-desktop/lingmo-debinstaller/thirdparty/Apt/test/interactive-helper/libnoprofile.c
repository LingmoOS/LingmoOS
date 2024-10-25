#define _GNU_SOURCE
#undef _FORTIFY_SOURCE
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>

int vprintf(const char *format, va_list ap) {
    if (strncmp(format, "profiling:", strlen("profiling:")) == 0)
        return 0;

    static int (*func_fprintf)(const char *format, va_list ap) = NULL;
    if (func_fprintf == NULL)
       func_fprintf = (int (*)(const char *format, va_list ap))dlsym(RTLD_NEXT, "vprintf");

    return func_fprintf(format, ap);
}
int printf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int res = vprintf(format, ap);
    va_end(ap);
    return res;
}

int vfprintf(FILE *stream, const char *format, va_list ap) {
    if (strncmp(format, "profiling:", strlen("profiling:")) == 0)
        return 0;

    static int (*func_vfprintf)(FILE * stream, const char *format, va_list ap) = NULL;
    if (func_vfprintf == NULL)
       func_vfprintf = (int (*)(FILE * stream, const char *format, va_list ap))dlsym(RTLD_NEXT, "vfprintf");

    return func_vfprintf(stream, format, ap);
}

int fprintf(FILE *stream, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int res = vfprintf(stream, format, ap);
    va_end(ap);
    return res;
}
