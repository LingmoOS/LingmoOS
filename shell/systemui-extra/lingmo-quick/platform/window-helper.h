/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_ITEMS_WINDOW_HELPER_H
#define LINGMO_QUICK_ITEMS_WINDOW_HELPER_H

#include <QObject>
#include <QRect>
#include <QWindow>
#include <QScreen>
#include "window-type.h"
#include "wm-interface.h"

namespace LingmoUIQuick {

class WindowProxyPrivate;

/**
 * @class WindowProxy
 *
 * usage:
 * new WindowProxy(this, WindowProxy::RemoveTitleBar, this);
 *
 * WindowProxy是一个事件过滤器,用于为窗口(Window)调用窗口管理器或wayland协议的相关接口,实现某些操作
 * 其中的操作可分为:
 * 1.去除标题栏
 * 2.跳过任务选择器和任务栏
 * 3.设置窗口Geometry(大小和位置)
 * 4.添加边缘划出特效
 * 5.添加自动从屏幕边缘隐藏窗口特效(一般情况下仅panel需要)
 * 6.为Window指定窗口类型(类型由窗口管理器或显示协议定义)
 *
 * 此外还提供了一些便利接口:
 * 1.判断是否wayland环境
 *
 */
class WindowProxy : public QObject
{
    Q_OBJECT
public:
    enum Operation {
        SkipTaskBar = 0x0001,
        SkipSwitcher = 0x0002,
        RemoveTitleBar = 0x0004
    };
    Q_DECLARE_FLAGS(Operations, Operation)

    /**
     * 窗口在哪个屏幕边缘滑出
     */
    enum SlideFromEdge {
        NoEdge = 0,
        TopEdge,
        RightEdge,
        BottomEdge,
        LeftEdge,
    };
    Q_ENUM(SlideFromEdge)

    explicit WindowProxy(QWindow *window, Operations operations = {SkipTaskBar | SkipSwitcher | RemoveTitleBar});
    ~WindowProxy() override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * 是否wayland环境
     */
    static bool isWayland();

    /**
     * 移除窗口管理器添加的标题栏
     */
    void setRemoveTitleBar(bool remove = true);

    /**
     * 不在taskBar(任务栏)上显示
     */
    void setSkipTaskBar(bool skip = true);

    /**
     * 不在多任务选择器中显示
     */
    void setSkipSwitcher(bool skip = true);

    /**
     * 设置窗口位置
     */
    void setPosition(const QPoint &point);
    void setGeometry(const QRect &rect);

    /**
     * 设置窗口类型
     * 此处支持常用的几个窗口类型
     * 每个类型在窗口管理器中的行为都不一样,使用前请先了解相关文档
     *
     * @see WindowType::Type
     */
    void setWindowType(WindowType::Type type);

    /**
     * 设置窗口为'自动隐藏'状态
     * 设置为true时, 窗口管理器会将窗口所在的位置归还给可用区域,但是目前不会将该窗口隐藏,需要窗口自己实现
     * 设置为false时,窗口管理器会将窗口所在的区域从可用区域中移除,普通应用将无法覆盖该区域.
     *
     * @warning 该接口仅对Panel和Dock层级有效(wayland)
     */
    void setPanelAutoHide(bool autoHide);

     /**
      * 为窗口添加滑出特效
      * @param location 窗口从哪个边缘滑出
      * @param offset 窗口滑出位置与屏幕边缘的间距
      */
    static void slideWindow(QWindow *window, SlideFromEdge fromEdge, int offset = -1);
    void slideWindow(SlideFromEdge fromEdge, int offset = -1);

    /**
     * 为窗口添加毛玻璃特效
     * @param window 需要添加毛玻璃特效的窗口
     * @param enable true: 启用, false: 禁用
     * @param region 窗口添加毛玻璃的区域, 空区域表示窗口整体添加毛玻璃
     */
    static void setBlurRegion(QWindow *window, bool enable = true, const QRegion &region = QRegion());
    void setBlurRegion(bool enable = true, const QRegion &region = QRegion());

    static QScreen* currentScreen();

private:
    WindowProxyPrivate *d = nullptr;
};

/**
 * @class WindowProxy2
 *
 * WindowProxy2 是一个事件过滤器,只负责在合适的时机调用后端的接口，为窗口设置窗口位置，类型和各种属性。
 * 具体的后端接口如何定义，由WMImplWayland和WMImplX11实现。
 *
 * @warning 警告：必须注意的一点，如果使用WindowProxy2，任何WindowProxy2提供了的功能接口都必须通过WindowProxy2进行设置，否则属性将被覆盖为默认值
 *
 */
class WindowProxy2 : public WMInterface
{
    Q_OBJECT
public:
    explicit WindowProxy2(QWindow *window, WindowProxy::Operations operations = {WindowProxy::SkipTaskBar | WindowProxy::SkipSwitcher | WindowProxy::RemoveTitleBar});
    ~WindowProxy2() override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    QPoint position() const override;
    void setPosition(const QPoint &point) override;
    void setWindowType(WindowType::Type type) override;
    void setSkipTaskBar(bool skip) override;
    void setSkipSwitcher(bool skip) override;
    void setRemoveTitleBar(bool remove) override;
    void setPanelTakesFocus(bool takesFocus) override;
    void setPanelAutoHide(bool autoHide) override;

    void setSlideWindowArgs(WindowProxy::SlideFromEdge fromEdge, int offset);

    QScreen* currentScreen();

private:
    class Private;
    WindowProxy2::Private *d = nullptr;
};

} // LingmoUIQuick

// 定义flag的操作符，eg: |, &
Q_DECLARE_OPERATORS_FOR_FLAGS(LingmoUIQuick::WindowProxy::Operations)
Q_DECLARE_METATYPE(LingmoUIQuick::WindowType::Type)
Q_DECLARE_METATYPE(LingmoUIQuick::WindowProxy::SlideFromEdge)

#endif //LINGMO_QUICK_ITEMS_WINDOW_HELPER_H
