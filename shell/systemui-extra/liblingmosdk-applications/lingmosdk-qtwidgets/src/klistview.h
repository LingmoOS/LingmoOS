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

#ifndef KLISTVIEW_H
#define KLISTVIEW_H

#include <QListView>
#include <QMouseEvent>

namespace kdk {

/**
 * @defgroup ListViewModule
 */
class KListViewPrivate;

/**
 * @brief 配合KListviewdelegate可以提供一个图片两行文字的显示效果，若只有一行文字，则对于 icon 居中显示 。
 */
class KListView :public QListView
{
    Q_OBJECT
public:
    KListView(QWidget*parent);
protected:
    void mousePressEvent ( QMouseEvent * event ) ;
private:
    Q_DECLARE_PRIVATE(KListView);
    KListViewPrivate* const d_ptr;

};
}
#endif // KLISTVIEW_H
