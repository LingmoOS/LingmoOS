/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2024 jccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "gbmloader.h"

#include <dlfcn.h>
#include <unistd.h>
#include <memory>

GbmLoader *GbmLoader::g_gbmLoader = new GbmLoader();

GbmLoader *GbmLoader::loader()
{
    return g_gbmLoader;
}

void GbmLoader::release()
{
    if (g_gbmLoader) {
        delete g_gbmLoader;
        g_gbmLoader = nullptr;
    }
}

GbmLoader::GbmLoader()
{
    m_gbmHandle = dlopen("libgbm.so", RTLD_LAZY | RTLD_LOCAL);
    if (m_gbmHandle) {
        *(void **)(&createWithModifiers) = dlsym(m_gbmHandle, "gbm_bo_create_with_modifiers");
    }
}

GbmLoader::~GbmLoader()
{
    if (m_gbmHandle) {
        dlclose(m_gbmHandle);
        m_gbmHandle = nullptr;
    }
}