/*
 * liblingmosdk-qtwidgets's Library
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

#ifndef KTITLEBAR_H
#define KTITLEBAR_H

#include "gui_g.h"
#include "kmenubutton.h"
#include <QFrame>
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>

namespace kdk
{

/** @defgroup BarModule
  * @{
  */

/**
 * @brief  最大化按钮提供两种状态响应
 */
enum MaximumButtonState
{
    Maximum,
    Restore
};
class KWindowButtonBarPrivate;

/**
 * @brief 窗口三联按钮和菜单按钮的集合
 */
class GUI_EXPORT KWindowButtonBar:public QFrame
{
    Q_OBJECT

public:
    KWindowButtonBar(QWidget*parent=nullptr);
    ~KWindowButtonBar();

    /**
     * @brief 获取最小化按钮
     * @return
     */
    QPushButton* minimumButton();

    /**
     * @brief 获取最大化按钮
     * @return
     */
    QPushButton* maximumButton();

    /**
     * @brief 获取关闭按钮
     * @return
     */
    QPushButton* closeButton();

    /**
     * @brief 获取菜单按钮
     * @return
     */
    KMenuButton* menuButton();

    /**
     * @brief 获取最大化按钮的状态(最大化/恢复)
     * @return
     */
    MaximumButtonState maximumButtonState();

    /**
     * @brief 设置最大化按钮图标状态（最大化/恢复）
     * @param state
     */
    void setMaximumButtonState(MaximumButtonState state);

    /**
    * @brief 设置是否遵循模式
    * @since 2.0
    * @param flag
    */
    void setFollowMode(bool flag);

    /**
    * @brief 返回是否遵循模式
    * @since 2.0
    * @return
    */
    bool followMode();


Q_SIGNALS:
    /**
     * @brief 双击会发出双击信号，父widget可以绑定相应槽函数
     */
    void doubleClick();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    QSize sizeHint() const override;

private:
    Q_DECLARE_PRIVATE(KWindowButtonBar)
    KWindowButtonBarPrivate *d_ptr;
};
}
/**
  * @example testWidget/testwidget.h
  * @example testWidget/testwidget.cpp
  * @}
  */

#endif //KTITLEBAR_H
