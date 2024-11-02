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
 * Authors: Xiangwei Liu <liuxiangwei@kylinos.cn>
 *
 */

#ifndef KPUSHBUTTON_H
#define KPUSHBUTTON_H

#include "gui_g.h"
#include <QWidget>
#include <QPushButton>

namespace kdk {

/**
 * @defgroup PushbuttonModule
 */
class KPushButtonPrivate;

/**
 * @brief 1.提供半透明效果
 *        2.可以设置按钮圆角、背景色、图标是否跟随系统高亮
 *        3.可以设置按钮类型，分为正常类型和圆形（需要设置大小实现正圆），其余同QPushButton
 * @since 1.2
 */

class GUI_EXPORT KPushButton:public QPushButton
{
    Q_OBJECT
public:

    /**
     * @brief 支持两种样式的按钮
     */
    enum ButtonType
    {
        NormalType,
        CircleType
    };

    KPushButton(QWidget* parent = nullptr);


    ~KPushButton();

    /**
     * @brief 设置按钮圆角
     * 通过圆角半径设置按钮圆角（每个圆角相同）
     * @param radius
     */
    void setBorderRadius(int radius);

    /**
     * @brief 设置按钮圆角
     * 通过四个点来设置圆角
     * @param bottomLeft（左下）
     * @param topLeft（左上）
     * @param topRight（右上）
     * @param bottomRight（右下）
     */
    void setBorderRadius(int bottomLeft,int topLeft,int topRight,int bottomRight);

    /**
     * @brief 获取按钮圆角
     * @return radius
     */
    int borderRadius();

    /**
     * @brief 设置按钮背景色
     * @param color
     */
    void setBackgroundColor(QColor color);

    /**
     * @brief 获取按钮背景色
     * @return color
     */
    QColor backgroundColor();

    /**
     * @brief 设置KPushButton的类型
     * @param type
     */
    void setButtonType(ButtonType type);

    /**
     * @brief 获取KPushButton的类型
     * @return
     */
    ButtonType buttonType();
    
    /**
     * @brief 设置KPushButton是否为半透明
     * @param flag
     */
    void setTranslucent(bool flag);

    /**
     * @brief 判断KPushButton是否为半透明
     * @return
     */
    bool isTranslucent();

    /**
     * @brief 设置图标是否跟随系统高亮色，默认不跟随
     * @param flag
     */
    void setIconHighlight(bool flag);

    /**
     * @brief 判断图标是否跟随系统高亮色
     * @return
     */
    bool isIconHighlight();

    /**
     * @brief 设置按钮添加图标的颜色
     * @param color
     */
    void setIconColor(QColor color);

    /**
     * @brief 获取按钮添加图标的颜色
     * @return
     */
    QColor IconColor();

    /**
     * @brief 设置按钮背景色是否跟随系统高亮色，默认不跟随
     * @param flag
     */
    void setBackgroundColorHighlight(bool flag);

    /**
     * @brief 判断按钮背景色是否跟随系统高亮色
     * @return
     */
    bool isBackgroundColorHighlight();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    Q_DECLARE_PRIVATE(KPushButton)
    KPushButtonPrivate * const d_ptr;
};

}

#endif // KPUSHBUTTON_H
