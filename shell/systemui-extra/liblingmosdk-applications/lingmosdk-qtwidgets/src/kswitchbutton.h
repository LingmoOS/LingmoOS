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

#ifndef KSWITCHBUTTON_H
#define KSWITCHBUTTON_H

#include "gui_g.h"
#include <QPushButton>
#include <QTimer>
#include <QPainter>
#include <QGSettings/QGSettings>

namespace kdk
{

/** @defgroup PushbuttonModule
  * @{
  */

class KSwitchButtonPrivate;

/**
 * @brief 指示打开/关闭两种状态
 */
class GUI_EXPORT KSwitchButton:public QPushButton
{
    Q_OBJECT

public:
    KSwitchButton(QWidget* parent = 0);
    ~KSwitchButton();

    /**
     * @brief 设置是否可选中
     */
    void setCheckable(bool);

    /**
     * @brief 返回是否可选中
     * @return
     */
    bool isCheckable() const;

    /**
     * @brief 返回是否选中
     * @return
     */
    bool isChecked() const;

    /**
     * @brief 设置是否选中
     * @return
     */
    void setChecked(bool);

    /**
     * @brief 设置是否启用半透明效果
     * @since 1.2
     * @param flag
     */
    void setTranslucent(bool flag);

    /**
     * @brief 获取是否启用半透明效果
     * @since 1.2
     * @return flag
     */
    bool isTranslucent();

Q_SIGNALS:
    /**
     * @brief 当选中状态发生变化时，发出此信号
     */
    void stateChanged(bool);

protected:
   void paintEvent(QPaintEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   QSize sizeHint() const;
    
private:
    Q_DECLARE_PRIVATE(KSwitchButton)
    KSwitchButtonPrivate* const d_ptr;
};

}
/**
  * @example testSwitchbutton/widget.h
  * @example testSwitchbutton/widget.cpp
  * @}
  */
#endif //KSWITCHBUTTON_H
