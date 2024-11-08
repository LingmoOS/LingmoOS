/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#ifndef LINGMO_QUICK_WM_INTERFACE_H
#define LINGMO_QUICK_WM_INTERFACE_H

#include <QObject>
#include <QWindow>
#include <QPoint>
#include <QScreen>

#include "window-type.h"

namespace LingmoUIQuick {

/**
 * @class WMInterface
 *
 * 抽出x和wayland环境或窗口管理器的常见接口
 *
 */
class WMInterface : public QObject
{
    Q_OBJECT
public:
    static bool isWayland();

    explicit WMInterface(QWindow *window);
    ~WMInterface() override;

    QWindow *window() const;

    // protocols
    /**
     * 设置窗口位置
     */
    virtual QPoint position() const;
    virtual void setPosition(const QPoint &pos);

    /**
     * 设置窗口类型
     * 此处支持常用的几个窗口类型
     * 每个类型在窗口管理器中的行为都不一样,使用前请先了解相关文档
     *
     * @see WindowType::Type
     */
    virtual void setWindowType(WindowType::Type type) = 0;

    /**
     * 不在taskBar(任务栏)上显示
     */
    virtual void setSkipTaskBar(bool skip = true) = 0;

    /**
     * 不在多任务选择器中显示
     */
    virtual void setSkipSwitcher(bool skip = true) = 0;

    /**
     * 移除窗口管理器添加的标题栏
     */
    virtual void setRemoveTitleBar(bool remove = true) = 0;

    /**
     * 设置窗口是否获取焦点
     * @param takesFocus
     */
    virtual void setPanelTakesFocus(bool takesFocus = true) = 0;

    /**
     * 设置窗口为'自动隐藏'状态
     * 设置为true时, 窗口管理器会将窗口所在的位置归还给可用区域,但是目前不会将该窗口隐藏,需要窗口自己实现
     * 设置为false时,窗口管理器会将窗口所在的区域从可用区域中移除,普通应用将无法覆盖该区域.
     *
     * @warning 该接口仅对Panel和Dock层级有效(wayland)
     */
    virtual void setPanelAutoHide(bool autoHide = false) = 0;

    /**
     * 获取当前鼠标所在屏幕
     * @return
     */
    virtual QScreen* currentScreen() = 0;

    // TODO: add wm interfaces

private:
    class Private;
    Private *d {nullptr};
};

/**
 * @class WManager
 *
 * WMInterface的工厂类，通过getWM方法获取WMInterface实例
 *
 */
class WManager
{
public:
    static WMInterface *getWM(QWindow *window);
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_WM_INTERFACE_H
