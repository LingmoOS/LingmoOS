// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dlfcn.h>
#include <iostream>

#include "event_sdk.h"

using namespace std;

typedef bool (*Initialize)(const std::string &packagename, bool enable_sig);
typedef void (*WriteEventLog)(const std::string &eventdata);

WriteEventLog fp_writeEventLog = nullptr;
void *handler = nullptr;

int InitEventSDK() {
    handler = dlopen("/usr/lib/libdeepin-event-log.so", RTLD_LAZY);
    if (handler == NULL) {
        std::cout << "dlerror = " << dlerror() << std::endl;
        return -1;
    }
    auto fp_initialize = (Initialize)dlsym(handler, "Initialize");
    fp_writeEventLog = (WriteEventLog)dlsym(handler, "WriteEventLog");
    bool ret = fp_initialize("org.deepin.dde.daemon", false);
    if (!ret) {
        dlclose(handler);
        fp_initialize = nullptr;
        fp_writeEventLog = nullptr;
        return -1;
    }
    return 0;
}

void CloseEventLog() {
    dlclose(handler);
}

void writeEventLog(const char *log,int isDebug) {
    if (isDebug) {
        cout << log << endl;
    }
    if (fp_writeEventLog) fp_writeEventLog(log);
}