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

#ifndef KPIXMAPCONTAINER_H
#define KPIXMAPCONTAINER_H

#include "gui_g.h"
#include <QLabel>
#include <QWidget>

namespace kdk
{

/** @defgroup BarModule
  * @{
  */

class KPixmapContainerPrivate;

/**
 * @brief 为指定的pixmap添加右上角消息提示气泡，样式类似微信头像的消息提示
 */
class  GUI_EXPORT KPixmapContainer:public QWidget
{
    Q_OBJECT

public:
    KPixmapContainer(QWidget *parent = nullptr);

    /**
     * @brief 获取值
     */
    int value() const;

    /**
     * @brief 设置值
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
     * @brief 设置pixmap
     * @param pixmap
     */
    void setPixmap(const QPixmap& pixmap);

    /**
     * @brief 获取pixmap
     */
    QPixmap pixmap()const;

    /**
     * @brief 清除值
     */
    void clearValue();

    /**
     * @brief 返回背景色
     * @return
     */
    QColor color();

    /**
     * @brief 设置背景色
     * @param color
     */
    void setColor(const QColor& color);

    /**
     * @brief 返回字体大小
     * @return
     */
    int fontSize();

    /**
     * @brief 设置字体大小
     * @param size
     */
    void setFontSize(int size);

protected:
    void paintEvent(QPaintEvent*event);

private:
    Q_DECLARE_PRIVATE(KPixmapContainer)
    KPixmapContainerPrivate*const d_ptr;

};
}
/**
  * @example testbadge/widget.h
  * @example testbadge/widget.cpp
  * @}
  */

#endif // KPIXMAPCONTAINER_H
