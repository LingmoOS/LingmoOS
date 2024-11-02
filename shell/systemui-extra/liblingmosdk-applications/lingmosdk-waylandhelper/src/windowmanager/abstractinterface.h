/*
 * liblingmosdk-waylandhelper's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef ABSTRACTINTERFACE_H
#define ABSTRACTINTERFACE_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include "windowinfo.h"
#include "outputinfo.h"
#include "netwm.h"

namespace kdk
{
using WindowId = QVariant;
class KWindows;

enum class WindowLayer {
    Normal,
    Desktop,
    Panel,
    OnScreenDisplay,
    Notification,
    ToolTip,
    CriticalNotification,
    AppletPop,
    ScreenLock,
    Watermark,
    SystemWindow,
    InputPanel,
    Logout,
    ScreenLockNotification,
    Switcher
};

class AbstractInterface:public QObject
{
    Q_OBJECT

public:
    AbstractInterface(QObject* parent=nullptr);
    virtual ~AbstractInterface();
    virtual WindowInfo requestInfo(WindowId wid) = 0;

    virtual void requestActivate(WindowId wid)  = 0;
    virtual void requestClose(WindowId wid)  = 0;
    virtual void requestToggleKeepAbove(WindowId wid)  = 0;
    virtual void requestToggleMinimized(WindowId wid)  = 0;
    virtual void requestToggleMaximized(WindowId wid)  = 0;
    virtual WindowId activeWindow()  = 0;

    virtual QIcon iconFor(WindowId wid)  = 0;
    virtual QString titleFor(WindowId wid)  = 0;
    virtual QString windowGroupFor(WindowId wid) = 0;

    virtual bool windowCanBeDragged(WindowId wid)  = 0;
    virtual bool windowCanBeMaximized(WindowId wid)  = 0;


    virtual void showCurrentDesktop()  = 0;
    virtual void hideCurrentDesktop()  = 0;
    virtual quint32 pid(WindowId wid) = 0;

    virtual void setGeometry(QWindow *window, const QRect &rect) = 0;
    virtual NET::WindowType windowType(WindowId wid) = 0;

    virtual void setSkipTaskBar(QWindow* window,bool skip) = 0;
    virtual void setSkipSwitcher(QWindow* window,bool skip) = 0;

    virtual bool skipTaskBar(const WindowId &wid) = 0;
    virtual bool skipSwitcher(const WindowId &wid) = 0;

    virtual bool isShowingDesktop() = 0;
    virtual void setOnAllDesktops(const WindowId &wid) = 0;

    bool inCurrentDesktopActivity(const WindowInfo &winfo) ;
    bool isPlasmaDesktop(const QRect &wGeometry);
    QString currentDesktop() ;
    QString currentActivity() ;

    void setPlasmaDesktop(WindowId wid);
    bool isValidFor(const WindowId &wid) ;

    QList<WindowId> windows();
    virtual void setPanelTakefocus(QWindow *window, bool flag) = 0;
    virtual void demandAttention(const WindowId &wid) = 0;
    virtual QString getProcessName(const WindowId &wid);
    virtual bool setWindowRadius(QWindow *windowHandle,int radius);
    virtual void activateWindow(QWindow*window1,QWindow*window2);
    virtual void setIdleInterval(int msec);
    virtual QRect windowGeometry(const WindowId& windowId);
    virtual void setPanelAutoHide(QWindow *window, bool autoHide);
    virtual void setGrabKeyboard(QWindow *window, bool grabKeyboard);
    virtual void setWindowLayer(QWindow *window, WindowLayer layer);
    virtual WindowLayer windowLayer(QWindow *window);
    virtual void setHighlight(const WindowId &wid, bool highlight);
    virtual bool istHighlight(const WindowId &wid);
    virtual void setOpenUnderCursor(QWindow *window);
    virtual void setOpenUnderCursor(QWindow *window, int x, int y);
    virtual void setIconName(QWindow *window, const QString& iconName);
    virtual QString currentSeatName();
    virtual QString currentOutputName();
    virtual QList<OutputInfo*> outputsInfo();
    virtual QList<WindowId> getWindowIdByPid(quint32 pid);
    virtual QList<WindowId> getWindowIdByTtile(const QString& title);

Q_SIGNALS:
    void activeWindowChanged(WindowId wid);
    void windowChanged(WindowId winfo);
    void windowAdded(WindowId wid);
    void windowRemoved(WindowId wid);
    void currentDesktopChanged();
    void isShowingDesktopChanged();

    //since 2.3
    void titleChanged(WindowId wid);
    void iconChanged(WindowId wid);
    void activeChanged(WindowId wid);
    void fullscreenChanged(WindowId wid);
    void keepAboveChanged(WindowId wid);
    void minimizedChanged(WindowId wid);
    void maximizedChanged(WindowId wid);
    void onAllDesktopsChanged(WindowId wid);
    void demandsAttentionChanged(WindowId wid);
    void skipTaskbarChanged(WindowId wid);
    void skipSwitcherChanged(WindowId wid);
    void geometryChanged(WindowId wid);

    //since 2.4
    void idled();
    void resumed();

    //since 2.5
    void outputInfoChanged(OutputInfo*);

public:
    QMap<WindowId, WindowInfo> m_windows;
    QString m_currentDesktop;
    QString m_currentActivity;
};
}

#endif // ABSTRACTINTERFACE_H
