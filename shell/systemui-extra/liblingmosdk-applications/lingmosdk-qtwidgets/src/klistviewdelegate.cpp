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

#include "klistviewdelegate.h"
#include <QString>
#include <QPainter>
#include <QFont>
#include <QApplication>
#include <QFontMetrics>
#include <QLabel>
#include <QDebug>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QStandardItemModel>
#include "themeController.h"
#include <QCoreApplication>
#include <QListView>
#include <QHBoxLayout>
#include <QPainterPath>

namespace kdk
{

class KListViewDelegatePrivate : public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KListViewDelegate)
public:
    KListViewDelegatePrivate(KListViewDelegate* parent);
    ~KListViewDelegatePrivate();

private:
    KListViewDelegate* q_ptr;
    QAbstractItemView* m_listView;

};

KListViewDelegatePrivate::KListViewDelegatePrivate(KListViewDelegate *parent)
    :q_ptr(parent)
{
    Q_Q(KListViewDelegate);
    initThemeStyle();
    connect(m_gsetting,&QGSettings::changed,this,[=](){initThemeStyle();});
}

KListViewDelegatePrivate::~KListViewDelegatePrivate()
{
}

KListViewDelegate::KListViewDelegate(QAbstractItemView*parent)
    :QStyledItemDelegate(parent)
    ,d_ptr(new KListViewDelegatePrivate(this))
{
    Q_D(KListViewDelegate);
    d->m_listView = parent;
}

KListViewDelegate::~KListViewDelegate()
{

}

void KListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_D(const KListViewDelegate);

    QFont font;
    font = QApplication::font();
    QFontMetrics fm(font);
    int height = fm.height();

    if (index.isValid())
    {
        QListView* view = dynamic_cast<QListView*>(d->m_listView);
        if(!view)
            return;
        if(view->viewMode() == QListView::ListMode)
        {
            QRectF rect;
            rect.setX(option.rect.x());
            rect.setY(option.rect.y());
            rect.setWidth(option.rect.width());
            rect.setHeight(option.rect.height());
            //QPainterPath画圆角矩形
            const qreal radius = 4;
            QPainterPath path;
            path.moveTo(rect.topRight() - QPointF(radius, 0));
            path.lineTo(rect.topLeft() + QPointF(radius, 0));
            path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
            path.lineTo(rect.bottomLeft() + QPointF(0, -radius));
            path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
            path.lineTo(rect.bottomRight() - QPointF(radius, 0));
            path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
            path.lineTo(rect.topRight() + QPointF(0, radius));
            path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));

            QString mainText = index.data(Qt::DisplayRole).toString();//获取主文本和副文本
            QString subText = index.data(Qt::UserRole).toString();

            if(subText.isNull())
            {
                auto *model =dynamic_cast<QStandardItemModel*>(const_cast<QAbstractItemModel*>(index.model()));//获取图标
                QIcon icon = model->item(index.row())->icon();
                QRect iconRect = QRect(rect.left()+8,rect.top()+10,d->m_listView->iconSize().width()+(height-23)*2,d->m_listView->iconSize().height()+(height-23)*2);// 绘制图标的位置
                QRect MainText = QRect(iconRect.right()+8,rect.top()+10+iconRect.height()/2-height/2,rect.width(),height); // 绘制文本的位置

                painter->save();

                //item可选中 (单选)
                QColor color;
                if (option.state.testFlag(QStyle::State_MouseOver)&&!(option.state.testFlag(QStyle::State_Selected)))
                {
                    if(ThemeController::themeMode() == ThemeFlag::DarkTheme)
                           {
                                color = option.palette.windowText().color();
                                color.setAlphaF(0.25);
                           }
                           else
                           {
                                color = option.palette.highlight().color().lighter(120);
                                color.setAlphaF(0.5);
                           }
                    painter->setPen(QPen(Qt::NoPen));
                    painter->setBrush(color);
                    painter->drawPath(path);
                }
                else if (option.state.testFlag(QStyle::State_Selected))
                    {
                        color = option.palette.highlight().color();
                        painter->setPen(QPen(Qt::NoPen));
                        painter->setBrush(color);
                        painter->drawPath(path);
                    }

                //icon.paint(painter,iconRect);
                QPixmap pixmap=icon.pixmap(d->m_listView->iconSize().width()+(height-23)*2,d->m_listView->iconSize().width()+(height-23)*2);
                painter->drawPixmap(iconRect,pixmap);


                //点击反白效果，画icon和text

                if(ThemeController::themeMode() == ThemeFlag::LightTheme)
                {
                    if(option.state.testFlag(QStyle::State_Selected))
                    {
                        painter->setPen(QColor(230,230,230));
                        painter->drawText(MainText,mainText);

                    }
                    else
                    {

                        painter->setPen(QColor(38,38,38));
                        painter->drawText(MainText,mainText);
                    }
                }
                else
                {

                    painter->setPen(QColor(230,230,230));
                    painter->drawText(MainText,mainText);
                }
                painter->restore();


            }
            else
            {
                auto *model =dynamic_cast<QStandardItemModel*>(const_cast<QAbstractItemModel*>(index.model()));//获取图标
                QIcon icon = model->item(index.row())->icon();
                QRect iconRect = QRect(rect.left()+8,rect.top()+10,d->m_listView->iconSize().width()+(height-23)*2,d->m_listView->iconSize().height()+(height-23)*2);// 绘制图标的位置
                QRect MainText = QRect(iconRect.right()+8,rect.top()+5,rect.width(),height); // 绘制两个文本的位置
                QRect SubText = QRect(iconRect.right()+8,rect.bottom()-5-height,rect.width(),height);

                painter->save();

                //item可选中 (单选)
                QColor color;
                if (option.state.testFlag(QStyle::State_MouseOver)&&!(option.state.testFlag(QStyle::State_Selected)))
                {
                    if(ThemeController::themeMode()==ThemeFlag::DarkTheme)
                           {
                                color = option.palette.windowText().color();
                                color.setAlphaF(0.25);
                           }
                           else
                           {
                                color = option.palette.highlight().color().lighter(120);
                                color.setAlphaF(0.5);
                           }
                    painter->setPen(QPen(Qt::NoPen));
                    painter->setBrush(color);
                    painter->drawPath(path);
                }
                else if (option.state.testFlag(QStyle::State_Selected))
                    {
                        color = option.palette.highlight().color();
                        painter->setPen(QPen(Qt::NoPen));
                        painter->setBrush(color);
                        painter->drawPath(path);
                    }

                //icon.paint(painter,iconRect);
                QPixmap pixmap=icon.pixmap(QSize(d->m_listView->iconSize().width()+(height-23)*2,d->m_listView->iconSize().height()+(height-23)*2));
                painter->drawPixmap(iconRect,pixmap);

                //点击反白效果，画icon和text
                if(ThemeController::themeMode() == ThemeFlag::LightTheme)
                {
                    if(option.state.testFlag(QStyle::State_Selected))
                    {
                        painter->setPen(QColor(230,230,230));
                        painter->drawText(MainText,mainText);

                        painter->setPen(QColor(169,169,169));
                        painter->drawText(SubText,subText);
                    }
                    else
                    {
                        painter->setPen(QColor(38,38,38));
                        painter->drawText(MainText,mainText);

                        painter->setPen(QColor(169,169,169));
                        painter->drawText(SubText,subText);
                    }
                }
                else
                {
                    painter->setPen(QColor(230,230,230));
                    painter->drawText(MainText,mainText);

                    painter->setPen(QColor(169,169,169));
                    painter->drawText(SubText,subText);
                }
                painter->restore();
            }
        }
        else if(view->viewMode() == QListView::IconMode)
        {
            QStyleOptionViewItem  styleOptionViewItem( option ); //定义
            initStyleOption( &styleOptionViewItem, index);  //初始化
            QStyle *pStyle = styleOptionViewItem.widget->style();
            pStyle->drawControl(QStyle::CE_ItemViewItem, &styleOptionViewItem, painter,  styleOptionViewItem.widget);
        }
    }

}

QSize KListViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_D(const KListViewDelegate);

    QFont font; //获取系统字体大小
    font = QApplication::font();
    QFontMetrics fm(font);
    int height = fm.height();

    return QSize(option.rect.width(),height*2 + 6);
}

}

#include "klistviewdelegate.moc"
#include "moc_klistviewdelegate.cpp"
