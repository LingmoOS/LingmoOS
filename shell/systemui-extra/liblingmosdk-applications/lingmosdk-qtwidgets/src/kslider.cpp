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

#include "kslider.h"
#include "themeController.h"
#include <QPainter>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QStyleOptionButton>
#include <QPalette>
#include <QMouseEvent>
#include <QToolTip>
#include <QPainterPath>
#include <QDebug>
#include <QVariantAnimation>
#include "parmscontroller.h"

using namespace kdk;

namespace kdk {

static const int node_radius = 5;
static const int handle_radius = 10;
static const int spaceing = handle_radius+2;
static const int line_width = 4;
static const int line_border_radius = 2;

class KSliderPrivate :public QObject,public ThemeController
{
    Q_DECLARE_PUBLIC(KSlider)
public:
    KSliderPrivate(KSlider*parent);
    ~KSliderPrivate();
    void animationStart();

private:
    KSlider* q_ptr;
    KSliderType m_sliderType;
    void drawCoverNode(QPainter*painter);
    void drawSlider(QPainter*painter);
    void locateNode();
    void drawBasePath(QPainter*painter);
    void drawCoverLine(QPainter*painter);
    void updateValuePosition();
    void updateValue();
    void updateColor(QPalette palette);
    void changeTheme();
    QRect baseLineRect();
    QRect coverLineRect();
    QList<QPoint>m_nodes;
    bool m_isPressed;
    bool m_isHovered;
    int m_valuePosition;
    QPoint m_currentpos;
    QColor m_baseColor;
    QColor m_coverColor;
    QColor m_startColor;
    QColor m_endColor;
    bool m_nodeVisible;
    QRect m_rect;
    QString m_pToolTipText;
    QLinearGradient m_linearGradient;
    bool m_isTranslucent;
    bool m_isTranslucentFlag;
    int m_handleRadius;
    QVariantAnimation *m_animation;
    bool m_animationStarted;
    bool m_ismoving;
    bool m_resize;
    bool m_forceUpdate;
};

KSliderPrivate::KSliderPrivate(KSlider *parent)
    :q_ptr(parent),
      m_sliderType(SmoothSlider),
      m_isPressed(false),
      m_isHovered(false),
      m_valuePosition(0),
      m_currentpos(QPoint(0,0)),
      m_nodeVisible(true),
      m_handleRadius(0),
      m_animationStarted(false),
      m_ismoving(false),
      m_resize(false),
      m_forceUpdate(false),
      m_isTranslucent(false),
      m_isTranslucentFlag(false)
{
    Q_Q(KSlider);
    q->setProperty("animation",false);
    QStyleOptionSlider sliderOption;
    m_animation = new QVariantAnimation(this);
    m_animation->setDuration(150);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_animation, &QVariantAnimation::finished, q, [=](){
        m_animationStarted = false;
    });
    connect(m_animation, &QVariantAnimation::valueChanged, q, [=](){
        q->update();
    });
    animationStart();
    connect(q,&KSlider::valueChanged,this,[=](){
        animationStart();
    });
    //获取滑动块大小
    QRect handle = q->style()->proxy()->subControlRect(QStyle::CC_Slider, &sliderOption, QStyle::SC_SliderHandle, q);
    m_handleRadius =handle.width()/2;
}

KSliderPrivate::~KSliderPrivate()
{

}

void KSliderPrivate::animationStart()
{
    Q_Q(KSlider);
    int prePosition = m_valuePosition;
    updateValuePosition();

    if((m_forceUpdate||!m_animationStarted) && !m_ismoving)
    {
        m_animation->stop();
        m_animation->setStartValue(prePosition);
        m_animation->setEndValue(m_valuePosition);
        m_animation->start();
        m_animationStarted = true;
        m_forceUpdate = false;
        m_resize = false;
    }
}

