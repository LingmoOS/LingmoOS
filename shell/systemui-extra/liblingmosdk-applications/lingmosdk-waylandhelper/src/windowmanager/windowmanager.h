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

#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <QString>
#include <QIcon>
#include "windowinfo.h"
#include "wmregister.h"

namespace kdk
{

/**
 * @defgroup WindowManagerModule
 */
using WindowId = QVariant;

/**
 * @brief 通常用于对窗口的一些操作
 * @since 1.2
 */
class WindowManager : public QObject
{
    Q_OBJECT

public:

    /**
     * @brief self
     * @return
     */
    static WindowManager* self();

    /**
      * @brief 获取窗口信息
      * @param windowId
      * @return
      */
    static WindowInfo getwindowInfo(const WindowId& windowId);

    /**
     * @brief 获取当前活动窗口
     * @return
     */
    static WindowId currentActiveWindow();

    /**
     * @brief 置顶窗口
     * @param windowId
     */
    static void keepWindowAbove(const WindowId& windowId);

    /**
     * @brief 获取窗口标题
     * @param windowId
     * @return
     */
     static QString getWindowTitle(const WindowId& windowId);

    /**
     * @brief 获取窗口图标
     * @param windowId
     * @return
     */
     static QIcon getWindowIcon(const WindowId& windowId);


     /**
      * @brief 获取窗口所在组的组名
      * @param windowId
      * @return
      */
     static QString getWindowGroup(const WindowId& windowId);


    /**
     * @brief 关闭窗口
     * @param windowId
     */
     static void closeWindow(const WindowId& windowId);

    /**
     * @brief 激活窗口
     * @param windowId
     */
     static void activateWindow(const WindowId& windowId);

    /**
     * @brief 最大化窗口
     * @param windowId
     */
     static void maximizeWindow(const WindowId& windowId);

    /**
     * @brief 最小化窗口
     * @param windowId
     */
     static void minimizeWindow(const WindowId& windowId);

     /**
      * @brief 获取窗口进程pid
      * @return
      */
     static quint32 getPid(const WindowId& windowId);

    /**
     * @brief 显示当前桌面
     */
     static void showDesktop();

    /**
     * @brief 取消显示当前桌面
     */
     static void hideDesktop();

     /**
     * @brief 获取当前桌面的名称
     * @return
     */
    static QString currentDesktop();

    /**
     * @brief 获取当前窗口列表
     * @return
     */
    static QList<WindowId> windows();

    /**
     * @brief 获取窗口类型，仅适用于X环境下，wayland下统一返回normal
     * @param windowId
     * @return
     */
    static NET::WindowType getWindowType(const WindowId& windowId);

    /**
     * @brief 设置窗口位置
     * @param window
     * @param rect
     */
    static void	setGeometry(QWindow *window,const QRect &rect);

    /**
     * @brief 设置是否跳过任务栏
     * @since 2.0
     * @param window
     * @param skip
     */
    static void setSkipTaskBar(QWindow *window,bool skip);

    /**
     * @brief 设置是否跳过窗口选择
     * @since 2.0
     * @param window
     * @param skip
     */
    static  void setSkipSwitcher(QWindow *window,bool skip);

    /**
     * @brief 判断窗体是否跳过任务栏
     * @since 2.0
     * @param windowId
     * @return
     */
    static bool skipTaskBar(const WindowId& windowId);

    /**
     * @brief 判断窗体是否跳过窗口选择
     * @since 2.0
     * @param windowId
     * @return
     */
    static bool skipSwitcher(const WindowId& windowId);

    /**
     * @brief 判断桌面是否处于显示状态
     * @since 2.0
     * @return
     */
    static bool isShowingDesktop();

    /**
     * @brief 设置窗口在所有桌面中显示
     * @since 2.0
     * @param wid
     */
    static void setOnAllDesktops(const WindowId &windowId);

    /**
     * @brief 判断窗口在所有桌面中显示
     * @since 2.0
     * @param windowId
     * @return
     */
    static bool isOnAllDesktops(const WindowId &windowId);

    /**
     * @brief 判断窗口是否在当前桌面
     * @param 窗口id
     * @since 2.1
     * @return
     */
    static bool isOnCurrentDesktop(const WindowId& id);

    /**
     * @brief 判断窗口是否在指定桌面
     * @param 窗口id
     * @param 桌面id
     * @since 2.1
     * @return
     */
    static bool isOnDesktop(const WindowId &id, int desktop);


    /**
     * @brief 设置panel属性窗体是否获取焦点，仅wayland环境下生效
     * @since 2.3
     * @param window
     * @param flag
     */
    static void setPanelTakefocus(QWindow *window, bool flag);
    
    /**
     * @brief 发送demandAttention状态
     * @since 2.3
     * @param wid
     */
    static void demandAttention(const WindowId &wid);

    /**
     * @brief 获取窗口所属的进程名称
     * @since 2.3
     * @param windowId
     * @return
     */
    static QString getProcessName(const WindowId& windowId);

    /**
     * @brief setWindowRadius
     * @since 2.5
     * @param window
     * @param radius
     */
    static void setWindowRadius(QWindow*window,int radius);

    /**
     * @brief activateWindow
     * @since 2.5
     * @param window1
     * @param window2
     */
    static void activateWindow(QWindow*window1,QWindow*window2);

