// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFORMNATIVEINTERFACE_H
#define DPLATFORMNATIVEINTERFACE_H

#include "global.h"

#include <QtGlobal>
#include <qwindowdefs.h>

QT_BEGIN_NAMESPACE
class QObject;
class QWindow;
class QPointF;
class QVariant;
class QPlatformNativeInterface;
QT_END_NAMESPACE

typedef struct xcb_connection_t xcb_connection_t;

DPP_BEGIN_NAMESPACE

class DXcbXSettings;
class DWaylandInterfaceHook
{
public:
    static DWaylandInterfaceHook *instance() {
        static DWaylandInterfaceHook *hook = new DWaylandInterfaceHook;
        return hook;
    }
    static QFunctionPointer platformFunction(QPlatformNativeInterface *interface, const QByteArray &function);

    static bool buildNativeSettings(QObject *object, quint32 settingWindow);
    static void clearNativeSettings(quint32 settingWindow);

    static bool setEnableNoTitlebar(QWindow *window, bool enable);
    static bool isEnableNoTitlebar(QWindow *window);

    static void setWindowRadius(QWindow *window, int value);
    static void setBorderColor(QWindow *window, const QColor &value);
    static void setShadowColor(QWindow *window, const QColor &value);
    static void setShadowRadius(QWindow *window, int value);
    static void setShadowOffset(QWindow *window, const QPoint &value);
    static void setBorderWidth(QWindow *window, int value);
    static void setWindowEffect(QWindow *window, const QVariant &value);
    static void setWindowStartUpEffect(QWindow *window, const QVariant &value);

    static void setWindowProperty(QWindow *window, const char *name, const QVariant &value);
    static void popupSystemWindowMenu(quintptr wid);
    static bool enableDwayland(QWindow *window);
    static bool isEnableDwayland(const QWindow *window);
    static void splitWindowOnScreen(WId wid, quint32 type);
    static void splitWindowOnScreenByType(WId wid, quint32 position, quint32 type);
    static bool supportForSplittingWindow(WId wid);
    static bool supportForSplittingWindowByType(quint32 wid, quint32 screenSplittingType);
};

DPP_END_NAMESPACE

#endif // DPLATFORMNATIVEINTERFACE_H