void KSliderPrivate::drawCoverNode(QPainter*painter)
{
    Q_Q(KSlider);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(q->palette().color(QPalette::Highlight));
    int side = 0;
    if( ThemeController::widgetTheme() == ClassicTheme )
    {
        if(ThemeController::themeMode() == LightTheme)
        {
            painter->setPen(ThemeController::lanhuHSLToQtHsl(0, 0, 59));
            painter->setBrush(ThemeController::ThemeController::lanhuHSLToQtHsl(0, 0, 85));

        }
        else
        {
            painter->setPen(ThemeController::lanhuHSLToQtHsl(0, 0, 50));
            painter->setBrush(ThemeController::ThemeController::lanhuHSLToQtHsl(0, 0, 17));
        }
        if(q->orientation() == Qt::Horizontal)
        {
            for(auto point : m_nodes)
            {
                if(m_ismoving)
                    side = m_currentpos.x();
                else if(m_resize)
                    side = m_valuePosition;
                else
                    side = m_animation->currentValue().toInt();
                QRect rect(point.x()-1,point.y()-5,3,10);
                if(point.x()<= side)
                    painter->drawRect(rect);//选中基点
            }
        }
        else
        {
            for(auto point : m_nodes)
            {
                if(m_ismoving)
                    side = m_currentpos.y();
                else if(m_resize)
                    side = m_valuePosition;
                else
                    side = m_animation->currentValue().toInt();
                QRect rect(point.x()-5,point.y()-3,10,3);
                if(point.y()>= side)
                    painter->drawRect(rect);
            }
        }
    }
    else
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(q->palette().color(QPalette::Highlight));
        if(q->orientation() == Qt::Horizontal)
        {
            for(auto point : m_nodes)
            {
                if(m_ismoving)
                    side = m_currentpos.x();
                else if(m_resize)
                    side = m_valuePosition;
                else
                    side = m_animation->currentValue().toInt();
//                if(point.x()<= side)
                if(q->layoutDirection() == Qt::LeftToRight && (point.x()<= side))
                    painter->drawEllipse(point,Parmscontroller::parm(Parmscontroller::Parm::PM_SliderNodeRadius)/2,Parmscontroller::parm(Parmscontroller::Parm::PM_SliderNodeRadius)/2);
                else if(q->layoutDirection() == Qt::RightToLeft && (point.x()>= side))
                    painter->drawEllipse(point,Parmscontroller::parm(Parmscontroller::Parm::PM_SliderNodeRadius)/2,Parmscontroller::parm(Parmscontroller::Parm::PM_SliderNodeRadius)/2);
            }
        }
        else
        {
            for(auto point : m_nodes)
            {
                if(m_ismoving)
                    side = m_currentpos.y();
                else if(m_resize)
                    side = m_valuePosition;
                else
                    side = m_animation->currentValue().toInt();
                if(point.y()>= side)
                    painter->drawEllipse(point,Parmscontroller::parm(Parmscontroller::Parm::PM_SliderNodeRadius)/2,Parmscontroller::parm(Parmscontroller::Parm::PM_SliderNodeRadius)/2);
            }
        }
    }
}

void KSliderPrivate::drawSlider(QPainter*painter)
{
    Q_Q(KSlider);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    //填充颜色待定
    if(ThemeController::widgetTheme() == FashionTheme && m_isHovered&&!m_isPressed && q->isEnabled())
    {
        painter->setBrush(m_linearGradient);
    }
    else
        painter->setBrush(m_startColor);
    //获取滑动块大小

    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        painter->setRenderHint(QPainter::HighQualityAntialiasing,true);
        painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(q->palette().highlight().color());
        double handle_radius =4;
        QPainterPath path;
        QPoint point;
        if(q->orientation() == Qt::Horizontal)
        {
            int x = 0;
            if(m_ismoving)
                x = m_currentpos.x();
            else if(m_resize)
                x = m_valuePosition;
            else
                x= m_animation->currentValue().toInt();
            point = QPoint(x,q->height()/2);
            m_rect=QRect(point.x()-handle_radius/2,point.y()-handle_radius,handle_radius,handle_radius*2);
            path.moveTo(point.x()-handle_radius , point.y()-handle_radius*2);
            path.lineTo(point.x()-handle_radius , point.y()+handle_radius);
            path.lineTo(point.x() , point.y()+handle_radius*2+1);
            path.lineTo(point.x()+handle_radius , point.y()+handle_radius);
            path.lineTo(point.x()+handle_radius , point.y()-handle_radius*2);
            path.lineTo(point.x()-handle_radius , point.y()-handle_radius*2);
        }
        else
        {
            int y = 0;
            if(m_ismoving)
                y = m_currentpos.y();
            else if(m_resize)
                y = m_valuePosition;
            else
                y = m_animation->currentValue().toInt();
            point = QPoint(q->width()/2,y);
            m_rect=QRect(point.x()-handle_radius,point.y()-handle_radius/2,handle_radius*2,handle_radius);
            path.moveTo(point.x()+handle_radius*2 , point.y()-handle_radius);
            path.lineTo(point.x()-handle_radius , point.y()-handle_radius);
            path.lineTo(point.x()-handle_radius*2-1 , point.y());
            path.lineTo(point.x()-handle_radius , point.y()+handle_radius);
            path.lineTo(point.x()+handle_radius*2 , point.y()+handle_radius);
            path.lineTo(point.x()+handle_radius*2 , point.y()-handle_radius);
        }
        painter->drawPath(path);
    }
    else
    {
        painter->setPen(Qt::NoPen);
        int handle_radius  = Parmscontroller::parm(Parmscontroller::Parm::PM_SliderHandleRadius)/2;
        QPoint point;
        if(q->orientation() == Qt::Horizontal)
        {
            int x = 0;
            if(m_ismoving)
                x = m_currentpos.x();
            else if(m_resize)
                x = m_valuePosition;
            else
                x= m_animation->currentValue().toInt();
            point = QPoint(x,q->height()/2);
        }
        else
        {
            int y = 0;
            if(m_ismoving)
                y = m_currentpos.y();
            else if(m_resize)
                y = m_valuePosition;
            else
                y = m_animation->currentValue().toInt();
            point = QPoint(q->width()/2,y);
        }
        painter->drawEllipse(point,handle_radius,handle_radius);
        m_rect=QRect(point.x()-handle_radius,point.y()-handle_radius,handle_radius*2,handle_radius*2);
    }
}

