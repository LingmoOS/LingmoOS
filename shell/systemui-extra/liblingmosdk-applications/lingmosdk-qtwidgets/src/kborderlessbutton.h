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

#ifndef KBORDERLESSBUTTON_H
#define KBORDERLESSBUTTON_H

#include "gui_g.h"
#include <QIcon>
#include <QPushButton>
#include <QWidget>
#include <QEvent>

namespace kdk
{

/** @defgroup PushbuttonModule
  * @{
  */

class KBorderlessButtonPrivate;

/**
 * @brief 无边框按钮
 */
class GUI_EXPORT KBorderlessButton:public QPushButton
{
    Q_OBJECT

public:
    KBorderlessButton(QWidget* parent = nullptr);
    KBorderlessButton(const QString &text, QWidget *parent = nullptr);
    KBorderlessButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);
    KBorderlessButton(const QIcon &icon, QWidget *parent = nullptr);
    ~KBorderlessButton();

    /**
     * @brief 设置按钮图标
     * @param icon
     */
    void setIcon(const QIcon &icon);

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const;

private:
    KBorderlessButtonPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(KBorderlessButton)
    Q_PRIVATE_SLOT(d_ptr, void changeTheme())
};
}
/**
  * @example testPushbutton/widget.h
  * @example testPushbutton/widget.cpp
  * @}
  */
#endif

