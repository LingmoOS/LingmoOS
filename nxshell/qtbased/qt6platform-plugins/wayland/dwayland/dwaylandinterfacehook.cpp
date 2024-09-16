// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#define private public
#include "QtWaylandClient/private/qwaylandnativeinterface_p.h"
#undef private

#include "dwaylandinterfacehook.h"
#include "dhighdpi.h"
#include "dxsettings.h"
#include "dnotitlebarwindowhelper_wl.h"

#include <qpa/qplatformnativeinterface.h>
#include <private/qguiapplication_p.h>
#include <QtWaylandClientVersion>

#include <QDebug>
#include <QLoggingCategory>

#ifndef QT_DEBUG
Q_LOGGING_CATEGORY(dwli, "dtk.wayland.interface" , QtInfoMsg);
#else
Q_LOGGING_CATEGORY(dwli, "dtk.wayland.interface");
#endif

DPP_BEGIN_NAMESPACE

static QFunctionPointer getFunction(const QByteArray &function)
{
    static QHash<QByteArray, QFunctionPointer> functionCache = {
        {buildNativeSettings, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::buildNativeSettings)},
        {clearNativeSettings, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::clearNativeSettings)},
        {setEnableNoTitlebar, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::setEnableNoTitlebar)},
        {isEnableNoTitlebar, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::isEnableNoTitlebar)},
        {setWindowRadius, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::setWindowRadius)},
        {setBorderColor, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::setBorderColor)},
        {setBorderWidth, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::setBorderWidth)},
        {setShadowColor, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::setShadowColor)},
        {setShadowOffset, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::setShadowOffset)},
        {setShadowRadius, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::setShadowRadius)},
        {setWindowEffect, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::setWindowEffect)},
        {setWindowStartUpEffect, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::setWindowStartUpEffect)},
        {setWindowProperty, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::setWindowProperty)},
        {popupSystemWindowMenu, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::popupSystemWindowMenu)},
        {enableDwayland, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::enableDwayland)},
        {isEnableDwayland, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::isEnableDwayland)},
        {splitWindowOnScreen, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::splitWindowOnScreen)},
        {supportForSplittingWindow, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::supportForSplittingWindow)},
        {splitWindowOnScreenByType, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::splitWindowOnScreenByType)},
        {supportForSplittingWindowByType, reinterpret_cast<QFunctionPointer>(&DWaylandInterfaceHook::supportForSplittingWindowByType)}
    };
    return functionCache.value(function);
}

QFunctionPointer DWaylandInterfaceHook::platformFunction(QPlatformNativeInterface *interface, const QByteArray &function)
{
    QFunctionPointer f = getFunction(function);

#if QTWAYLANDCLIENT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    if (Q_UNLIKELY(!f)) {
        f = static_cast<QtWaylandClient::QWaylandNativeInterface*>(interface)->QtWaylandClient::QWaylandNativeInterface::platformFunction(function);
    }
#endif
    return f;
}

bool DWaylandInterfaceHook::buildNativeSettings(QObject *object, quint32 settingWindow) {
    return dXSettings->buildNativeSettings(object, settingWindow);
}

void DWaylandInterfaceHook::clearNativeSettings(quint32 settingWindow) {
    dXSettings->clearNativeSettings(settingWindow);
}

bool DWaylandInterfaceHook::setEnableNoTitlebar(QWindow *window, bool enable)
{
    if (enable) {
        if (DNoTitlebarWlWindowHelper::mapped.value(window))
            return true;
        if (window->type() == Qt::Desktop)
            return false;
        window->setProperty(noTitlebar, true);
        Q_UNUSED(new DNoTitlebarWlWindowHelper(window))
        return true;
    } else {
        if (auto helper = DNoTitlebarWlWindowHelper::mapped.value(window)) {
            helper->deleteLater();
        }
        window->setProperty(noTitlebar, false);
    }

    return true;
}

bool DWaylandInterfaceHook::isEnableNoTitlebar(QWindow *window)
{
    return window->property(noTitlebar).toBool();
}

void DWaylandInterfaceHook::setWindowRadius(QWindow *window, int value)
{
    if (!window)
        return;
    DNoTitlebarWlWindowHelper::setWindowProperty(window, ::windowRadius, value);
}

void DWaylandInterfaceHook::setBorderColor(QWindow *window, const QColor &value)
{
    if (!window)
        return;
    DNoTitlebarWlWindowHelper::setWindowProperty(window, ::borderColor, value);
}

