// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "setproctitle.h"

#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <cstdint>

// https://github.com/nginx/nginx/blob/master/src/os/unix/ngx_setproctitle.c

static char** os_argv;
static char* os_argv_last;

int init_setproctitle(char* argv[], char* environ[])
{
    os_argv = argv;
    char *p;
    int i = 0;
    uint32_t size = 0;
    for (i = 0; environ[i]; i++) {
        size += strlen(environ[i]) + 1;
    }

    p = (char*)malloc(size);
    if (p == NULL) {
        return PROCTITLE_ERROR;
    }

    os_argv_last = os_argv[0];
    for (i = 0; os_argv[i]; i++) {
        if (os_argv_last == os_argv[i]) {
            os_argv_last = os_argv[i] + strlen(os_argv[i]) + 1;
        }
    }

    for (i = 0; environ[i]; i++) {
        if (os_argv_last == environ[i]) {
            size = strlen(environ[i]) + 1;
            os_argv_last = environ[i] + size;
            strncpy(p, environ[i], size);
            environ[i] = (char *) p;
            p += size;
        }
    }
    os_argv_last--;

    return PROCTITLE_OK;
}

void setproctitle(const char *title)
{
    os_argv[1] = NULL;
    strncpy(os_argv[0], title, os_argv_last - os_argv[0]);
}
