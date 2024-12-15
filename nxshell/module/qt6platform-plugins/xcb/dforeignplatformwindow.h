// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFOREIGNPLATFORMWINDOW_H
#define DFOREIGNPLATFORMWINDOW_H

#include "global.h"

#include <QtGlobal>

#ifdef Q_OS_LINUX
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <any>
#endif
#define private public
#include "qxcbwindow.h"
typedef QXcbWindow QNativeWindow;
#undef private
#elif defined(Q_OS_WIN)
#include "qwindowswindow.h"
typedef QWindowsWindow QNativeWindow;
#endif

DPP_BEGIN_NAMESPACE

class DForeignPlatformWindow : public QNativeWindow
{
public:
    explicit DForeignPlatformWindow(QWindow *window, WId winId);
    ~DForeignPlatformWindow();

    QRect geometry() const Q_DECL_OVERRIDE;
    QMargins frameMargins() const override;

#ifdef Q_OS_LINUX
    void handleConfigureNotifyEvent(const xcb_configure_notify_event_t *) override;
    void handlePropertyNotifyEvent(const xcb_property_notify_event_t *) override;

    QNativeWindow *toWindow() override;
#endif

private:
    void create() override;
    void destroy() override;

#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    bool isForeignWindow() const override {
        return true;
    }
#else
    QPlatformScreen *screenForGeometry(const QRect &newGeometry) const;
#endif

    void updateTitle();
    void updateWmClass();
    void updateWmDesktop();
    void updateWindowState();
    void updateWindowTypes();
    void updateProcessId();

    void init();
};

DPP_END_NAMESPACE

#endif // DFOREIGNPLATFORMWINDOW_H