void KSliderPrivate::locateNode()
{
    Q_Q(KSlider);
    if(q->orientation() == Qt::Horizontal)
    {
        m_nodes.clear();
        if(q->layoutDirection() == Qt::LeftToRight)
        {
            QPoint beginNode(baseLineRect().left(),q->height()/2);
            QPoint endNode(baseLineRect().right(),q->height()/2);
            m_nodes.append(beginNode);

            if(!q->tickInterval())
            {
                m_nodes.append(endNode);
                return;
            }

            for(int i = q->minimum() + q->tickInterval();i < q->maximum();i += q->tickInterval())
            {
                QPoint point;
                int x = baseLineRect().left()+baseLineRect().width()*(i-q->minimum())/(q->maximum()-q->minimum());
                int y = q->height()/2;
                point.setX(x);
                point.setY(y);
                m_nodes.append(point);
            }
            m_nodes.append(endNode);
        }
        else
        {
            QPoint beginNode(baseLineRect().right(),q->height()/2);
            QPoint endNode(baseLineRect().left(),q->height()/2);
            m_nodes.append(beginNode);

            if(!q->tickInterval())
            {
                m_nodes.append(endNode);
                return;
            }

            for(int i = q->minimum() + q->tickInterval();i < q->maximum();i += q->tickInterval())
            {
                QPoint point;
                int x = baseLineRect().right()- baseLineRect().width()*(i-q->minimum())/(q->maximum()-q->minimum());
                int y = q->height()/2;
                point.setX(x);
                point.setY(y);
                m_nodes.append(point);
            }
            m_nodes.append(endNode);
        }
    }
    else
    {
        m_nodes.clear();
        QPoint beginNode(q->width()/2,baseLineRect().bottom());
        QPoint endNode(q->width()/2,baseLineRect().top());
        m_nodes.append(beginNode);
        if(!q->tickInterval())
        {
            m_nodes.append(endNode);
            return;
        }
        for(int i = q->minimum() + q->tickInterval();i < q->maximum();i += q->tickInterval())
        {
            QPoint point;
            int x = q->width()/2;
            int y = baseLineRect().top() + baseLineRect().height()*(i-q->minimum())/(q->maximum()-q->minimum());
            point.setX(x);
            point.setY(y);
            m_nodes.append(point);
        }
        m_nodes.append(endNode);
    }
}

void KSliderPrivate::drawBasePath(QPainter*painter)
{
    Q_Q(KSlider);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_baseColor);
     QPainterPath basePath;
    basePath.addRoundedRect(this->baseLineRect(),line_border_radius,line_border_radius);

    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        QColor color;
        if(ThemeController::themeMode() == LightTheme)
        {
            painter->setPen(ThemeController::lanhuHSLToQtHsl(0, 0, 59));
            color=ThemeController::ThemeController::lanhuHSLToQtHsl(0, 0, 85);
        }
        else
        {
            painter->setPen(ThemeController::lanhuHSLToQtHsl(0, 0, 50));
            color=ThemeController::ThemeController::lanhuHSLToQtHsl(0, 0, 17);
        }

        basePath.addRect(this->baseLineRect());

        painter->setBrush(Qt::NoBrush);
        basePath.setFillRule(Qt::FillRule::WindingFill);
        painter->drawPath(basePath.simplified());

        if(m_nodeVisible)
        {   
            painter->setBrush(color);
            for(auto point : m_nodes)
            {
                if(q->orientation() == Qt::Horizontal)
//                    basePath.addr(point.x()-1,point.y()-5,3,10);
                    painter->drawRect(point.x()-1,point.y()-5,3,10);
                else
//                    basePath.addRect(point.x()-5,point.y()-3,10,3);
                    painter->drawRect(point.x()-5,point.y()-3,10,3);
            }
        }
    }
    else
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(m_baseColor);
        basePath.addRoundedRect(this->baseLineRect(),line_border_radius,line_border_radius);
        if(m_nodeVisible)
        {
            for(auto point : m_nodes)
            {
                basePath.addEllipse(point,node_radius,node_radius);
            }
        }
        basePath.setFillRule(Qt::FillRule::WindingFill);
        painter->drawPath(basePath.simplified());
    }

}

