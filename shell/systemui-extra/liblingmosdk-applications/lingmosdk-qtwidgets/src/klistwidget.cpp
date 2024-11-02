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

#include "klistwidget.h"
#include <QDebug>
namespace kdk
{

class KListWidgetPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KListWidget)
public:
    KListWidgetPrivate(KListWidget* parent);
private:
    KListWidget* q_ptr;
    QListWidgetItem* item;
    QListWidgetItem * m_item;

};

KListWidgetPrivate::KListWidgetPrivate(KListWidget* parent):q_ptr(parent)
{
    Q_Q(KListWidget);
    setParent(parent);
}
KListWidget::KListWidget(QWidget* parent):d_ptr(new KListWidgetPrivate(this))
{
    Q_D(KListWidget);
}
void KListWidget::AddItemWidget(KItemWidget *m_itemwidget)
{
    Q_D(KListWidget);
    if(!m_itemwidget)
        return ;
    d->item = new QListWidgetItem(this);
    d->item->setSizeHint(QSize(this->width(),54));


    this->setItemWidget(d->item,m_itemwidget);

    //建立连接
    connect(this,&KListWidget::itemClicked,this,[=](QListWidgetItem *current){
        KItemWidget* curWidget = dynamic_cast<KItemWidget*>(this->itemWidget(current));
        if(curWidget)
            curWidget->SetInverse();
    });
    connect(this,&KListWidget::currentItemChanged,this,[=](QListWidgetItem *current, QListWidgetItem *previous){
        KItemWidget* curWidget = dynamic_cast<KItemWidget*>(this->itemWidget(current));
        KItemWidget* preWidget = dynamic_cast<KItemWidget*>(this->itemWidget(previous));
        if(curWidget&&!preWidget)
        {           
            curWidget->CancelInverse();
        }
        else if(curWidget)
        {
             curWidget->SetInverse();
        }
        if(preWidget)
        {
            preWidget->CancelInverse();
        }
    });
}








}
#include "klistwidget.moc"
#include "moc_klistwidget.cpp"
