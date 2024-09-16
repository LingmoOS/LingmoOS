// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dlfcn.h>
#include <stdio.h>
#include "ddcci_wrapper.h"
typedef int (*ddca_free_all_displays)();

ddca_free_all_displays fp_ddca_free_all_displays = NULL;

int InitDDCCISo(const char *path) {
    void *handler = dlopen(path, RTLD_LAZY);
    if (handler == NULL) {
        return -1;
    }
    fp_ddca_free_all_displays = (ddca_free_all_displays)dlsym(handler, "ddca_free_all_displays");
    if (fp_ddca_free_all_displays == NULL) {
        if (handler) {
            dlclose(handler);
        }
        return -2;
    }
    return 0;
}

int freeAllDisplaysWrapper() {
    if (fp_ddca_free_all_displays) {
        return fp_ddca_free_all_displays();
    }
}
