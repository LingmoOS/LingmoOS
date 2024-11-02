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

#ifndef KITEMWIDGET_H
#define KITEMWIDGET_H

#include <QWidget>
#include <QIcon>
#include <QPainter>
#include <QString>
#include <QSize>
#include <QPixmap>
#include <QFont>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include "gui_g.h"
#include "themeController.h"

namespace kdk
 {


class KItemWidgetPrivate;

class GUI_EXPORT KItemWidget :public QWidget
{
    Q_OBJECT
public:

    /*
     * Myicon 需要显示的图片
     * MmainText 需要写入的miantext
     * MsecText  需要写入的sectext
    */
    KItemWidget(const QIcon &Myicon,QString MmainText,QString MsecText,QWidget *parent);
    /*
     * 设置反白效果
    */
    void SetInverse();

    /*
     * 取消反白效果
    */
    void CancelInverse();
    /*
     * 设置图片大小
    */
    void SetIconSize(QSize size);


protected:
    void paintEvent(QPaintEvent *event);
    /*void mousePressEvent(QMouseEvent *event);*/

private:
    Q_DECLARE_PRIVATE(KItemWidget);
    KItemWidgetPrivate* const d_ptr;
};

}
#endif // KITEMWIDGET_H