void KSliderPrivate::drawCoverLine(QPainter*painter)
{
    Q_Q(KSlider);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_coverColor);
    painter->drawRoundedRect(this->coverLineRect(),line_border_radius,line_border_radius);
}

void KSliderPrivate::updateValuePosition()
{
    Q_Q(KSlider);
    int prePosition = m_valuePosition;
    if(q->orientation() == Qt::Horizontal)
    {
        if(q->layoutDirection() == Qt::LeftToRight)
        {
            if(q->maximum() != q->minimum())
                m_valuePosition =this->baseLineRect().left() +
                    this->baseLineRect().width()*(q->value()-q->minimum())/(q->maximum()-q->minimum());
        }
        else
        {
            if(q->maximum() != q->minimum())
            {
                m_valuePosition =this->baseLineRect().adjusted(0,0,1,0).right() -
                    this->baseLineRect().width()*(q->value()-q->minimum())/(q->maximum()-q->minimum());
            }
        }
    }
    else
    {
        if(q->maximum() != q->minimum())
            m_valuePosition =this->baseLineRect().top()+this->baseLineRect().height()
                -this->baseLineRect().height()*(q->value()-q->minimum())/(q->maximum()-q->minimum());
    }
    if(!m_animationStarted && !m_ismoving)
    {
        m_animation->stop();
        m_animation->setStartValue(prePosition);
        m_animation->setEndValue(m_valuePosition);
        m_animation->start();
        m_animationStarted = true;
        m_resize = false;
    }
}

