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

#include <QMouseEvent>
#include <QStyle>
#include <QStyleOption>
#include <QPainterPath>
#include "kswitchbutton.h"
#include "themeController.h"
#include <QDebug>
#include <QVariantAnimation>
#include <QtMath>

namespace kdk
{
class KSwitchButtonPrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KSwitchButton)

public:
    KSwitchButtonPrivate(KSwitchButton*parent)
        :q_ptr(parent),
         m_animation(new QVariantAnimation(parent))
    {
        setParent(parent);
        m_animation->setDuration(200);
        m_animation->setStartValue(0.0);
        m_animation->setEndValue(1.0);
        m_animation->setEasingCurve(QEasingCurve::Linear);
        m_animation->setLoopCount(1);
    }

protected:
    void changeTheme();

private:
    KSwitchButton* q_ptr;

    QVariantAnimation *m_animation;
    int m_space;                  //滑块离背景间隔
    int m_rectRadius;             //圆角角度
    int m_startX;                 //滑块开始X轴坐标
    int m_endX;                   //滑块结束X轴坐标
    QColor m_sliderColor;
    QColor m_bkgStartColor;
    QColor m_bkgEndColor;
    bool m_isHoverd;
    bool m_isTranslucent;
    bool m_isTranslucentFlag;
    QLinearGradient m_linearGradient;
    void updateColor(const QStyleOptionButton& opt);
    void drawBackground(QPainter *painter);
    void drawSlider(QPainter *painter);
    void stepChanged();
};


KSwitchButton::KSwitchButton(QWidget* parent)
    :QPushButton(parent),d_ptr(new KSwitchButtonPrivate(this))

{
    Q_D(KSwitchButton);
    d->m_isHoverd = false;
    d->m_isTranslucent = false;
    d->m_isTranslucentFlag = false;
    this->setCheckable(true);

    d->changeTheme();
    connect(d->m_gsetting,&QGSettings::changed, d,&KSwitchButtonPrivate::changeTheme);
    connect(d->m_animation,&QVariantAnimation::valueChanged,d,&KSwitchButtonPrivate::stepChanged);

    connect(this,&KSwitchButton::toggled,this,[=](bool flag)
    {
        d->m_endX = width()>height() ? width()-height() : 0;
        d->m_animation->start();
        Q_EMIT stateChanged(flag);
    });
    d->m_space = 4;
}

KSwitchButton::~KSwitchButton()
{

}

void KSwitchButton::setCheckable(bool flag)
{
    QPushButton::setCheckable(flag);
    this->update();
}

bool KSwitchButton::isCheckable() const
{
    return QPushButton::isCheckable();
}

bool KSwitchButton::isChecked() const
{
    return QPushButton::isChecked();
}

void KSwitchButton::setChecked(bool flag)
{
    Q_D(KSwitchButton);
    QPushButton::setChecked(flag);
    if(signalsBlocked())
    {
        d->m_animation->start();
    }
    this->update();
}

void KSwitchButton::setTranslucent(bool flag)
{
    Q_D(KSwitchButton);
    d->m_isTranslucentFlag = flag;
    d->m_isTranslucent = flag;
    d->changeTheme();
}

bool KSwitchButton::isTranslucent()
{
    Q_D(KSwitchButton);
    return d->m_isTranslucent;
}

void KSwitchButton::paintEvent(QPaintEvent *event)
{
    Q_D(KSwitchButton);

    d->m_linearGradient = QLinearGradient(this->width()/2,0,this->width()/2,this->height());
    QStyleOptionButton opt;
    initStyleOption(&opt);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    d->updateColor(opt);
    d->drawBackground(&painter);
    d->drawSlider(&painter);
    painter.drawText(0, 0, width(), height(), Qt::AlignCenter, text());
}

