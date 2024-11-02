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

#ifndef KBADGE_H
#define KBADGE_H

#include "gui_g.h"
#include <QWidget>
#include <QPixmap>
#include <QColor>

namespace kdk
{
class KBadgePrivate;
/**
 * @defgroup MessageAlertModule
 * @{
 */

/**
 * @brief 消息提醒气泡
 */
class GUI_EXPORT KBadge:public QWidget
{
    Q_OBJECT

public:
    KBadge(QWidget*parent);

    /**
     * @brief 返回值
     * @return
     */
    int value();

    /**
     * @brief 设置值，最大显示数值为999，大于999显示"..."
     * @param value
     */
    void setValue(int value);

    /**
     * @brief 设置值是否可见
     * @param flag
     */
    void setValueVisiable(bool flag);

    /**
     * @brief 获取值是否可见
     */
    bool isValueVisiable() const;

    /**
     * @brief 获取背景色
     * @return
     */
    QColor color();

    /**
     * @brief 设置背景色
     * @param color
     */
    void setColor(const QColor& color);

    /**
     * @brief 获取字体大小
     * @return
     */
    int fontSize();

    /**
     * @brief 设置字体大小
     * @param size
     */
    void setFontSize(int size);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    Q_DECLARE_PRIVATE(KBadge)
    KBadgePrivate*const d_ptr;
};
}
/**
  * @example testbadge/widget.h
  * @example testbadge/widget.cpp
  * @}
  */

#endif // KBADGE_H
