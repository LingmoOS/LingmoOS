/*
    SPDX-FileCopyrightText: 2022 Aleix Pol i Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef DMABUFHANDLER_H
#define DMABUFHANDLER_H

#include "kpipewiredmabuf_export.h"
#include "pipewiresourcestream.h"
#include <QImage>
#include <epoxy/egl.h>

class KPIPEWIREDMABUF_EXPORT DmaBufHandler
{
public:
    DmaBufHandler();
    ~DmaBufHandler();

    bool downloadFrame(QImage &image, const PipeWireFrame &frame);

private:
    void setupEgl();
    bool m_eglInitialized = false;
    qint32 m_drmFd = 0; // for GBM buffer mmap
    gbm_device *m_gbmDevice = nullptr; // for passed GBM buffer retrieval

    struct EGLStruct {
        EGLDisplay display = EGL_NO_DISPLAY;
        EGLContext context = EGL_NO_CONTEXT;
    };
    EGLStruct m_egl;
};

#endif // DMABUFHANDLER_H
