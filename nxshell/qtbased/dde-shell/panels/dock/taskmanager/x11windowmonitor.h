// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "abstractwindow.h"
#include "appitem.h"
#include "dsglobal.h"
#include "x11window.h"
#include "x11preview.h"
#include "abstractwindowmonitor.h"

#include <cstdint>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include <QHash>
#include <QScopedPointer>
#include <QAbstractNativeEventFilter>

namespace dock {
class XcbEventFilter: public QAbstractNativeEventFilter
{
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;
};

class X11WindowMonitor : public AbstractWindowMonitor
{
    Q_OBJECT

public:
    explicit X11WindowMonitor(QObject* parent = nullptr);
    virtual void start() override;
    virtual void stop() override;
    virtual void clear() override;

    virtual QPointer<AbstractWindow> getWindowByWindowId(ulong windowId) override;
    virtual void presentWindows(QList<uint32_t> windows) override;
    virtual void showItemPreview(const QPointer<AppItem> &item, QObject* relativePositionItem, int32_t previewXoffset, int32_t previewYoffset, uint32_t direction) override;
    virtual void hideItemPreview() override;
    void previewWindow(uint32_t winId);
    void cancelPreviewWindow();

Q_SIGNALS:
    void windowMapped(xcb_window_t window);
    void windowDestoried(xcb_window_t window);
    void windowPropertyChanged(xcb_window_t window, xcb_atom_t atom);

private Q_SLOTS:
    void onWindowMapped(xcb_window_t window);
    void onWindowDestoried(xcb_window_t window);
    void onWindowPropertyChanged(xcb_window_t window, xcb_atom_t atom);

private:
    void monitorX11Event();
    void handleRootWindowPropertyNotifyEvent(xcb_atom_t atom);
    void handleRootWindowClientListChanged();

private:
    xcb_window_t m_rootWindow;
    QScopedPointer<XcbEventFilter> m_xcbEventFilter;
    QScopedPointer<X11WindowPreviewContainer> m_windowPreview;
    QHash<xcb_window_t, QSharedPointer<X11Window>> m_windows;
};
}
