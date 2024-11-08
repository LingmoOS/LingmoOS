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

#include "kcolorbutton.h"
#include <QPainter>
#include <QStyleOptionButton>
#include "themeController.h"
#include <QDebug>

namespace kdk {

static const int circleButtonSize = 20;
static const int normalButtonSize = 24;

class KColorButtonPrivate : public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KColorButton)
public:
    KColorButtonPrivate(KColorButton* parent);

protected:
    void changeTheme();

private:
    KColorButton* const q_ptr;
    bool m_backgroundFlag;
    QColor m_backgroundColor;
    int m_radius;
    KColorButton::ButtonType m_buttonType;
};

KColorButton::KColorButton(QWidget *parent)
    :QPushButton(parent)
    ,d_ptr(new KColorButtonPrivate(this))
{
    Q_D(KColorButton);
    setCheckable(true);
}

void KColorButton::setBackgroundColor(QColor color)
{
    Q_D(KColorButton);
    d->m_backgroundFlag = true;
    d->m_backgroundColor = color;
    update();
}

QColor KColorButton::backgroundColor()
{
    Q_D(KColorButton);
    return d->m_backgroundColor;
}

void KColorButton::setBorderRadius(int radious)
{
    Q_D(KColorButton);
    d->m_radius = radious;
}

void KColorButton::setButtonType(KColorButton::ButtonType type)
{
    Q_D(KColorButton);
    d->m_buttonType = type;
}

void KColorButton::paintEvent(QPaintEvent *)
{
    Q_D(KColorButton);
    QStyleOptionButton option;
    initStyleOption(&option);

    QRect rect = option.rect;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);

    if(!option.state.testFlag(QStyle::State_Enabled))
    {
        painter.save();
        painter.setBrush(option.palette.color(QPalette::Disabled,QPalette::ButtonText));
        painter.drawRoundedRect(rect,d->m_radius,d->m_radius);
        painter.restore();
    }
    else if(d->m_buttonType == RoundedRect)
    {
        painter.save();
        if(d->m_backgroundFlag)
        {
            painter.setBrush(d->m_backgroundColor);
        }
        else
        {
            painter.setBrush(palette().highlight().color());
        }
        if(isChecked() || option.state.testFlag(QStyle::State_MouseOver))
        {
            painter.save();
            painter.setBrush(Qt::white);
            painter.drawRoundedRect(rect,d->m_radius,d->m_radius);//绘制边框
            rect = rect.adjusted(2,2,-2,-2);
            painter.restore();

            painter.drawRoundedRect(rect,d->m_radius/2,d->m_radius/2);
        }
        else
            painter.drawRoundedRect(rect,d->m_radius,d->m_radius);
        painter.restore();
    }
    else if(d->m_buttonType == Circle)
    {
        painter.save();
        QRect rect = option.rect.adjusted(1,1,0,0);
        if(d->m_backgroundFlag)
        {
            painter.setBrush(d->m_backgroundColor);
        }
        else
        {
            painter.setBrush(palette().highlight().color());
        }

        painter.drawEllipse(rect);

        QRect rect1 = rect.adjusted(rect.width()/4,rect.height()/4,-rect.width()/4,-rect.height()/4);
        if(option.state.testFlag(QStyle::State_MouseOver) || isChecked())
        {
            painter.save();
            painter.setBrush(Qt::white);
            painter.drawEllipse(rect1);
            painter.restore();
        }
        painter.restore();
    }
    else
    {
        painter.save();
        if(d->m_backgroundFlag)
        {
            painter.setBrush(d->m_backgroundColor);
        }
        else
        {
            painter.setBrush(palette().highlight().color());
        }
        if(isChecked())
        {
            QPointF points[3] = {
                        QPointF(rect.x() + rect.width() * 2 / 11, rect.y() + rect.height() * 6 / 11),
                        QPointF(rect.x() + rect.width() * 5 / 11, rect.y() + rect.height() * 8 / 11),
                        QPointF(rect.x() + rect.width() * 9 / 11, rect.y() + rect.height() * 4 / 11),
                    };
        painter.drawRoundedRect(rect,d->m_radius,d->m_radius);
        painter.setPen(QPen(Qt::white,2));
        painter.drawPolyline(points, 3);
        }
        else
        {
            painter.drawRoundedRect(rect,d->m_radius,d->m_radius);
        }
        painter.restore();
    }
}

QSize KColorButton::sizeHint() const
{
    Q_D(const KColorButton);
    if(d->m_buttonType == Circle)
        return QSize(circleButtonSize,circleButtonSize);
    else
        return QSize(normalButtonSize,normalButtonSize);
}

KColorButtonPrivate::KColorButtonPrivate(KColorButton *parent)
    :q_ptr(parent)
{
    Q_Q(KColorButton);
    m_backgroundFlag = false;
    m_radius =6;
    m_buttonType = KColorButton::RoundedRect;
    m_backgroundColor = q->palette().highlight().color();
    connect(m_gsetting,&QGSettings::changed,this,[=](){
        changeTheme();
        q->update();
    });
}

void KColorButtonPrivate::changeTheme()
{
    Q_Q(KColorButton);
    initThemeStyle();
}


}

#include "kcolorbutton.moc"
#include "moc_kcolorbutton.cpp"