void KSwitchButton::resizeEvent(QResizeEvent *event)
{
    Q_D(KSwitchButton);
    QPushButton::resizeEvent(event);
    if (this->isChecked())
        d->m_startX = width()>height() ? width()-height() : 0;
    else
        d->m_startX = 0;
    d->m_rectRadius = height() / 2;
    d->m_endX = width()>height() ? width()-height() : 0;
}

QSize KSwitchButton::sizeHint() const
{
    return QSize(50,24);
}

void KSwitchButtonPrivate::changeTheme()
{
    Q_Q(KSwitchButton);
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
    q->update();
}

void KSwitchButtonPrivate::updateColor(const QStyleOptionButton& opt)
{
    Q_Q(KSwitchButton);

    //根据option状态去指定以下两个子部件的颜色
    //m_sliderColor
    //m_bkgColor
    if(m_isTranslucent && !q->isChecked())//半透明效果
    {
        m_sliderColor = QColor("#FFFFFF");
        m_bkgStartColor = opt.palette.color(QPalette::BrightText);

        if(ThemeController::themeMode() == LightTheme) //浅色模式
        {
            if(!opt.state.testFlag(QStyle::State_Enabled)) //disable
            {
                m_sliderColor = opt.palette.color(QPalette::BrightText);
                m_sliderColor.setAlphaF(0.16);
                m_bkgStartColor.setAlphaF(0.1);
                return;
            }
            m_bkgStartColor.setAlphaF(0.1); //normal
            if(opt.state.testFlag(QStyle::State_MouseOver))
            {
                if (opt.state & QStyle::State_Sunken) //clicked
                {
                    m_isHoverd = false;
                    m_bkgStartColor.setAlphaF(0.21);
                }
                else //hover
                {
                    m_isHoverd = true;
                    m_bkgStartColor.setAlphaF(0.21);
                }
            }
        }
        else
        {
            if(!opt.state.testFlag(QStyle::State_Enabled)) //disable
            {
                m_sliderColor = opt.palette.color(QPalette::BrightText);
                m_sliderColor.setAlphaF(0.2);
                m_bkgStartColor.setAlphaF(0.1);
                return;
            }
            m_bkgStartColor.setAlphaF(0.1); //normal
            if(opt.state.testFlag(QStyle::State_MouseOver))
            {
                if (opt.state & QStyle::State_Sunken) //clicked
                {
                    m_isHoverd = false;
                    m_bkgStartColor.setAlphaF(0.3);
                }
                else //hover
                {
                    m_isHoverd = true;
                    m_bkgStartColor.setAlphaF(0.2);
                }
            }
        }
    }
    else //常规效果
    {
        if(!opt.state.testFlag(QStyle::State_Enabled))
        {
            m_sliderColor = opt.palette.color(QPalette::Disabled,QPalette::ButtonText);
            m_bkgStartColor = opt.palette.color(QPalette::Disabled,QPalette::Button);
            return;
        }
        QColor mix = opt.palette.brightText().color();
        m_sliderColor = QColor("#FFFFFF");
        if(q->isChecked())
            m_bkgStartColor = opt.palette.highlight().color();
        else
        {
            if(ThemeController::widgetTheme() == ClassicTheme)
            {
                if(ThemeController::themeMode() == LightTheme)
                {
                    m_bkgStartColor=ThemeController::lanhuHSLToQtHsl(0, 0, 90);
                }
                else
                {
                    m_bkgStartColor=ThemeController::lanhuHSLToQtHsl(0, 0, 17);
                }
            }
            else
                m_bkgStartColor = opt.palette.button().color();
        }

        if(opt.state.testFlag(QStyle::State_MouseOver))
        {
            if (opt.state & QStyle::State_Sunken) //clicked
            {
                m_isHoverd = false;
                if(ThemeController::widgetTheme() == ClassicTheme &&q->isChecked() )
//                    m_bkgStartColor = ThemeController::mixColor(m_bkgStartColor,m_sliderColor,0.2);
                    if(ThemeController::themeMode() == LightTheme)
                    {
                        m_bkgStartColor = ThemeController::highlightClick(false,opt.palette);
                        m_bkgStartColor = ThemeController::adjustH(m_bkgStartColor,-2);
                        m_bkgStartColor = ThemeController::adjustS(m_bkgStartColor,-27);
                        m_bkgStartColor = ThemeController::adjustL(m_bkgStartColor,-19);
                    }
                    else
                    {
                        m_bkgStartColor = ThemeController::highlightClick(true,opt.palette);
                        m_bkgStartColor = ThemeController::adjustH(m_bkgStartColor,-2);
                        m_bkgStartColor = ThemeController::adjustS(m_bkgStartColor,-3);
                        m_bkgStartColor = ThemeController::adjustL(m_bkgStartColor,30);
                    }
                else
                    m_bkgStartColor = ThemeController::mixColor(m_bkgStartColor,mix,0.2);
            }
            else //hover
            {
                m_isHoverd = true;
                if(ThemeController::themeMode() == LightTheme) //浅色模式
                {
                    if(ThemeController::widgetTheme() == FashionTheme) //和印主题
                    {
                        if(q->isChecked())//开启
                        {
                            QColor startColor = opt.palette.color(QPalette::Highlight);
                            QColor startLightColor("#E6E6E6");
                            QColor endLightColor("#000000");
                            m_bkgStartColor = ThemeController::mixColor(startColor,startLightColor,0.2);
                            m_bkgEndColor  = ThemeController::mixColor(startColor,endLightColor,0.05);

                            m_linearGradient.setColorAt(0,m_bkgStartColor);
                            m_linearGradient.setColorAt(1,m_bkgEndColor);
                        }
                        else //关闭
                        {
                            QColor startColor("#E6E6E6");
                            QColor startLightColor("#000000 ");
                            m_bkgStartColor = ThemeController::mixColor(startColor,startLightColor,0.05);
                            m_bkgEndColor  = ThemeController::mixColor(startColor,startLightColor,0.2);

                            m_linearGradient.setColorAt(0,m_bkgStartColor);
                            m_linearGradient.setColorAt(1,m_bkgEndColor);
                        }

                    }
                    else if(ThemeController::widgetTheme() == ClassicTheme)
                    {
                        if(q->isChecked())//开启
                        {
                            m_bkgStartColor = ThemeController::highlightHover(false,opt.palette);
                            m_bkgStartColor = ThemeController::adjustH(m_bkgStartColor,-4);
                            m_bkgStartColor = ThemeController::adjustS(m_bkgStartColor,0);
                            m_bkgStartColor = ThemeController::adjustL(m_bkgStartColor,-8);
                        }
                        else
                        {
                            m_bkgStartColor = ThemeController::lanhuHSLToQtHsl(0, 0, 85);
                        }
                    }
                    else //寻光主题
                    {
                        m_bkgStartColor = ThemeController::mixColor(m_bkgStartColor,mix,0.05);
                    }

                }
                else //深色模式
                {
                    if(ThemeController::widgetTheme() == FashionTheme) //和印主题
                    {
                        if(q->isChecked())//开启
                        {
                            QColor startColor = opt.palette.color(QPalette::Highlight);
                            QColor startDarkColor("#FFFFFF");
                            m_bkgStartColor = ThemeController::mixColor(startColor,startDarkColor,0.2);
                            m_bkgEndColor  = startColor;

                            m_linearGradient.setColorAt(0,m_bkgStartColor);
                            m_linearGradient.setColorAt(1,m_bkgEndColor);
                        }
                        else //关闭
                        {
                            QColor startColor("#373737");
                            QColor startDarkColor("#FFFFFF");
                            m_bkgStartColor = ThemeController::mixColor(startColor,startDarkColor,0.2);
                            m_bkgEndColor  = ThemeController::mixColor(startColor,startDarkColor,0.05);

                            m_linearGradient.setColorAt(0,m_bkgStartColor);
                            m_linearGradient.setColorAt(1,m_bkgEndColor);
                        }

                    }
                    else if(ThemeController::widgetTheme() == ClassicTheme)
                    {
                        if(q->isChecked())//开启
                        {
                            m_bkgStartColor = ThemeController::highlightHover(true,opt.palette);
                            m_bkgStartColor = ThemeController::adjustH(m_bkgStartColor,-2);
                            m_bkgStartColor = ThemeController::adjustS(m_bkgStartColor,3);
                            m_bkgStartColor = ThemeController::adjustL(m_bkgStartColor,51);
                        }
                        else
                        {
                            m_bkgStartColor = ThemeController::lanhuHSLToQtHsl(0, 0, 23);
                        }
                    }
                    else //寻光
                    {
                        m_bkgStartColor = ThemeController::mixColor(m_bkgStartColor,mix,0.05); // deafult
                    }
                }

            }
        }
    }

}

