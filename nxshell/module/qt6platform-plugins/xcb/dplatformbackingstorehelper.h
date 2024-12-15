// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFORMBACKINGSTOREHELPER_H
#define DPLATFORMBACKINGSTOREHELPER_H

#include <QtGlobal>

#include "global.h"

QT_BEGIN_NAMESPACE
class QPlatformBackingStore;
class QWindow;
class QRegion;
class QPoint;
class QPaintDevice;
class QSize;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class DPlatformBackingStoreHelper
{
public:
    DPlatformBackingStoreHelper();

    bool addBackingStore(QPlatformBackingStore *store);

    QPlatformBackingStore *backingStore() const
    { return reinterpret_cast<QPlatformBackingStore*>(const_cast<DPlatformBackingStoreHelper*>(this));}

    QPaintDevice *paintDevice();
    void beginPaint(const QRegion &);
    void flush(QWindow *window, const QRegion &region, const QPoint &offset);
#ifdef Q_OS_LINUX
    void resize(const QSize &size, const QRegion &staticContents);
#endif
};

DPP_END_NAMESPACE

#endif // DPLATFORMBACKINGSTOREHELPER_H
