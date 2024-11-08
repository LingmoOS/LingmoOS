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

#include "kbackgroundgroup.h"
#include "themeController.h"
#include <QVBoxLayout>
#include "klineframe.h"
#include <QPainter>
#include <QPainterPath>
#include <QEvent>

namespace kdk {

class KBackgroundGroupPrivate : public QObject ,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KBackgroundGroup)
public:
    enum WidgetPosition
    {
        Beginning,
        Middle,
        End
    };
    KBackgroundGroupPrivate(KBackgroundGroup* parent);
    void updateLayout();
private:
    KBackgroundGroup* q_ptr;
    QVBoxLayout* m_pmainWidgetLayout;
    QList<QWidget*> m_pwidgetList;
    QList<QWidget*> m_pwidgetStateList;
    QPalette::ColorRole m_brushColorRole;
    QColor m_backgroundColor;
    WidgetPosition m_widgetPosition;
    QRect m_rect;
    int m_rectLocal;
    int m_radius;
};

KBackgroundGroup::KBackgroundGroup(QWidget *parent)
    :QFrame(parent)
    ,d_ptr(new KBackgroundGroupPrivate(this))
{
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Plain);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
}

void KBackgroundGroup::addWidget(QWidget *widget)
{
    Q_D(KBackgroundGroup);
    if(widget->maximumHeight() != widget->minimumHeight())
        widget->setFixedHeight(60);
    if(!d->m_pwidgetList.contains(widget))
        d->m_pwidgetList.append(widget);
    else
        return;
    d->updateLayout();
}

void KBackgroundGroup::addWidgetList(QList<QWidget *> list)
{
    Q_D(KBackgroundGroup);
    for (int i = 0;  i < list.count();i++)
    {
        auto widget = list.at(i);
        if(widget->maximumHeight() != widget->minimumHeight())
            widget->setFixedHeight(60);
        if(!d->m_pwidgetList.contains(list.at(i)))
            d->m_pwidgetList.append(list.at(i));
        else
            continue ;
    }
    d->updateLayout();
}

void KBackgroundGroup::removeWidgetAt(int i)
{
    Q_D(KBackgroundGroup);
    if(d->m_pwidgetList.contains(d->m_pwidgetList.at(i)))
        d->m_pwidgetList.removeAt(i);
    else
        return;
    d->updateLayout();
}

void KBackgroundGroup::removeWidget(QWidget *widget)
{
    Q_D(KBackgroundGroup);
    if(d->m_pwidgetList.contains(widget))
        d->m_pwidgetList.removeOne(widget);
    else
        return ;
    d->updateLayout();
}

void KBackgroundGroup::removeWidgetList(QList<QWidget *> list)
{
    Q_D(KBackgroundGroup);
    for (int i = 0;  i < list.count();i++)
    {
        if(d->m_pwidgetList.contains(list.at(i)))
            d->m_pwidgetList.removeOne(list.at(i));
        else
            continue ;
    }
    d->updateLayout();
}

void KBackgroundGroup::insertWidgetAt(int index, QWidget *widget)
{
    Q_D(KBackgroundGroup);
    if(widget->maximumHeight() != widget->minimumHeight())
        widget->setFixedHeight(60);
    if(!d->m_pwidgetList.contains(widget))
        d->m_pwidgetList.insert(index,widget);
    else
        return ;
    d->updateLayout();
}

void KBackgroundGroup::insertWidgetList(int index, QList<QWidget *> list)
{
    Q_D(KBackgroundGroup);
    for (int i = 0;  i < list.count();i++)
    {
        auto widget = list.at(i);
        if(widget->maximumHeight() != widget->minimumHeight())
            widget->setFixedHeight(60);
        if(!d->m_pwidgetList.contains(list.at(i)))
            d->m_pwidgetList.insert(index++,list.at(i));
        else
            continue ;
    }
    d->updateLayout();
}