void KSwitchButtonPrivate::drawBackground(QPainter *painter)
{
    Q_Q(KSwitchButton);
    painter->save();
    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        if(ThemeController::themeMode() == LightTheme )
            painter->setPen(ThemeController::lanhuHSLToQtHsl(0, 0, 73));
        else
            painter->setPen(ThemeController::lanhuHSLToQtHsl(0, 0, 35));
    }
    else
        painter->setPen(Qt::NoPen);
    if(ThemeController::widgetTheme() == FashionTheme)
    {
        if(m_isHoverd)
        {
            m_isHoverd = false;
            if(m_isTranslucent && !q->isChecked())
                painter->setBrush(m_bkgStartColor);
            else
                painter->setBrush(m_linearGradient);
        }
        else
            painter->setBrush(m_bkgStartColor);
    }
    else
    {
        m_isHoverd = false;
        painter->setBrush(m_bkgStartColor);
    }
    QRect rect(0, 0, q->width()-1, q->height()-1);

    //背景框两边是半圆，所以圆角的半径应该固定是矩形高度的一半
    int radius = rect.height() / 2;
    int circleWidth = rect.height();
    QPainterPath path;
    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        rect=rect.adjusted(1,1,0,0);
        path.moveTo(rect.topLeft());
        path.lineTo(rect.bottomLeft());
        path.lineTo(rect.bottomRight());
        path.lineTo(rect.topRight());
        path.lineTo(rect.topLeft());
        QPen pen;
        pen.setWidth(1);
        if(ThemeController::themeMode() == LightTheme)
            pen.setColor(ThemeController::lanhuHSLToQtHsl(0, 0, 73));
        else
            pen.setColor(ThemeController::lanhuHSLToQtHsl(0, 0, 35));
        painter->setPen(pen);
        painter->translate(0.5,0.5);
        painter->drawPath(path);
    }
    else
    {
        path.moveTo(radius, rect.top());
        path.arcTo(QRectF(rect.left(), rect.top(), circleWidth, circleWidth), 90, 180);
        path.lineTo(rect.width() - radius, rect.height());
        path.arcTo(QRectF(rect.width() - rect.height(), rect.top(), circleWidth, circleWidth), 270, 180);
        path.lineTo(radius, rect.top());
        painter->drawPath(path);
    }
    painter->restore();
}

