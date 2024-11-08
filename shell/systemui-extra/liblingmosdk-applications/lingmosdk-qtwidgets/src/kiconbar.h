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

#ifndef KICONBAR_H
#define KICONBAR_H

#include "gui_g.h"
#include <QFrame>
#include <QIcon>
#include <QLabel>
#include <QString>

namespace kdk
{

/** @defgroup BarModule
  * @{
  */

class KIconBarPrivate;

/**
 * @brief 图标和标题组合控件
 */
class GUI_EXPORT KIconBar:public QFrame
{
    Q_OBJECT
public:
    KIconBar(QWidget* parent = nullptr);
    KIconBar(const QString& iconName,const QString& text,QWidget* parent = nullptr);
    ~KIconBar();

    /**
     * @brief 设置图标
     * @param iconName
     */
    void setIcon(const QString& iconName);

    /**
     * @brief 设置图标
     * @param icon
     */
    void setIcon(const QIcon& icon);

    /**
     * @brief 设置标题
     * @param widgetName
     */
    void setWidgetName(const QString& widgetName);

    /**
     * @brief 获取标题label
     * @return
     */
    QLabel* nameLabel();

    /**
     * @brief 获取图标label
     * @return
     */
    QLabel* iconLabel();

Q_SIGNALS:
    /**
     * @brief 双击会发出双击信号，父widget可以绑定相应槽函数
     */
    void doubleClick();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    QSize sizeHint() const override;

private:
    Q_DECLARE_PRIVATE(KIconBar)
    KIconBarPrivate*const d_ptr;

};
}
/**
  * @example testWidget/testwidget.h
  * @example testWidget/testwidget.cpp
  * @}
  */
#endif // KICONBAR_H
