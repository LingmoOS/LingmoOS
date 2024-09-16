// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dockhelper.h"
#include "dsglobal.h"
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_ewmh.h>


namespace dock {
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
    QTimer *m_timer;
    QMap<QString, xcb_atom_t> m_atoms;
    xcb_connection_t* m_connection;
    xcb_window_t m_rootWindow;
    xcb_ewmh_connection_t m_ewmh;
    uint32_t m_currentWorkspace;
};

class DockTriggerArea : public QObject
{
    Q_OBJECT

public:
    DockTriggerArea(DockPanel *panel, X11DockHelper *helper, QScreen *qScreen);
    ~DockTriggerArea();

    xcb_window_t triggerWindow() const { return m_triggerWindow; }
    QScreen *screen() const { return m_screen; }
    void enableWakeArea();
    void disableWakeArea();
    void mouseEnter();
    void mouseLeave();
public Q_SLOTS:
    void updateDockTriggerArea();
    void onTriggerTimer();
    void onHoldingTimer();

private:
    const QRect matchDockTriggerArea();

    DockPanel *m_panel;
    X11DockHelper *m_helper;
    QScreen *m_screen;
    xcb_window_t m_triggerWindow;
    xcb_window_t m_rootWindow;
    xcb_connection_t *m_connection;
    bool m_enableWakeArea;

    QTimer* m_triggerTimer;
    QTimer* m_holdingTimer;
};


class X11DockHelper : public DockHelper
{
    Q_OBJECT

public:
    X11DockHelper(DockPanel* panel);
    HideState hideState() override;
    QList<DockTriggerArea*> triggerAreas() const { return m_areas; }

public Q_SLOTS:
    void updateDockTriggerArea() override;
    void updateEnterState(bool enter);

private Q_SLOTS:
    void updateHideState();
    void onHideModeChanged(HideMode mode);

    void onWindowClientListChanged();
    void onWindowAdded(xcb_window_t window);
    void onWindowPropertyChanged(xcb_window_t window, xcb_atom_t atom);
    void onWindowGeometryChanged(xcb_window_t window);
    void onWindowWorkspaceChanged(xcb_window_t window);

    void updateWindowHideState(xcb_window_t window);
    void updateSmartHideState(const dock::HideState &state);
    void updateDockHideState();
    void delayedUpdateState();
    void updateDockArea();
    void updateWindowState();

private:
    inline void updateWakeArea();

private:
    friend class XcbEventFilter;

private:
    dock::HideState m_hideState;
    QString m_registerKey;
    QList<DockTriggerArea*> m_areas;
    QRect m_dockArea;
    bool m_needUpdateState;
    bool m_enter;
    dock::HideState m_smartHideState;
    QHash<xcb_window_t, WindowData*> m_windows;
    XcbEventFilter *m_xcbHelper;
};
}