void KSliderPrivate::updateValue()
{
    Q_Q(KSlider);
    int dur = q->maximum() - q->minimum();
    int pos,value;
    int step = q->singleStep();
    int nodeInterval =q->tickInterval();
    if(q->orientation() == Qt::Horizontal)
    {
        int left,width,right;
        if( q->layoutDirection() == Qt::LeftToRight)
        {
            if(m_currentpos.x() > baseLineRect().right())
                value = q->maximum();
            else if(m_currentpos.x() < baseLineRect().left())
                value = q->minimum();
            pos = qRound(q->minimum()+dur*((double)m_currentpos.x()-baseLineRect().left())/baseLineRect().width());
            left = baseLineRect().left();
            width = baseLineRect().width();
        }
        else
        {
            if(m_currentpos.x() > baseLineRect().right())
                value = q->minimum();
            else if(m_currentpos.x() < baseLineRect().left())
                value = q->maximum();
            pos = qRound(q->minimum()-dur*((double)m_currentpos.x()-baseLineRect().right() -1)/baseLineRect().width());
            right = baseLineRect().right();
            width = baseLineRect().width();
        }
        switch (m_sliderType)
        {
        case SingleSelectSlider:
        case SmoothSlider:
            value = pos;
            break;
        case StepSlider:
        {
            int frontIndex = (pos-q->minimum()) / step;
            int backIndex = (pos-q->minimum()) / step + 1;
            if( q->layoutDirection() == Qt::LeftToRight)
            {
                if((m_currentpos.x()-left-frontIndex*step*width/dur)
                        <(backIndex*step*width/dur - (m_currentpos.x()-left)))
                    pos = frontIndex * step + q->minimum();
                else
                    pos = backIndex * step + q->minimum();
            }
            else
            {
                if((right - m_currentpos.x()-frontIndex*step*width/dur)
                        <(backIndex*step*width/dur - (right - m_currentpos.x())))
                    pos = frontIndex * step + q->minimum();
                else
                    pos = backIndex * step + q->minimum();
            }
            value = pos;
        }
            break;
        case NodeSlider:
        {
            int frontIndex = (pos-q->minimum()) / nodeInterval;
            int backIndex = (pos-q->minimum()) / nodeInterval + 1;
            if( q->layoutDirection() == Qt::LeftToRight)
            {
                if((m_currentpos.x()-left-frontIndex*nodeInterval*width/dur)
                        <(backIndex*nodeInterval*width/dur - (m_currentpos.x()-left)))
                    pos = frontIndex * nodeInterval + q->minimum();
                else
                    pos = backIndex * nodeInterval + q->minimum();
            }
            else
            {
                if((right - m_currentpos.x()-frontIndex*nodeInterval*width/dur)
                        <(backIndex*nodeInterval*width/dur - (right - m_currentpos.x())))
                    pos = frontIndex * nodeInterval + q->minimum();
                else
                    pos = backIndex * nodeInterval + q->minimum();
            }
            value = pos;
        }
            break;
        default:
            break;
        }
    }
    else
    {
        if(m_currentpos.y() < baseLineRect().top())
            value = q->maximum();
        else if(m_currentpos.y() > baseLineRect().bottom())
            value = q->minimum();
        pos = qRound(q->minimum()+dur*((double)q->height()-m_currentpos.y())/q->height());
        int top = baseLineRect().top();
        int height = baseLineRect().height();
        switch (m_sliderType)
        {
        case SingleSelectSlider:
        case SmoothSlider:
            value = pos;
            break;
        case StepSlider:
        {
            int frontIndex = (pos-q->minimum())  / step + 1;
            int backIndex = (pos-q->minimum())  / step ;
            if((m_currentpos.y()-top - (height-frontIndex*step*height/dur))<
                    ((height-backIndex*step*height/dur) - (m_currentpos.y()-top)))
                pos = frontIndex * step + q->minimum();
            else
                pos = backIndex * step + q->minimum();
            value = pos;
        }
            break;
        case NodeSlider:
        {
            int frontIndex = (pos-q->minimum()) / nodeInterval + 1;
            int backIndex = (pos-q->minimum()) / nodeInterval ;
            if((m_currentpos.y()-top - (height-frontIndex*nodeInterval*height/dur))<
                    ((height-backIndex*nodeInterval*height/dur) - (m_currentpos.y()-top)))
                pos = frontIndex * nodeInterval + q->minimum();
            else
                pos = backIndex * nodeInterval + q->minimum();
            value = pos;
        }
            break;
        default:
            break;
        }
    }
    if(q->tickInterval() !=0)
    {
        if(!m_ismoving)
            q->setValue(value);
    }
    else
    {
        q->setValue(value);
    }

}

void KSliderPrivate::updateColor(QPalette palette)
{
    Q_Q(KSlider);
    QColor highlightColor = palette.color(QPalette::Highlight);
    QColor mix=q->palette().color(QPalette::BrightText);

    if(m_isTranslucent)
    {
        m_baseColor = palette.color(QPalette::BrightText);
        m_baseColor.setAlphaF(0.1);
    }
    else
        m_baseColor = palette.color(QPalette::Button);

    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        if(ThemeController::themeMode() == LightTheme)
        {
            if(!q->isEnabled())
            {
                m_coverColor = ThemeController::lanhuHSLToQtHsl(0, 0, 73);
                m_startColor = ThemeController::lanhuHSLToQtHsl(0,0,98);
                return;
            }
            m_coverColor = palette.highlight().color();
            if(m_isPressed)
            {
                m_startColor = ThemeController::highlightClick(false,palette);
            }
            else if(m_isHovered)
            {
                m_startColor = ThemeController::highlightHover(false,palette);
            }
            else
            {
                m_startColor = ThemeController::lanhuHSLToQtHsl(0, 0, 98);
            }
        }
        else
        {
            if(!q->isEnabled())
            {
                m_coverColor = ThemeController::lanhuHSLToQtHsl(240,2,30);
                m_startColor = ThemeController::lanhuHSLToQtHsl(0, 0, 35);
                return;
            }
            m_coverColor = palette.highlight().color();
            if(m_isPressed)
            {
                m_startColor = ThemeController::highlightClick(true,palette);
            }
            else if(m_isHovered)
            {
                m_startColor = ThemeController::highlightHover(true,palette);
            }
            else
            {
                m_startColor = ThemeController::lanhuHSLToQtHsl(0, 0, 27);
            }
        }
    }
    else
    {
        if(!q->isEnabled())
        {
            m_coverColor = palette.color(QPalette::Disabled,QPalette::ButtonText);
            m_startColor = palette.color(QPalette::Disabled,QPalette::ButtonText);
            return;
        }
        if(ThemeController::themeMode() == LightTheme)
        {
            QColor whiteColor("#FFFFFF");
            QColor darkColor("#000000");
            m_coverColor = ThemeController::mixColor(highlightColor,mix,0.2);
            if(m_isPressed)
            {
    //            m_coverColor = highlightColor.darker(120).darker(120); //取消lineRect三态
                m_startColor = highlightColor.darker(120);
                return;
            }
            else if(m_isHovered)
            {
    //            m_coverColor = highlightColor.darker(120);//取消lineRect三态
                if(ThemeController::widgetTheme() == FashionTheme)
                {
                    m_startColor = ThemeController::mixColor(highlightColor,whiteColor,0.2);
                    m_endColor = ThemeController::mixColor(highlightColor,darkColor,0.05);

                    m_linearGradient.setColorAt(0,m_startColor);
                    m_linearGradient.setColorAt(1,m_endColor);
                }
                else
                    m_startColor = highlightColor.darker(105);
                return;
            }
            else
            {
    //            m_coverColor = highlightColor.darker(120);//取消lineRect三态
                m_startColor = highlightColor;
            }
        }
        else
        {
            QColor whiteColor("#FFFFFF");
            m_coverColor = ThemeController::mixColor(highlightColor,mix,0.05);
            if(m_isPressed)
            {
    //            m_coverColor = highlightColor.lighter(120).lighter(120);//取消lineRect三态
                m_startColor = ThemeController::mixColor(highlightColor,mix,0.1);
                return;
            }
            else if(m_isHovered)
            {
    //            m_coverColor = highlightColor.lighter(120);//取消lineRect三态
                if(ThemeController::widgetTheme() == FashionTheme)
                {
                     m_startColor = ThemeController::mixColor(highlightColor,whiteColor,0.2);
                     m_endColor = highlightColor;

                     m_linearGradient.setColorAt(0,m_startColor);
                     m_linearGradient.setColorAt(1,m_endColor);
                }
                else
                    m_startColor = highlightColor.lighter(105);
                return;
            }
            else
            {
    //            m_coverColor = highlightColor.lighter(120);//取消lineRect三态
                m_startColor = highlightColor;
            }
        }
    }
}

