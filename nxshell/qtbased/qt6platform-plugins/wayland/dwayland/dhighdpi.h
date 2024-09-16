// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DHIGHDPI_H
#define DHIGHDPI_H

#include "global.h"

#include <QPointF>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformwindow.h>
#include <qpa/qplatformbackingstore.h>

QT_BEGIN_NAMESPACE
class QWindow;
class QPlatformWindow;

namespace QtWaylandClient {
class QWaylandScreen;
}
QT_END_NAMESPACE

typedef struct xcb_connection_t xcb_connection_t;

DPP_BEGIN_NAMESPACE

class HighDpiImage;
class DHighDpi
{
public:
    static QPointF fromNativePixels(const QPointF &pixelPoint, const QWindow *window);

    static void init();
    static bool isActive();
    static bool overrideBackingStore();
    static QDpi logicalDpi(QtWaylandClient::QWaylandScreen *s);
    static qreal devicePixelRatio(QPlatformWindow *w);

    static void removeScreenFactorCache(QScreen *screen);

private:

    static bool active;
    static QDpi oldDpi;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    static QHash<QPlatformScreen*, qreal> screenFactorMap;
#endif
};

DPP_END_NAMESPACE

#endif // DHIGHDPI_H
