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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#ifndef KTRANSLUCENTFLOOR_H
#define KTRANSLUCENTFLOOR_H

#include "gui_g.h"
#include <QFrame>

namespace kdk {

/**
 *  @defgroup FormModule
 */
class KTranslucentFloorPrivate;

/**
 * @brief 提供一个半透明底板，可作为基础窗体进行布局
 * @since 1.2
 */
class GUI_EXPORT KTranslucentFloor : public QFrame
{
    Q_OBJECT
public:
    KTranslucentFloor(QWidget* parent =nullptr);

    /**
     * @brief 设置圆角半径
     * @param radius
     */
    void setBorderRadius(int radius);

    /**
     * @brief 返回圆角半径
     * @return
     */
    int borderRadius();

    /**
     * @brief 设置是否显示阴影
     * @param flag
     */
    void setShadow(bool flag);

    /**
     * @brief 返回是否显示阴影
     * @return
     */
    bool shadow();

    /**
     * @brief 设置是否启用毛玻璃效果
     * @param flag
     */
    void setEnableBlur(bool flag);

    /**
     * @brief 获取是否已启用毛玻璃效果
     * @return
     */
    bool enableBlur();

    /**
     * @brief 设置透明度
     * @param opacity
     */
    void setOpacity(qreal opacity);

    /**
     * @brief 获取透明度
     * @return
     */
    qreal opacity();

protected:
    void paintEvent(QPaintEvent *);

private:
    KTranslucentFloorPrivate*const d_ptr;
    Q_DECLARE_PRIVATE(KTranslucentFloor)
};
}

#endif // KTRANSLUCENTFLOOR_H
