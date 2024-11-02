/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include "customstyle.h"

#include <QStyleOptionToolButton>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
CustomStyle::CustomStyle(const QString &proxyStyleName, QObject *parent) : QProxyStyle (proxyStyleName)
{
    Q_UNUSED(parent);
    m_helpTip = new SliderTipLabelHelper(this);
}
CustomStyle::~CustomStyle()
{
}

void CustomStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    if(control == CC_ToolButton)
    {
        /// 我们需要获取ToolButton的详细信息，通过qstyleoption_cast可以得到
        /// 对应的option，通过拷贝构造函数得到一份备份用于绘制子控件
        /// 我们一般不用在意option是怎么得到的，大部分的Qt控件都能够提供了option的init方法

    }
    return QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void CustomStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{

    return QProxyStyle::drawControl(element, option, painter, widget);
}

void CustomStyle::drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment, const QPixmap &pixmap) const
{
    return QProxyStyle::drawItemPixmap(painter, rectangle, alignment, pixmap);
}

void CustomStyle::drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    return QProxyStyle::drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
}

//绘制简单的颜色圆角等
void CustomStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    //绘制 ToolButton
    case PE_PanelButtonTool:{
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0xff,0xff,0xff,0x00));
        painter->drawRoundedRect(option->rect,4,4);
        if (option->state & State_MouseOver) {
            if (option->state & State_Sunken) {
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(0xff,0xff,0xff,0x14));
                painter->drawRoundedRect(option->rect,4,4);
                qDebug() << " 点击按钮";
            } else {
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(0xff,0xff,0xff,0x1f));
                painter->drawRoundedRect(option->rect,4,4);
                qDebug() << "悬停按钮";
            }
        }
        painter->restore();
        return;
    }
    case PE_PanelTipLabel:{
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0xff,0xff,0x00,0xff));
        painter->drawRoundedRect(option->rect,4,4);
        painter->restore();
        return;
    }break;
    case PE_PanelButtonCommand:{
        painter->save();
        painter->setRenderHint(QPainter::TextAntialiasing,true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0xff,0xff,0xff,0x00));
        if (option->state & State_MouseOver) {
            if (option->state & State_Sunken) {
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(0x3d,0x6b,0xe5,0xff));
                painter->drawRoundedRect(option->rect,4,4);
            } else {
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(0xff,0xff,0xff,0x1f));
                painter->drawRoundedRect(option->rect.adjusted(2,2,-2,-2),4,4);
            }
        }
        painter->restore();
        return;
    }break;
    }
    return QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QPixmap CustomStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const
{
    return QProxyStyle::generatedIconPixmap(iconMode, pixmap, option);
}

QStyle::SubControl CustomStyle::hitTestComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, const QPoint &position, const QWidget *widget) const
{
    return QProxyStyle::hitTestComplexControl(control, option, position, widget);
}

QRect CustomStyle::itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap &pixmap) const
{
    return QProxyStyle::itemPixmapRect(rectangle, alignment, pixmap);
}

QRect CustomStyle::itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment, bool enabled, const QString &text) const
{
    return QProxyStyle::itemTextRect(metrics, rectangle, alignment, enabled, text);
}

//
int CustomStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric){
    case PM_ToolBarIconSize:{
        return (int)48*qApp->devicePixelRatio();
    }
    default:
        break;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

//
void CustomStyle::polish(QWidget *widget)
{
    if (widget) {
        if (widget->inherits("QTipLabel")) {
            widget->setAttribute(Qt::WA_TranslucentBackground);
            QPainterPath path;
            auto rect = widget->rect();
            rect.adjust(0,0,0,0);
            path.addRoundedRect(rect,6,6);
            widget->setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
        }
    }
    if (widget) {
        if (widget->inherits("QLable")) {
            const_cast<QWidget *> (widget)->setAttribute(Qt::WA_TranslucentBackground);
            widget->setAttribute(Qt::WA_TranslucentBackground);
            QPainterPath path;
            auto rect = widget->rect();
            rect.adjust(0,0,0,0);
            path.addRoundedRect(rect,6,6);
            widget->setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
        }
    }
    if (widget){
        widget->setAttribute(Qt::WA_Hover);
        widget->inherits("QSlider");
        m_helpTip->registerWidget(widget);
        widget->installEventFilter(this);
    }

    return QProxyStyle::polish(widget);
}

void CustomStyle::polish(QApplication *application)
{
    return QProxyStyle::polish(application);
}

//
void CustomStyle::polish(QPalette &palette)
{
    //    return QProxyStyle::polish(palette);
    //    QProxyStyle::polish(palette);
    //    palette.setBrush(QPalette::Foreground, Qt::black);
    QColor lightBlue(200, 0, 0);
    palette.setBrush(QPalette::Highlight, lightBlue);
}

void CustomStyle::unpolish(QWidget *widget)
{
    return QProxyStyle::unpolish(widget);
}

void CustomStyle::unpolish(QApplication *application)
{
    return QProxyStyle::unpolish(application);
}

QSize CustomStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const
{
    return QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
}

QIcon CustomStyle::standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::standardIcon(standardIcon, option, widget);
}

QPalette CustomStyle::standardPalette() const
{
    return QProxyStyle::standardPalette();
}

//如果需要背景透明也许需要用到这个函数
int CustomStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    switch (hint) {
    /// 让ScrollView viewport的绘制区域包含scrollbar和corner widget
    /// 这个例子中没有什么作用，如果我们需要绘制一个背景透明的滚动条
    /// 这个style hint对我们的意义应该很大，因为我们希望视图能够帮助
    /// 我们填充滚动条的背景区域，否则当背景透明时底下会出现明显的分割

    case SH_ScrollView_FrameOnlyAroundContents: {
        return false;
    }
    default:
        break;
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

QRect CustomStyle::subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option, QStyle::SubControl subControl, const QWidget *widget) const
{
    return QProxyStyle::subControlRect(control, option, subControl, widget);
}

QRect CustomStyle::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::subElementRect(element, option, widget);
}