void KBackgroundGroup::setBorderRadius(int radius)
{
    Q_D(KBackgroundGroup);
    d->m_radius = radius;
}

int KBackgroundGroup::borderRadius()
{
    Q_D(KBackgroundGroup);
    return d->m_radius;
}

void KBackgroundGroup::setBackgroundRole(QPalette::ColorRole role)
{
    Q_D(KBackgroundGroup);
    d->m_brushColorRole = role;
}

QPalette::ColorRole KBackgroundGroup::backgroundRole() const
{
    Q_D(const KBackgroundGroup);
    return d->m_brushColorRole ;
}

void KBackgroundGroup::setStateEnable(QWidget *widget, bool flag)
{
    Q_D(KBackgroundGroup);
    if(!d->m_pwidgetStateList.contains(widget) && flag)
    {
        d->m_pwidgetStateList.append(widget);
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_TranslucentBackground);
    }
}

QList<QWidget *> KBackgroundGroup::widgetList()
{
    Q_D(KBackgroundGroup);
    return d->m_pwidgetList;
}

void KBackgroundGroup::paintEvent(QPaintEvent *event)
{
    Q_D(KBackgroundGroup);
    //绘制backgroundgroup
    QRect rect =this->rect();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);
    painter.setBrush(palette().color(d->m_brushColorRole));

    QPainterPath path;
    path.moveTo(rect.topLeft() + QPointF(d->m_radius , 0));
    path.quadTo(rect.topLeft() , rect.topLeft() + QPointF(0,d->m_radius));
    path.lineTo(rect.bottomLeft() - QPointF(0, d->m_radius));
    path.quadTo(rect.bottomLeft() , rect.bottomLeft() + QPointF(d->m_radius,0));
    path.lineTo(rect.bottomRight() - QPointF(d->m_radius,0));
    path.quadTo(rect.bottomRight() , rect.bottomRight() - QPointF(0,d->m_radius));
    path.lineTo(rect.topRight() + QPointF(0,d->m_radius));
    path.quadTo(rect.topRight() , rect.topRight() - QPointF(d->m_radius,0));
    path.lineTo(rect.topLeft() + QPointF(d->m_radius , 0));
    painter.drawPath(path);

    //三态事件响应颜色
    painter.save();
    painter.setBrush(d->m_backgroundColor);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);

    //确定三态区域
    int index,offset=0;
    for(index =0;index<d->m_pwidgetList.count()-1;index++)
    {
        auto widget = d->m_pwidgetList.at(index);
        if( widget == d->m_pwidgetStateList.at(d->m_rectLocal))
            break;
        else
            offset = offset + d->m_pwidgetList.at(index)->height();
    }

    d->m_rect = d->m_rect.translated(0,offset+index);
    d->m_rect = d->m_rect.adjusted(0,0,2,2);

    //三态区域颜色调整
    QPainterPath path1;
    if(d->m_pwidgetList.count() == 1 && d->m_pwidgetStateList.count() == 1)
    {
         painter.drawRoundedRect(d->m_rect,d->m_radius,d->m_radius);
    }
    else if(d->m_pwidgetList.count() > 1)
    {
        switch (d->m_widgetPosition)
        {
        case KBackgroundGroupPrivate::Beginning:
            path1.moveTo(d->m_rect.topLeft() + QPointF(0,d->m_radius));
            path1.lineTo(d->m_rect.bottomLeft());
            path1.lineTo(d->m_rect.bottomRight());
            path1.lineTo(d->m_rect.topRight() + QPointF(0,d->m_radius));
            path1.quadTo(d->m_rect.topRight() , d->m_rect.topRight() - QPointF(d->m_radius,0));
            path1.lineTo(d->m_rect.topLeft() + QPointF(d->m_radius,0));
            path1.quadTo(d->m_rect.topLeft() , d->m_rect.topLeft() + QPointF(0,d->m_radius));
            painter.drawPath(path1);
            break;
        case KBackgroundGroupPrivate::Middle:
            painter.drawRoundedRect(d->m_rect,0,0);
            break;
        case KBackgroundGroupPrivate::End:
            path1.moveTo(d->m_rect.topLeft());
            path1.lineTo(d->m_rect.bottomLeft() - QPointF(0,d->m_radius));
            path1.quadTo(d->m_rect.bottomLeft() , d->m_rect.bottomLeft() + QPointF(d->m_radius,0));
            path1.lineTo(d->m_rect.bottomRight() - QPointF(d->m_radius,0));
            path1.quadTo(d->m_rect.bottomRight() , d->m_rect.bottomRight() - QPointF(0,d->m_radius));
            path1.lineTo(d->m_rect.topRight());
            path1.lineTo(d->m_rect.topLeft());
            painter.drawPath(path1);
            break;
        default:
            break;
        }
    }
    painter.restore();
}