void KSwitchButtonPrivate::drawSlider(QPainter *painter)
{
    Q_Q(KSwitchButton);
    painter->save();
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        QColor borColor;
        if(ThemeController::themeMode() == LightTheme)
        {
            m_sliderColor=ThemeController::lanhuHSLToQtHsl(0, 0, 98);
            borColor=ThemeController::lanhuHSLToQtHsl(0,0,73);
        }
        else
        {
            m_sliderColor=ThemeController::lanhuHSLToQtHsl(0, 0, 27);
            borColor=ThemeController::lanhuHSLToQtHsl(0, 0, 50);
        }
        painter->setPen(borColor);
        painter->setBrush(m_sliderColor);
        int sliderWidth = q->height() - m_space - 1 ;

        QRect sliderRect(m_startX + m_space/2, m_space/2, sliderWidth, sliderWidth);
        if(painter->pen().width() == 1)
            painter->translate(0.5,0.5);
        painter->drawRect(sliderRect);

        QIcon icon(":/image/lingmo-switchbutton-symbolic.svg") ;
        QPixmap pix = icon.pixmap(sliderWidth,sliderWidth+1);
//        pix=pix.fromImage(image);
//            qreal pixelRatio = painter->device()->devicePixelRatioF(); // 获取不同显示器的分辨率比例，防止不同分辨率下图片失真
//            pix = pix.scaled(QSize(sliderWidth * pixelRatio, sliderWidth *pixelRatio)
//                                         , Qt::KeepAspectRatio, Qt::SmoothTransformation);

        if(ThemeController::themeMode() == LightTheme)
            pix = ThemeController::drawColoredPixmap(pix,ThemeController::lanhuHSLToQtHsl(0, 0, 50));
        else
            pix = ThemeController::drawColoredPixmap(pix,ThemeController::lanhuHSLToQtHsl(0, 0, 85));

        painter->setPen(Qt::NoPen);
        painter->drawPixmap(sliderRect,pix);

        if(!q->isEnabled())
        {
            //绘制disenable状态下的小矩形条
            if(!q->isChecked())
            {
                int topX = q->width()-sliderWidth/2-10;
                int topY = (q->height()-sliderWidth/4)/2;
                QRect lineRect(topX,topY,sliderWidth/2,sliderWidth/4);
                if(painter->pen().width() == 1)
                    painter->translate(0.5,0.5);
                painter->drawRect(lineRect);
            }
            else
            {
                int topX = 10;
                int topY = (q->height()-sliderWidth/4)/2;
                QRect lineRect(topX,topY,sliderWidth/2,sliderWidth/4);
                if(painter->pen().width() == 1)
                    painter->translate(0.5,0.5);
                painter->drawRect(lineRect);
            }
        }
    }
    else
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(m_sliderColor);
        //滑块的直径
        int sliderWidth = q->height() - m_space * 2;
        QRect sliderRect(m_startX + m_space, m_space, sliderWidth, sliderWidth);
        painter->drawEllipse(sliderRect);
        if(!q->isEnabled())
        {
            //绘制disenable状态下的小矩形条
            if(!q->isChecked())
            {
                int topX = q->width()-sliderWidth/2-10;
                int topY = (q->height()-sliderWidth/4)/2;
                QRect lineRect(topX,topY,sliderWidth/2,sliderWidth/4);
                int radius = lineRect.height()/2;
                painter->drawRoundedRect(lineRect,radius,radius);
            }
            else
            {
                int topX = 10;
                int topY = (q->height()-sliderWidth/4)/2;
                QRect lineRect(topX,topY,sliderWidth/2,sliderWidth/4);
                int radius = lineRect.height()/2;
                painter->drawRoundedRect(lineRect,radius,radius);
            }
        }
    }
    painter->restore();
}

void KSwitchButtonPrivate::stepChanged()
{
    Q_Q(KSwitchButton);
    if(q->isChecked())
    {
        if(m_startX < m_endX)
        {
            m_startX =qCeil(m_endX * m_animation->currentValue().toDouble());
        }
        else
        {
            m_startX = m_endX;
            m_animation->stop();
        }
    }
    else
    {
        if (m_startX > 0)
        {
            double percent = 1.0 - m_animation->currentValue().toDouble();
            m_startX = qCeil(m_endX * percent);
        }
        else
        {
            m_startX = 0;
            m_animation->stop();
        }
    }
    q->update();
}

}
#include "kswitchbutton.moc"
#include "moc_kswitchbutton.cpp"

