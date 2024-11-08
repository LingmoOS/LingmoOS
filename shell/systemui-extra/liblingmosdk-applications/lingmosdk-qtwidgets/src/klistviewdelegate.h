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

#ifndef KLISTVIEWDELEGATE_H
#define KLISTVIEWDELEGATE_H

#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPixmap>
#include "themeController.h"
#include <QAbstractItemView>

namespace kdk {

/**
 * @defgroup ListViewModule
 */

class KListViewDelegatePrivate;

/**
 * @brief 配合KListview可以提供一个图片两行文字的显示效果，若只有一行文字，则对于 icon 居中显示 。
 */
class KListViewDelegate :public QStyledItemDelegate,public ThemeController
{
    Q_OBJECT
public:
    KListViewDelegate(QAbstractItemView*parent);
    ~KListViewDelegate();

protected:
     void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;//重写绘制事件
     virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
     Q_DECLARE_PRIVATE(KListViewDelegate);
     KListViewDelegatePrivate* const d_ptr;
};
}
#endif // KLISTVIEWDELEGATE_H
