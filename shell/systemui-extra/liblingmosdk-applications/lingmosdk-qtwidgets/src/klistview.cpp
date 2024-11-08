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

#include "klistview.h"
#include <QDebug>
namespace kdk {

class KListViewPrivate :public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KListView)
public:
    KListViewPrivate(KListView* parent);

private:
    KListView* q_ptr;
};


KListViewPrivate::KListViewPrivate(KListView *parent)
    :q_ptr(parent)
{
    Q_Q(KListView);
    setParent(parent);
}


KListView::KListView(QWidget *parent)
    :QListView(parent)
    ,d_ptr(new KListViewPrivate(this))
{
    Q_D(KListView);
}

void KListView::mousePressEvent(QMouseEvent *event)
{
    Q_D(KListView);
    QPoint p = event->pos(); //获取点击坐标
    QModelIndex index = indexAt(p); //点击位置是否为空
    if(!index.isValid())
    {
        setCurrentIndex(QModelIndex());     //实现点空白处取消选择
    }
    QListView::mousePressEvent(event);
}

}

#include "klistview.moc"
#include "moc_klistview.cpp"