    /**
     * @brief windowGeometry
     * @since 2.5
     * @param windowId
     * @return
     */
    static QRect windowGeometry(const WindowId& windowId);

    /**
     * @brief setPanelAutoHide
     * @since 2.5
     * @param window
     * @param autoHide
     */
    static void setPanelAutoHide(QWindow *window,bool autoHide);

    /**
     * @brief setGrabKeyboard
     * @since 2.5
     * @param window
     * @param grabKeyboard
     */
    static void setGrabKeyboard(QWindow *window, bool grabKeyboard);

    /**
     * @brief setWindowLayer
     * @since 2.5
     * @param window
     * @param layer
     */
    static void setWindowLayer(QWindow *window, WindowLayer layer);

    /**
     * @brief windowLayer
     * @since 2.5
     * @param window
     * @return
     */
    static WindowLayer windowLayer(QWindow *window);

    /**
     * @brief setHighlight
     * @since 2.5
     * @param wid
     * @param highlight
     */
    static void setHighlight(const WindowId &wid, bool highlight);

    /**
     * @brief isHightlight
     * @since 2.5
     * @param wid
     */
    static bool isHightlight(const WindowId &wid);

    /**
     * @brief setOpenUnderCursor
     * @since 2.5
     * @param window
     */
    static void setOpenUnderCursor(QWindow *window);

    /**
     * @brief setOpenUnderCursor
     * @since 2.5
     * @param window
     * @param x
     * @param y
     */
    static void setOpenUnderCursor(QWindow *window, int x, int y);

    /**
     * @brief setIconName
     * @since 2.5
     * @param window
     * @param iconName
     */
    static void setIconName(QWindow *window, const QString& iconName);

    /**
     * @brief currentSeatName
     * @since 2.5
     * @return
     */
    static QString currentSeatName();

    /**
     * @brief currentOutputName
     * @since 2.5
     * @return
     */
    static QString currentOutputName();

    /**
     * @brief outputsInfo
     * @since 2.5
     * @return
     */
    static QList<OutputInfo*> outputsInfo();

    /**
     * @brief getWindowIdByPid
     * @since 2.5
     * @return
     */
    static QList<WindowId> getWindowIdByPid(quint32 pid);

    /**
     * @brief getWindowIdByTtile
     * @since 2.5
     * @return
     */
    static QList<WindowId> getWindowIdByTtile(const QString&title);

Q_SIGNALS:
     /**
      * @brief 窗口添加信号
      * @param windowId
      */
     void windowAdded(const WindowId& windowId);

     /**
      * @brief 窗口删除信号
      * @param windowId
      */
     void windowRemoved(const WindowId& windowId);

     /**
      * @brief 活动窗口改变信号
      * @param wid
      */
     void activeWindowChanged(const WindowId& wid);

     /**
      * @brief 窗口改变信号
      * @param wid
      */
     void windowChanged(const WindowId& wid);

     /**
      * @brief 当前桌面改变信号
      * @param wid
      */
     void currentDesktopChanged();

     /**
      * @brief 桌面显示状态变化信号
      */
     void isShowingDesktopChanged();

     /**
      * @brief 窗口标题改变信号
      * @since 2.3
      * @param wid
      */
     void titleChanged(const WindowId& wid);
     
     /**
      * @brief 窗口图标改变信号
      * @since 2.3
      * @param wid
      */
     void iconChanged(const WindowId& wid);

     /**
      * @brief 窗口活动状态改变信号
      * @since 2.3
      * @param wid
      */
     void activeChanged(const WindowId& wid);

     /**
      * @brief 窗口全屏状态改变信号
      * @since 2.3
      * @param wid
      */
     void fullscreenChanged(const WindowId& wid);

     /**
      * @brief 窗口置顶状态改变信号
      * @since 2.3
      * @param wid
      */
     void keepAboveChanged(const WindowId& wid);

     /**
      * @brief 窗口最小化状态改变信号
      * @since 2.3
      * @param wid
      */
     void minimizedChanged(const WindowId& wid);

     /**
      * @brief 窗口最大化状态改变信号
      * @since 2.3
      * @param wid
      */
     void maximizedChanged(const WindowId& wid);

     /**
      * @brief onAllDesktopsChanged
      * @since 2.3
      * @param wid
      */
     void onAllDesktopsChanged(const WindowId& wid);

     /**
      * @brief demandsAttention状态改变信号
      * @since 2.3
      * @param wid
      */
     void demandsAttentionChanged(const WindowId& wid);

     /**
      * @brief 跳过任务栏状态改变信号
      * @since 2.3
      * @param wid
      */
     void skipTaskbarChanged(const WindowId& wid);

     /**
      * @brief 跳过窗口选择器状态改变信号
      * @since 2.3
      * @param wid
      */
     void skipSwitcherChanged(const WindowId& wid);

     /**
      * @brief 窗口位置改变信号
      * @since 2.3
      * @param wid
      */
     void geometryChanged(const WindowId& wid);

     /**
      * @brief 屏幕信息改变信号
      * @since 2.5
      */
     void outputInfoChanged(OutputInfo*);

private:
     WindowManager(QObject *parent = nullptr);
};
}


#endif // WINDOWMANAGER_H