void KSliderPrivate::changeTheme()
{
    Q_Q(KSlider);
    if(m_isTranslucentFlag)
    {
        if(ThemeController::widgetTheme() == ClassicTheme)
            m_isTranslucent = false;
        else
            m_isTranslucent = true;
    }
    else
    {
        m_isTranslucent = false;
    }
    initThemeStyle();
}

QRect KSliderPrivate::baseLineRect()
{
    Q_Q(KSlider);
    QRect rect;
        if(q->orientation() == Qt::Horizontal)
        {
            rect.setBottom(q->height()/2 + line_width/2);
            rect.setTop(q->height()/2 - line_width/2);
            rect.setLeft(spaceing +Parmscontroller::parm(Parmscontroller::Parm::PM_SliderHandleRadius)/2);
            rect.setRight(q->width() - spaceing -Parmscontroller::parm(Parmscontroller::Parm::PM_SliderHandleRadius)/2);
        }
        else
        {
            rect.setBottom(q->height() - spaceing -m_handleRadius);
            rect.setTop(spaceing);
            rect.setLeft(q->width()/2 - line_width/2);
            rect.setRight(q->width()/2 + line_width/2);
        }
    return rect;
}

QRect KSliderPrivate::coverLineRect()
{
    Q_Q(KSlider);
    QRect rect;
    if(q->orientation() == Qt::Horizontal)
    {
        if(q->layoutDirection() == Qt::LeftToRight)
        {
            rect.setBottom(q->height()/2 + line_width/2);
            rect.setTop(q->height()/2 - line_width/2);
            rect.setLeft(spaceing + Parmscontroller::parm(Parmscontroller::Parm::PM_SliderHandleRadius)/2);
            if(m_ismoving)
                rect.setRight(m_currentpos.x());
            else if(m_resize)
                rect.setRight(m_valuePosition);
            else
                rect.setRight(m_animation->currentValue().toInt());
        }
        else
        {
            rect.setBottom(q->height()/2 + line_width/2);
            rect.setTop(q->height()/2 - line_width/2);
            if(m_ismoving)
                rect.setLeft(m_currentpos.x());
            else if(m_resize)
                rect.setLeft(m_valuePosition);
            else
                rect.setLeft(m_animation->currentValue().toInt());
            rect.setRight(q->width() - spaceing -Parmscontroller::parm(Parmscontroller::Parm::PM_SliderHandleRadius)/2);
        }
    }
    else
    {
        rect.setBottom(q->height() - spaceing -m_handleRadius);
        rect.setLeft(q->width()/2 - line_width/2);
        rect.setRight(q->width()/2 + line_width/2);
        if(m_ismoving)
            rect.setTop(m_currentpos.y());
        else if(m_resize)
            rect.setTop(m_valuePosition);
        else
            rect.setTop(m_animation->currentValue().toInt());
    }
    return rect;
}

