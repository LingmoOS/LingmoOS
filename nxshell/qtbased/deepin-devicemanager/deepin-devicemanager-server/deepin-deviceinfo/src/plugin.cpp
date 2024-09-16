// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainjob.h"

#include <QLoggingCategory>

static MainJob *mainJob = nullptr;

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)data;
    mainJob = new MainJob(name);
    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    mainJob->deleteLater();
    mainJob = nullptr;
    return 0;
}
