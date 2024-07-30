/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "pipewiresourcestream.h"
#include <QByteArray>
#include <epoxy/egl.h>
#include <kpipewire_export.h>

typedef unsigned int GLenum;

namespace GLHelpers
{

KPIPEWIRE_EXPORT void initDebugOutput();
KPIPEWIRE_EXPORT QByteArray formatGLError(GLenum err);
KPIPEWIRE_EXPORT QByteArray formatEGLError(GLenum err);

KPIPEWIRE_EXPORT EGLImage createImage(EGLDisplay display, const DmaBufAttributes &attribs, uint32_t format, const QSize &size, gbm_device *device);
}