KSlider::KSlider(QWidget *parent)
    :KSlider(Qt::Horizontal,parent)
{
}

KSlider::KSlider(Qt::Orientation orientation, QWidget *parent)
    :QSlider(orientation,parent),d_ptr(new KSliderPrivate(this))
{
    Q_D(KSlider);
    d->changeTheme();
    connect(d->m_gsetting,&QGSettings::changed, d,&KSliderPrivate::changeTheme);
    connect(Parmscontroller::self(),&Parmscontroller::modeChanged,this,[=](){
        updateGeometry();
        update();
    });
    installEventFilter(this);
    setFocusPolicy(Qt::ClickFocus);
    d->m_resize = false;
}

void KSlider::setTickInterval(int interval)
{
    QSlider::setTickInterval(interval);
    update();
}

void KSlider::setSliderType(KSliderType type)
{
    Q_D(KSlider);
    d->m_sliderType = type;
    if(!tickInterval() && type == KSliderType::NodeSlider)
        setTickInterval(10);
   if(!singleStep()  && type == KSliderType::StepSlider)
       setSingleStep(10);
    if(type == KSliderType::SingleSelectSlider)
    {
        setTickInterval(1);
        setSingleStep(1);
        setRange(0,2);
    }
}

KSliderType KSlider::sliderType()
{
    Q_D(KSlider);
    return d->m_sliderType;
}

int KSlider::tickInterval() const
{
    return QSlider::tickInterval();
}

void KSlider::setValue(int value)
{
    Q_D(KSlider);
    QSlider::setValue(value);
    d->animationStart();
    update();
}

void KSlider::setNodeVisible(bool flag)
{
    Q_D(KSlider);
    if(d->m_nodeVisible != flag)
    {
        d->m_nodeVisible = flag;
        repaint();
    }
}

bool KSlider::nodeVisible()
{
    Q_D(KSlider);
    return d->m_nodeVisible;
}

void KSlider::setToolTip(const QString &str)
{
    Q_D(KSlider);
    d->m_pToolTipText = str;
}

QString KSlider::toolTip() const
{
    Q_D(const KSlider);
    return d->m_pToolTipText;
}

void KSlider::setTranslucent(bool flag)
{
     Q_D(KSlider);
    d->m_isTranslucentFlag = flag;
    d->m_isTranslucent = flag;
    d->changeTheme();
}

bool KSlider::isTranslucent()
{
     Q_D(KSlider);
    return d->m_isTranslucent;
}

void KSlider::paintEvent(QPaintEvent *event)
{
    Q_D(KSlider);
    if(orientation() == Qt::Horizontal)
    {
        for(auto point : d->m_nodes)
        {
            if(point.x()<= d->m_valuePosition)
                d->m_linearGradient=QLinearGradient(point.x(),point.y()-node_radius,point.x(),point.y()+node_radius);
        }
    }
    else
    {
        for(auto point : d->m_nodes)
        {
            if(point.y()>= d->m_valuePosition)
                d->m_linearGradient = QLinearGradient(point.x(),point.y()-node_radius,point.x(),point.y()+node_radius);
        }
    }

    d->updateColor(palette());
    d->locateNode();
    d->updateValuePosition();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setRenderHint(QPainter::SmoothPixmapTransform,true);
    p.setRenderHint(QPainter::HighQualityAntialiasing,true);
    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        if(p.pen().width() == 1)
            p.translate(0.5,0.5);
    }
    d->drawBasePath(&p);
    if(d->m_sliderType != SingleSelectSlider)
        d->drawCoverLine(&p);
    if(d->m_nodeVisible)
    {
        if(d->m_sliderType != SingleSelectSlider)
            d->drawCoverNode(&p);
    }
    d->drawSlider(&p);
}

void KSlider::resizeEvent(QResizeEvent *event)
{
    Q_D(KSlider);
    d->m_resize = true;
    QSlider::resizeEvent(event);
}

void KSlider::mousePressEvent(QMouseEvent *event)
{
    Q_D(KSlider);

    if(event->button() == Qt::LeftButton)
    {
        d->m_currentpos = event->pos();
        d->m_isPressed = true;
        d->updateColor(palette());
        d->updateValue();
        update();
    }
}