bool KBackgroundGroup::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KBackgroundGroup);
    auto widget = static_cast<QWidget*>(watched);

    if(d->m_pwidgetStateList.contains(widget))
    {
        //事件过滤器判断事件设置颜色
        QColor color;
        switch (event->type())
        {
        case QEvent::HoverEnter:
            d->m_rect = widget->rect();
            //设置三态区域背景色
            if(ThemeController::themeMode() == LightTheme)
            {
                color = palette().button().color();//QColor("#E6E6E6");
                d->m_backgroundColor = ThemeController::mixColor(color,Qt::black,0.05);
            }
            else
            {
                color =palette().base().color();// QColor("#373737");
                d->m_backgroundColor = ThemeController::mixColor(color,Qt::white,0.2);
            }
            //获取三态区域位置
            if(d->m_pwidgetList.at(0) == widget)
                d->m_widgetPosition = KBackgroundGroupPrivate::Beginning;
            else if(d->m_pwidgetList.at(d->m_pwidgetList.count()-1) == widget)
                d->m_widgetPosition = KBackgroundGroupPrivate::End;
            else
                d->m_widgetPosition = KBackgroundGroupPrivate::Middle;
            d->m_rectLocal =d->m_pwidgetStateList.indexOf(widget);
            update();
            break;
        case QEvent::Enter:
            d->m_rect = widget->rect();
            if(ThemeController::themeMode() == LightTheme)
            {
                color = palette().button().color();
                d->m_backgroundColor = ThemeController::mixColor(color,Qt::black,0.05);
            }
            else
            {
                color = palette().base().color();
                d->m_backgroundColor = ThemeController::mixColor(color,Qt::white,0.2);
            }
            if(d->m_pwidgetList.at(0) == widget)
                d->m_widgetPosition = KBackgroundGroupPrivate::Beginning;
            else if(d->m_pwidgetList.at(d->m_pwidgetList.count()-1) == widget)
                d->m_widgetPosition = KBackgroundGroupPrivate::End;
            else
                d->m_widgetPosition = KBackgroundGroupPrivate::Middle;
            d->m_rectLocal =d->m_pwidgetStateList.indexOf(widget);
            update();
            break;
        case QEvent::MouseButtonPress:
            d->m_rect = widget->rect();
            if(ThemeController::themeMode() == LightTheme)
            {
                color = palette().button().color();
                d->m_backgroundColor = ThemeController::mixColor(color,Qt::black,0.2);
            }
            else
            {
                color = palette().base().color();
                d->m_backgroundColor = ThemeController::mixColor(color,Qt::white,0.05);
            }
            if(d->m_pwidgetList.at(0) == widget)
                d->m_widgetPosition = KBackgroundGroupPrivate::Beginning;
            else if(d->m_pwidgetList.at(d->m_pwidgetList.count()-1) == widget)
                d->m_widgetPosition = KBackgroundGroupPrivate::End;
            else
                d->m_widgetPosition = KBackgroundGroupPrivate::Middle;
            d->m_rectLocal =d->m_pwidgetStateList.indexOf(widget);
            update();
            break;
        case QEvent::MouseButtonRelease:
        {
            d->m_rect = widget->rect();
            if(ThemeController::themeMode() == LightTheme)
            {
                color = palette().button().color();
                d->m_backgroundColor = ThemeController::mixColor(color,Qt::black,0.05);
            }
            else
            {
                color = palette().base().color();
                d->m_backgroundColor = ThemeController::mixColor(color,Qt::white,0.2);
            }
            if(d->m_pwidgetList.at(0) == widget)
                d->m_widgetPosition = KBackgroundGroupPrivate::Beginning;
            else if(d->m_pwidgetList.at(d->m_pwidgetList.count()-1) == widget)
                d->m_widgetPosition = KBackgroundGroupPrivate::End;
            else
                d->m_widgetPosition = KBackgroundGroupPrivate::Middle;
            Q_EMIT clicked(widget);
            d->m_rectLocal =d->m_pwidgetStateList.indexOf(widget);
            update();
            break;
        }
        case QEvent::Leave:
            d->m_rect = widget->rect();
            d->m_backgroundColor = palette().color(QPalette::Base);
            if(d->m_pwidgetList.at(0) == widget)
                d->m_widgetPosition = KBackgroundGroupPrivate::Beginning;
            else if(d->m_pwidgetList.at(d->m_pwidgetList.count()-1) == widget)
                d->m_widgetPosition = KBackgroundGroupPrivate::End;
            else
                d->m_widgetPosition = KBackgroundGroupPrivate::Middle;
            d->m_rectLocal =d->m_pwidgetStateList.indexOf(widget);
            update();
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(watched,event);
}

KBackgroundGroupPrivate::KBackgroundGroupPrivate(KBackgroundGroup *parent)
    :q_ptr(parent)
    ,m_widgetPosition(KBackgroundGroupPrivate::Beginning),m_rectLocal(0)
    ,m_radius(12),m_backgroundColor(parent->palette().color(QPalette::Base))
    ,m_brushColorRole(QPalette::Base)
{
    Q_Q(KBackgroundGroup);
    m_pmainWidgetLayout = new QVBoxLayout(q);
    m_pmainWidgetLayout->setContentsMargins(0,0,0,0);
    m_pmainWidgetLayout->setSpacing(0);
    m_pmainWidgetLayout->setMargin(0);

//    q->setFocus(Qt::MouseFocusReason);
    connect(m_gsetting,&QGSettings::changed,this,[=](){
        initThemeStyle();
    });
}

void KBackgroundGroupPrivate::updateLayout()
{
    Q_Q(KBackgroundGroup);
    //清空
    QLayoutItem *child;
    while ((child = m_pmainWidgetLayout->takeAt(0)) != 0)
    {
        //删除Stretch(弹簧)等布局
        if (child->spacerItem())
        {
            m_pmainWidgetLayout->removeItem(child);
            continue;
        }
        //删除布局
        m_pmainWidgetLayout->removeWidget(child->widget());
        child->widget()->setParent(nullptr);
        delete child;
        child =nullptr;
    }

    //添加布局
    if(m_pwidgetList.count() < 1)
        return;
    else if(m_pwidgetList.count() == 1)
    {
        m_pmainWidgetLayout->addWidget(m_pwidgetList.at(0));
    }
    else
    {
        for (int i = 0; i < m_pwidgetList.count(); ++i)
        {
              m_pmainWidgetLayout->addWidget(m_pwidgetList.at(i));
              if(i  != m_pwidgetList.count()-1)
              {
                  KHLineFrame* frame = new KHLineFrame();
                  m_pmainWidgetLayout->addWidget(frame);
              }
        }
    }
}

}

#include "kbackgroundgroup.moc"
#include "moc_kbackgroundgroup.cpp"
