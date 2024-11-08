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

#ifndef KBORDERBUTTON_H
#define KBORDERBUTTON_H

#include "gui_g.h"
#include <QWidget>
#include <QPushButton>
#include <QIcon>
#include <QEvent>

namespace kdk
{

/** @defgroup PushbuttonModule
  * @{
  */

class KBorderButtonPrivate;

/**
 * @brief 带边框的按钮
 */
class GUI_EXPORT KBorderButton:public QPushButton
{
    Q_OBJECT
public:
    KBorderButton(QWidget* parent = nullptr);
    KBorderButton(const QString &text, QWidget *parent = nullptr);
    KBorderButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);
    KBorderButton(const QIcon &icon, QWidget *parent = nullptr);

    /**
     * @brief 设置按钮图标
     * @param icon
     */
    void setIcon(const QIcon &icon);
    ~KBorderButton();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const override;

private:
    Q_DECLARE_PRIVATE(KBorderButton)
    KBorderButtonPrivate * const d_ptr;
};
}
/**
  * @example testPushbutton/widget.h
  * @example testPushbutton/widget.cpp
  * @}
  */
#endif

