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

#ifndef KCOLORBUTTON_H
#define KCOLORBUTTON_H

#include <QPushButton>
#include "gui_g.h"

namespace kdk {
/**
 *  @defgroup PushbuttonModule
 */
class KColorButtonPrivate;


/**
 * @brief 提供用于设置颜色和样式的按钮，以适用于不同的场景。
 * @since 2.0
 */
class GUI_EXPORT KColorButton :public QPushButton
{
    Q_OBJECT
public:

    /**
     * @brief KColorButton提供三种不同的样式
     */
    enum ButtonType{
        Circle,
        RoundedRect,
        CheckedRect
    };

    KColorButton(QWidget* parent = nullptr);

    /**
     * @brief 设置colorButton背景色
     * @param color
     */
    void setBackgroundColor(QColor color);

    /**
     * @brief 返回colorButton背景色
     * @return
     */
    QColor backgroundColor();

    /**
     * @brief 设置圆角(RoundedRect，CheckedRect类型生效)
     * @param radious
     */
    void setBorderRadius(int radious);

    /**
     * @brief 返回圆角
     * @return
     */
    int borderRadius();

    /**
     * @brief 设置colorButton类型
     * @param type
     */
    void setButtonType(KColorButton::ButtonType type);

    /**
     * @brief 返回colorButton类型
     * @return
     */
    KColorButton::ButtonType buttonType();

protected:
    void paintEvent(QPaintEvent *) override;
    QSize sizeHint() const override;

private:
    Q_DECLARE_PRIVATE(KColorButton)
    KColorButtonPrivate* const d_ptr;
};

}

#endif // KCOLORBUTTON_H
