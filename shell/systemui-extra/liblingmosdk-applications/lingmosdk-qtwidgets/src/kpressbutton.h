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

#ifndef KPRESSBUTTON_H
#define KPRESSBUTTON_H
#include "gui_g.h"
#include <QPushButton>
#include <QWidget>
namespace kdk {

/**
 * @defgroup PushbuttonModule
 */
class KPressButtonPrivate;

/**
 * @brief 按压按钮，可根据情况设置loading状态
 * @since 1.2
 */
class GUI_EXPORT KPressButton:public QPushButton
{
    Q_OBJECT
public:
    enum ButtonType{
        NormalType,
        CircleType
    };

    KPressButton(QWidget* parent = 0);

    /**
     * @brief 设置圆角
     * @param radius
     */
    void setBorderRadius(int radius);

    /**
     * @brief 设置圆角
     * @param buttomLeft
     * @param topLeft
     * @param topRight
     * @param bottomRight
     */
    void setBorderRadius(int bottomLeft,int topLeft,int topRight,int bottomRight);

    /**
     * @brief 设置是否可选中
     * @param flag
     */
    void setCheckable(bool);

    /**
     * @brief 返回是否可选中
     * @return
     */
    bool isCheckable() const;

    /**
     * @brief 设置是否选中
     * @return
     */
    void setChecked(bool);

    /**
     * @brief 返回是否选中
     * @return
     */
    bool isChecked() const;

    /**
     * @brief 设置button类型
     * @param type
     */
    void setButtonType(ButtonType type);

    /**
     * @brief 返回button类型
     * @return
     */
    KPressButton::ButtonType buttonType();

    /**
     * @brief 设置是否启用loading状态
     * @param flag
     */
    void setLoaingStatus(bool flag);

    /**
     * @brief 返回是否启用loading状态
     * @return
     */
    bool isLoading();

    /**
     * @brief 设置是否启用透明度
     * @param flag
     */
    void setTranslucent(bool flag);

    /**
     * @brief 返回是否启用透明度
     * @return
     */
    bool isTranslucent();
protected:
   void paintEvent(QPaintEvent *event) override;

private:
    Q_DECLARE_PRIVATE(KPressButton)
    KPressButtonPrivate* const d_ptr;
};

}
#endif // KPRESSBUTTON_H
