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

#ifndef KLISTWIDGET_H
#define KLISTWIDGET_H

#include <QWidget>
#include "kitemwidget.h"
#include "themeController.h"
#include "gui_g.h"
namespace  kdk 
{


class KListWidgetPrivate;
/*
 *  一个可以显示两行文本的listwidget
 */
class GUI_EXPORT KListWidget : public QListWidget
{
     Q_OBJECT
public:
    KListWidget(QWidget* parent);
    /*
     * 将Itemwidget插入到listwidget
     * m_itemwidget需要插入的Itemwidget
    */
    void AddItemWidget(KItemWidget* m_itemwidget);

    //void mousePressEvent(QMouseEvent *event);

    //void itemClicked(QListWidgetItem * item);


private:
    Q_DECLARE_PRIVATE(KListWidget);
    KListWidgetPrivate* const d_ptr;
};
}
#endif // KLISTWIDGET_H
