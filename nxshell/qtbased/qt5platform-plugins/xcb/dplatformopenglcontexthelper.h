// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPLATFORMOPENGLCONTEXTHELPER_H
#define DPLATFORMOPENGLCONTEXTHELPER_H

#include <QtGlobal>

#include "global.h"

QT_BEGIN_NAMESPACE
class QPlatformOpenGLContext;
class QPlatformSurface;
class QOpenGLContext;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class DPlatformOpenGLContextHelper
{
public:
    DPlatformOpenGLContextHelper();

    bool addOpenGLContext(QOpenGLContext *object, QPlatformOpenGLContext *context);

    QPlatformOpenGLContext *context() const
    { return reinterpret_cast<QPlatformOpenGLContext*>(const_cast<DPlatformOpenGLContextHelper*>(this));}

    void initialize();
    void swapBuffers(QPlatformSurface *surface);
};

DPP_END_NAMESPACE

#endif // DPLATFORMOPENGLCONTEXTHELPER_H