void DWaylandInterfaceHook::setShadowColor(QWindow *window, const QColor &value)
{
    if (!window)
        return;
    DNoTitlebarWlWindowHelper::setWindowProperty(window, ::shadowColor, value);
}

void DWaylandInterfaceHook::setShadowRadius(QWindow *window, int value)
{
    if (!window)
        return;
    DNoTitlebarWlWindowHelper::setWindowProperty(window, ::shadowRadius, value);
}

void DWaylandInterfaceHook::setShadowOffset(QWindow *window, const QPoint &value)
{
    if (!window)
        return;

    QPoint offect  = value;
    if (window->screen())
        offect *= window->screen()->devicePixelRatio();

    DNoTitlebarWlWindowHelper::setWindowProperty(window, ::shadowOffset, offect);
}

void DWaylandInterfaceHook::setBorderWidth(QWindow *window, int value)
{
    if (!window)
        return;
    DNoTitlebarWlWindowHelper::setWindowProperty(window, ::borderWidth, value);
}

void DWaylandInterfaceHook::setWindowEffect(QWindow *window, const QVariant &value)
{
    if (!window)
        return;
    DNoTitlebarWlWindowHelper::setWindowProperty(window, ::windowEffect, value);
}

void DWaylandInterfaceHook::setWindowStartUpEffect(QWindow *window, const QVariant &value)
{
    if (!window)
        return;
    DNoTitlebarWlWindowHelper::setWindowProperty(window, ::windowStartUpEffect, value);
}

void DWaylandInterfaceHook::setWindowProperty(QWindow *window, const char *name, const QVariant &value)
{
    DNoTitlebarWlWindowHelper::setWindowProperty(window, name, value);
}

void DWaylandInterfaceHook::popupSystemWindowMenu(WId wid)
{
    DNoTitlebarWlWindowHelper::popupSystemWindowMenu(wid);
}

bool DWaylandInterfaceHook::enableDwayland(QWindow *window)
{
    static bool xwayland = QByteArrayLiteral("wayland") == qgetenv("XDG_SESSION_TYPE")
            && !qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY");

    if (xwayland) {
        // for xwayland
        return false;
    }

    if (window->type() == Qt::Desktop)
        return false;

    QPlatformWindow *xw = static_cast<QPlatformWindow*>(window->handle());

    if (!xw) {
        window->setProperty(useDwayland, true);

        return true;
    }
    if (DNoTitlebarWlWindowHelper::mapped.value(window))
        return true;

    if (xw->isExposed())
        return false;

#ifndef USE_NEW_IMPLEMENTING
    return false;
#endif

    window->setProperty(useDwayland, true);
    // window->setProperty("_d_dwayland_TransparentBackground", window->format().hasAlpha());

    return true;
}

bool DWaylandInterfaceHook::isEnableDwayland(const QWindow *window)
{
    return window->property(useDwayland).toBool();
}

void DWaylandInterfaceHook::splitWindowOnScreen(WId wid, quint32 type)
{
    return splitWindowOnScreenByType(wid, 1, type);
}

void DWaylandInterfaceHook::splitWindowOnScreenByType(WId wid, quint32 position, quint32 type)
{
    QWindow *window = fromQtWinId(wid);
    if(!window || !window->handle())
        return;
    // position: 15 not preview
    if (position == 15) {
        if (window->windowStates().testFlag(Qt::WindowMaximized)) {
            window->showNormal();
        } else {
            window->showMaximized();
        }
    } else {
        // type 1:two splitting 2:three splitting 4:four splitting
        // position enum class SplitType in kwin-dev, Left=0x1, Right=0x10, Top=0x100, Bottom=0x1000
        QVariantList value{position, type};
        DNoTitlebarWlWindowHelper::setWindowProperty(window, ::splitWindowOnScreen, value);
    }
}

bool DWaylandInterfaceHook::supportForSplittingWindow(WId wid)
{
    return supportForSplittingWindowByType(wid, 1);
}

// screenSplittingType: 0: can't splitting, 1:two splitting, 2: four splitting(includ three splitting)
bool DWaylandInterfaceHook::supportForSplittingWindowByType(quint32 wid, quint32 screenSplittingType)
{
    QWindow *window = fromQtWinId(wid);
    if(!window || !window->handle())
        return false;
    DNoTitlebarWlWindowHelper::setWindowProperty(window, ::supportForSplittingWindow, false);
    return window->property(::supportForSplittingWindow).toInt() >= screenSplittingType;
}

DPP_END_NAMESPACE
