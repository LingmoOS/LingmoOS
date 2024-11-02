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

#ifndef KTABBAR_H
#define KTABBAR_H

#include "gui_g.h"
#include <QTabBar>
#include <QIcon>

namespace kdk
{

/** @defgroup BarModule
  * @{
  */

/**
 * @brief 支持三种样式
 */
enum KTabBarStyle
{
    SegmentDark,
    SegmentLight,
    Sliding
};

class KTabBarPrivate;

/**
 * @brief 支持三种样式
 */
class GUI_EXPORT KTabBar: public QTabBar
{
    Q_OBJECT

public:
    KTabBar(KTabBarStyle barStyle = SegmentLight,QWidget* parent = nullptr);
    ~KTabBar();

    /**
     * @brief 设置TabBar样式
     * @param barStyle
     */
    void setTabBarStyle(KTabBarStyle barStyle);

    /**
     * @brief 返回TabBar样式
     * @return
     */
    KTabBarStyle barStyle();

    /**
     * @brief 设置圆角半径，只对SegmentDark，SegmentLight样式生效
     * @param radius
     */
    void setBorderRadius(int radius);

    /**
     * @brief 获取圆角半径
     * @return
     */
    int borderRadius();

    /**
     * @brief 设置背景色
     * @param color
     * @since 1.2
     */
    void setBackgroundColor(const QColor& color);

protected:
    QSize sizeHint() const;
    QSize minimumTabSizeHint(int index) const;
     QSize tabSizeHint(int index) const;
    void paintEvent(QPaintEvent *event);

private:
    Q_DECLARE_PRIVATE(KTabBar)
    KTabBarPrivate*const d_ptr;
};
}
/**
  * @example testtabbar/widget.h
  * @example testtabbar/widget.cpp
  * @}
  */
#endif //KTABBAR_H