void KSlider::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(KSlider);
    if(event->button() == Qt::LeftButton)
    {
        if(orientation() == Qt::Horizontal)
        {
            if( event->pos().x() >= d->baseLineRect().left() && event->pos().x() <= d->baseLineRect().adjusted(0,0,1,0).right() )
                d->m_currentpos = event->pos();
            else if( event->pos().x() < d->baseLineRect().left() )
                d->m_currentpos.setX(d->baseLineRect().left());
            else if( event->pos().x() > d->baseLineRect().adjusted(0,0,1,0).right() )
                d->m_currentpos.setX(d->baseLineRect().adjusted(0,0,1,0).right());
        }
        else
        {
            if( event->pos().y() >= d->baseLineRect().top() && event->pos().y() <= d->baseLineRect().adjusted(0,0,0,1).bottom() )
                d->m_currentpos = event->pos();
            else if( event->pos().y() > d->baseLineRect().adjusted(0,0,0,1).bottom() )
                d->m_currentpos.setY(d->baseLineRect().adjusted(0,0,0,1).bottom());
            else if( event->pos().y() < d->baseLineRect().top() )
                d->m_currentpos.setY(d->baseLineRect().top());
        }
        if(d->m_ismoving)
        {
            if(orientation() == Qt::Horizontal)
            d->m_valuePosition =  d->m_currentpos.x();
            else
                d->m_valuePosition =  d->m_currentpos.y();
        }
        d->m_isPressed = false;
        d->m_ismoving = false;
        d->updateColor(palette());
        d->updateValue();
        update();
        d->animationStart();
    }
}

void KSlider::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(KSlider);
    if(d->m_isPressed)
    {
        if(orientation() == Qt::Horizontal)
        {
            if(event->pos().x() >= d->baseLineRect().left() &&event->pos().x() <= d->baseLineRect().adjusted(0,0,1,0).right())
                d->m_currentpos = event->pos();
            else if(event->pos().x() < d->baseLineRect().left())
                d->m_currentpos.setX(d->baseLineRect().left());
            else if(event->pos().x() > d->baseLineRect().adjusted(0,0,1,0).right())
                d->m_currentpos.setX(d->baseLineRect().adjusted(0,0,1,0).right());
        d->m_ismoving = true;
        d->updateValue();
        update();
    }
        else
        {
            if( event->pos().y() >= d->baseLineRect().top() && event->pos().y() <= d->baseLineRect().adjusted(0,0,0,1).bottom() )
                d->m_currentpos = event->pos();
            else if( event->pos().y() > d->baseLineRect().adjusted(0,0,0,1).bottom() )
                d->m_currentpos.setY(d->baseLineRect().adjusted(0,0,0,1).bottom());
            else if( event->pos().y() < d->baseLineRect().top() )
                d->m_currentpos.setY(d->baseLineRect().top());
        d->m_ismoving = true;
        d->updateValue();
        update();
    }
    }
    else {
        d->m_ismoving = false;
    }
}

void KSlider::wheelEvent(QWheelEvent *event)
{
    QSlider::wheelEvent(event);
}

bool KSlider::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KSlider);
    if(watched == this)
    {
        switch (event->type()) {
        case QEvent::Enter:
            d->m_isHovered = true;
            d->updateColor(palette());
            break;
        case QEvent::Leave:
            d->m_isHovered = false;
            d->updateColor(palette());
            break;
        case QEvent::ToolTip:
        {
            QHelpEvent* ex = static_cast<QHelpEvent*>(event);
            d->m_rect = QRect(d->m_currentpos.x()-handle_radius,d->m_currentpos.y()-handle_radius,handle_radius*2,handle_radius*2);
            if(d->m_rect.contains(ex->pos()))
            {
                QToolTip::showText(cursor().pos(),d->m_pToolTipText);
            }
            else
                return true;
            break;
        }
        case QEvent::Wheel:
        {
            d->m_forceUpdate = true;
        }
            break;
        case QEvent::KeyPress:
        {
            d->m_forceUpdate = true;
        }
            break;
        default:
            break;
        }
    }
    return QSlider::eventFilter(watched,event);
}

QSize KSlider::sizeHint() const
{
    auto size = QSlider::sizeHint();
    if(orientation() == Qt::Horizontal)
        size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_SliderHandleRadius)+2);
    else
        size.setWidth(Parmscontroller::parm(Parmscontroller::Parm::PM_SliderHandleRadius)+2);
    return size;
}

}
//#include "kslider.moc"
#include "moc_kslider.cpp"
