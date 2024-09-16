// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DHIGHDPI_H
#define DHIGHDPI_H

#include "global.h"

#include <QPointF>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformbackingstore.h>

QT_BEGIN_NAMESPACE
class QWindow;
class QXcbScreen;
class QXcbVirtualDesktop;
class QPlatformWindow;
class QXcbBackingStore;
QT_END_NAMESPACE

typedef struct xcb_connection_t xcb_connection_t;

DPP_BEGIN_NAMESPACE

class DHighDpi
{
public:
    static QPointF fromNativePixels(const QPointF &pixelPoint, const QWindow *window);

    static void init();
    static bool isActive();
    static bool overrideBackingStore();
    static QDpi logicalDpi(QXcbScreen *s);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    static qreal pixelDensity(QXcbScreen *s);
#endif
    static qreal devicePixelRatio(QPlatformWindow *w);
    static void onDPIChanged(xcb_connection_t *screen, const QByteArray &name, const QVariant &property, void *handle);

private:

    static bool active;
    static QHash<QPlatformScreen*, qreal> screenFactorMap;
};

DPP_END_NAMESPACE

#endif // DHIGHDPI_H
