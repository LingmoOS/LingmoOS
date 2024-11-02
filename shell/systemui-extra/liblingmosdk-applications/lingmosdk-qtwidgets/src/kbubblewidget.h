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

#ifndef KBubbleWidget_H
#define KBubbleWidget_H


#include <QWidget>
#include <QBoxLayout>

namespace kdk {
/**
 * @brief 指定气泡尾部显示的方向
 */
enum TailDirection
    {
        TopDirection,
        LeftDirection,
        BottomDirection,
        RightDirection,
        None
    };

/**
 * @brief 指定气泡尾部的显示位置
 */
enum TailLocation
    {
        LeftLocation,
        MiddleLocation,
        RightLocation
    };

/**
 * @defgroup FormModule
 */

class KBubbleWidgetPrivate;


/**
 * @brief 提供了一个带三角形窗体。
 * @since 2.0
 */
class KBubbleWidget  : public QWidget
{
    Q_OBJECT

public:
    KBubbleWidget (QWidget *parent = nullptr);

    /**
     * @brief 设置气泡尾部尺寸
     * @param size
     */
    void setTailSize(const QSize& size);

    /**
     * @brief 获取气泡尾部尺寸
     * @return
     */
    QSize tailSize();

    /**
     * @brief 设置气泡尾部显示位置
     * @param dirType
     * @param locType
     */
    void setTailPosition(TailDirection dirType, TailLocation locType=MiddleLocation);

    /**
     * @brief 获取气泡尾部显示方向
     * @return 左、上、右、下
     */
    TailDirection tailDirection();

    /**
     * @brief 获取气泡尾部显示位置
     * @return 居左、居中、居右
     */
    TailLocation tailLocation();

    /**
     * @brief 设置窗体圆角半径
     * @param bottomLeft
     * @param topLeft
     * @param topRight
     * @param bottomRight
     */
    void setBorderRadius(int bottomLeft,int topLeft,int topRight,int bottomRight);

    /**
     * @brief 设置窗体圆角半径
     * @param radius
     */
    void setBorderRadius(int radius);

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
    Q_DECLARE_PRIVATE(KBubbleWidget)
    KBubbleWidgetPrivate* const d_ptr;
};

}

#endif // KBubbleWidget_H
