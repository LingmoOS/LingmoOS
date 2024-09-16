/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2024 jccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <gbm.h>

typedef gbm_bo *(*CreateWithModifiers)(struct gbm_device *gbm,
                                       uint32_t width, uint32_t height,
                                       uint32_t format,
                                       const uint64_t *modifiers,
                                       const unsigned int count);

class GbmLoader
{
private:
    GbmLoader();
    ~GbmLoader();

    GbmLoader(const GbmLoader &loader);
    const GbmLoader &operator=(const GbmLoader &loader);

private:
    static GbmLoader *g_gbmLoader;
    void *m_gbmHandle = nullptr;

public:
    static GbmLoader *loader();
    static void release();

    CreateWithModifiers createWithModifiers{nullptr};
};