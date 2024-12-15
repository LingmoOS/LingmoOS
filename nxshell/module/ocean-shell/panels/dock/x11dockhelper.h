// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dockhelper.h"

#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xproto.h>

namespace dock {
class X11DockWakeUpArea;
class X11DockHelper;
struct WindowData;

class XcbEventFilter: public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    XcbEventFilter(X11DockHelper* helper);
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;

    xcb_atom_t getAtomByName(const QString& name);
    QString getNameByAtom(const xcb_atom_t& atom);
    QList<xcb_window_t> getWindowClientList();
    QList<xcb_atom_t> getWindowState(const xcb_window_t& window);
    QList<xcb_atom_t> getWindowTypes(const xcb_window_t& window);
    QRect getWindowGeometry(const xcb_window_t& window);
    xcb_window_t getDecorativeWindow(const xcb_window_t& window);
    uint32_t getWindowWorkspace(const xcb_window_t& window);
    uint32_t getCurrentWorkspace();
    void checkCurrentWorkspace();
    bool shouldSkip(const xcb_window_t& window);
    void monitorWindowChange(const xcb_window_t& window);
    void setWindowState(const xcb_window_t& window, uint32_t list_len, xcb_atom_t *state);

Q_SIGNALS:
    void windowClientListChanged();
    void windowPropertyChanged(xcb_window_t window, xcb_atom_t atom);
    void windowGeometryChanged(xcb_window_t window);
    void currentWorkspaceChanged();

private:
    bool inTriggerArea(xcb_window_t win) const;
    void processEnterLeave(xcb_window_t win, bool enter);

    QPointer<X11DockHelper> m_helper;
    QMap<QString, xcb_atom_t> m_atoms;
    xcb_connection_t* m_connection;
    xcb_window_t m_rootWindow;
    xcb_ewmh_connection_t m_ewmh;
    uint32_t m_currentWorkspace;
};

class X11DockHelper : public DockHelper
{
    Q_OBJECT

public:
    X11DockHelper(DockPanel *panel);

protected:
    bool currentActiveWindowFullscreened() override;
    bool isWindowOverlap() override;

    [[nodiscard]] DockWakeUpArea *createArea(QScreen *screen) override;
    void destroyArea(DockWakeUpArea *area) override;

private Q_SLOTS:
    void onHideModeChanged(HideMode mode);

    void onWindowClientListChanged();
    void onWindowAoceand(xcb_window_t window);
    void onWindowPropertyChanged(xcb_window_t window, xcb_atom_t atom);
    void onWindowGeometryChanged(xcb_window_t window);
    void onWindowWorkspaceChanged(xcb_window_t window);

    void updateWindowHideState(xcb_window_t window);

    void updateDockArea();

private:
    friend class XcbEventFilter;

private:
    QHash<xcb_window_t, X11DockWakeUpArea *> m_areas;
    QRect m_dockArea;
    QHash<xcb_window_t, WindowData*> m_windows;
    XcbEventFilter *m_xcbHelper;
    QTimer *m_updateDockAreaTimer;
};

class X11DockWakeUpArea : public QObject, public DockWakeUpArea
{
    Q_OBJECT

private:
    X11DockWakeUpArea(QScreen *screen, X11DockHelper *helper);
    ~X11DockWakeUpArea() override;

    void open() override;
    void close() override;
    void updateDockWakeArea(Position pos) override;

private:
    friend class X11DockHelper;
    xcb_window_t m_triggerWindow;
    xcb_window_t m_rootWindow;
    xcb_connection_t *m_connection;
};
}

