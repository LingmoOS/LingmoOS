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

#include "kitemwidget.h"
#include "klistwidget.h"
#include <QListWidgetItem>
#include <QDebug>
#include <QPainter>

namespace  kdk
{


class KItemWidgetPrivate : public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KItemWidget)

public:
    KItemWidgetPrivate(KItemWidget* parent);


protected:
    void changeTheme();
private:
    KItemWidget* q_ptr;
    QIcon m_icon;
    QString m_maintext;
    QString m_sectext;
    QSize m_iconsize;
    QPixmap pixmap;
    int x;
    int y;
    KItemWidget* m_ltem;
    bool flag = false;

};

KItemWidgetPrivate::KItemWidgetPrivate (KItemWidget* parent):q_ptr(parent)
{
    Q_Q(KItemWidget);
   setParent(parent);
    m_iconsize=QSize(35,35);
    x = m_iconsize.width();
    y = m_iconsize.height();
    changeTheme();

    connect(m_gsetting,&QGSettings::changed,this,&KItemWidgetPrivate::changeTheme);

}



void KItemWidgetPrivate::changeTheme()
{
    Q_Q(KItemWidget);
    initThemeStyle();

}



KItemWidget::KItemWidget(const QIcon &Myicon, QString MmainText, QString MsecText, QWidget *parent):QWidget(parent),d_ptr(new KItemWidgetPrivate(this))
{
    Q_D(KItemWidget);
    setMinimumSize(800,800);

    d->pixmap=Myicon.pixmap(QSize(d->m_iconsize));



    d->m_maintext=MmainText;

    d->m_sectext=MsecText;
}

void KItemWidget::SetInverse()
{
    Q_D(KItemWidget);
    d->flag = true;
    update();
}

void KItemWidget::CancelInverse()
{
    Q_D(KItemWidget);
    d->flag = false;
    update();
}


void KItemWidget::SetIconSize(QSize size)
{
    Q_D(KItemWidget);
    d->m_iconsize=size; 
    d->x = d->m_iconsize.width();
    d->y = d->m_iconsize.height();
    update();
}


void KItemWidget::paintEvent(QPaintEvent *event)
{
    Q_D(KItemWidget);    



    QPainter painter(this);

    painter.drawPixmap(8,10,d->x,d->y,d->pixmap); //画图片



    QFont font=painter.font();
    font.setPointSize(10);//设置字体
    painter.setFont(font);
    if(ThemeController::themeMode()==LightTheme) //判断主题
    {
        if(d->flag)
        {
            painter.setPen(QColor(255,255,255)); //白色
            painter.drawText(QRect(52,10,1000,16),d->m_maintext);//画文本
        }
        else
        {
            painter.setPen(QColor(54,54,54));//黑色
            painter.drawText(QRect(52,10,1000,16),d->m_maintext);//画文本
        }
        painter.setPen(QColor(150,150,150));//褐色
        painter.drawText(QRect(52,28,1000,16),d->m_sectext);
    }
    else if(ThemeController::themeMode() == DarkTheme)
    {
        painter.setPen(QColor(255,255,255)); //白色
        painter.drawText(QRect(52,10,1000,16),d->m_maintext);//画文本

        painter.setPen(QColor(150,150,150));//褐色
        painter.drawText(QRect(52,28,1000,16),d->m_sectext);
    }


    painter.setRenderHint(QPainter::Antialiasing);
    /* 尽可能消除文本锯齿边缘 */
    painter.setRenderHint(QPainter::TextAntialiasing);

    this->setContentsMargins(8,10,10,5);//边距


}



}
#include "kitemwidget.moc"
#include "moc_kitemwidget.cpp"
