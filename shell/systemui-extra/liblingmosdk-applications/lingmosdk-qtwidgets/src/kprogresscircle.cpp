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

#include "kprogresscircle.h"
#include <QLocale>
#include <QPainter>
#include <QIcon>
#include <QDebug>
#include "themeController.h"

namespace kdk
{
class KProgressCirclePrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KProgressCircle)

public:
    KProgressCirclePrivate(KProgressCircle*parent);
    void initDefaultFormat();
    void drawBaseCircle(QPainter*painter);
    void dawColoredCircle(QPainter*painter);
    void drawText(QPainter*painter);
    void calculateCircleRect();
    void changeTheme();

private:
    KProgressCircle *q_ptr;
    int m_minimum;
    int m_maximum;
    int m_value;
    QString m_format;
    QString m_text;
    bool m_defaultFormat;
    bool m_isTextVisiable;
    ProgressBarState m_state;

    QColor m_color;
    QColor m_baseColor;
    QColor m_baseCircleColor;
    QRect m_baseRect;
    QRect m_contentRect;
    int m_cirleWidth;
    int m_startAngel;
    int m_pixWidth;
    int m_textWidth;
};

KProgressCircle::KProgressCircle(QWidget *parent)
    : QWidget(parent),
      d_ptr(new KProgressCirclePrivate(this))
{
    setFixedSize(60,60);
    connect(this,&KProgressCircle::valueChanged,this,[=](){
        if(this->value()==this->maximum())
            setState(SuccessProgress);
    });
}

int KProgressCircle::minimum() const
{
    Q_D(const KProgressCircle);
    return d->m_minimum;
}

int KProgressCircle::maximum() const
{
    Q_D(const KProgressCircle);
    return d->m_maximum;
}

int KProgressCircle::value() const
{
    Q_D(const KProgressCircle);
    return d->m_value;
}

QString KProgressCircle::text() const
{
    Q_D(const KProgressCircle);
    if ((d->m_maximum == 0 && d->m_minimum == 0) || d->m_value < d->m_minimum
            || (d->m_value == INT_MIN && d->m_minimum == INT_MIN))
        return QString();


    qint64 totalSteps = qint64(d->m_maximum) - d->m_minimum;

    QString result = d->m_format;
    QLocale locale = QWidget::locale();
    locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);
    result.replace(QLatin1String("%m"), QString::number(totalSteps));
    result.replace(QLatin1String("%v"), QString::number(d->m_value));

    if (totalSteps == 0) {
        result.replace(QLatin1String("%p"), locale.toString(100));
        return result;
    }

    const auto progress = static_cast<int>((qint64(d->m_value) - d->m_minimum) * 100.0 / totalSteps);
    result.replace(QLatin1String("%p"), QString::number(progress));
    return result;
}

void KProgressCircle::setTextVisible(bool visible)
{
    Q_D(KProgressCircle);
    d->m_isTextVisiable = visible;
    update();
}

bool KProgressCircle::isTextVisible() const
{
    Q_D(const KProgressCircle);
    return d->m_isTextVisiable;
}

ProgressBarState KProgressCircle::state()
{
    Q_D(const KProgressCircle);
    return d->m_state;
}

void KProgressCircle::setState(ProgressBarState state)
{
    Q_D(KProgressCircle);
    d->m_state = state;
    switch (d->m_state)
    {
    case FailedProgress:
        d->m_color = "#FF4D4F";
        break;
    case SuccessProgress:
        d->m_color = "#52C429";
        break;
    case NormalProgress:
    default:
        d->m_color = "#3790FA";
        break;
    }
    update();
}

void KProgressCircle::reset()
{
    Q_D(KProgressCircle);
    if (d->m_minimum == INT_MIN)
        d->m_value = INT_MIN;
    else
        d->m_value = d->m_minimum - 1;
    repaint();
}

void KProgressCircle::setRange(int minimum, int maximum)
{
    Q_D(KProgressCircle);
    if (minimum != d->m_minimum || maximum != d->m_maximum) {
        d->m_minimum = minimum;
        d->m_maximum = qMax(minimum, maximum);

        if (d->m_value < qint64(d->m_minimum) - 1 || d->m_value > d->m_maximum)
            reset();
        else
            update();
    }
}

void KProgressCircle::setMinimum(int minimum)
{
    Q_D(KProgressCircle);
    setRange(minimum, qMax(d->m_maximum, minimum));
}

void KProgressCircle::setMaximum(int maximum)
{
    Q_D(KProgressCircle);
    setRange(qMin(d->m_minimum, maximum), maximum);
}

void KProgressCircle::setValue(int value)
{
    Q_D(KProgressCircle);
    if (d->m_value == value
            || ((value > d->m_maximum || value < d->m_minimum)
                && (d->m_maximum != 0 || d->m_minimum != 0)))
        return;
    d->m_value = value;
    Q_EMIT valueChanged(value);
    repaint();
}

void KProgressCircle::paintEvent(QPaintEvent *)
{
    Q_D(KProgressCircle);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    if(painter.pen().width() == 1)
        painter.translate(0.5,0.5);
    d->calculateCircleRect();
    d->drawBaseCircle(&painter);
    d->dawColoredCircle(&painter);
    d->drawText(&painter);
}

KProgressCirclePrivate::KProgressCirclePrivate(KProgressCircle *parent)
    :q_ptr(parent),
      m_minimum(0),
      m_maximum(100),
      m_value(0),
      m_cirleWidth(6),
      m_isTextVisiable(true),
      m_startAngel(90),
      m_pixWidth(20),
      m_textWidth(30),
      m_state(NormalProgress),
      m_color("#3790FA"),
      m_defaultFormat(true)
{
    initDefaultFormat();
    changeTheme();
    connect(m_gsetting,&QGSettings::changed,this,&KProgressCirclePrivate::changeTheme);

}

void KProgressCirclePrivate::initDefaultFormat()
{
    Q_Q(KProgressCircle);
    if (m_defaultFormat)
    {
        QLocale locale = q->locale();
        m_format = QLatin1String("%p") + locale.percent();
    }

}

void KProgressCirclePrivate::drawBaseCircle(QPainter *painter)
{
    Q_Q(KProgressCircle);
    QPen pen;
    if(m_isTextVisiable)
    {
        if(ThemeController::widgetTheme() == ClassicTheme)
        {
            if(ThemeController::themeMode() == LightTheme)
                pen.setColor(ThemeController::lanhuHSLToQtHsl(0, 0, 73));
            else
                pen.setColor(ThemeController::lanhuHSLToQtHsl(0, 0, 35));
            pen.setWidth(8);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->setRenderHint(QPainter::Antialiasing);
            painter->drawEllipse(m_baseRect.adjusted(1,1,-1,-1));
        }
        pen.setColor(m_baseColor);
        pen.setWidth(m_cirleWidth);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawEllipse(m_baseRect.adjusted(1,1,-1,-1));
    }
    else
    {
        QPen penColor;
        QBrush outBrush,inBrush;
        if(ThemeController::widgetTheme() == ClassicTheme)
        {
            if(ThemeController::themeMode() == LightTheme)
            {
                outBrush=QBrush(ThemeController::lanhuHSLToQtHsl(0, 0, 98));
                inBrush=QBrush(ThemeController::lanhuHSLToQtHsl(0, 0, 98));
                penColor.setColor(ThemeController::lanhuHSLToQtHsl(0, 0, 73));
            }
            else
            {
                outBrush=QBrush(ThemeController::lanhuHSLToQtHsl(0, 0, 27));
                inBrush=QBrush(ThemeController::lanhuHSLToQtHsl(0, 0, 27));
                penColor.setColor(ThemeController::lanhuHSLToQtHsl(0, 0, 35));
            }
        }
        else
        {
            outBrush = m_baseColor;
            inBrush=QBrush(Qt::NoBrush);
            penColor=QPen(Qt::NoPen);
        }

        painter->save();
        painter->setPen(penColor);
        painter->setBrush(outBrush);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawEllipse(m_baseRect);
        painter->restore();
        pen.setColor(m_baseCircleColor);
        pen.setWidth(m_cirleWidth);
        painter->setPen(pen);
        painter->setBrush(inBrush);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawEllipse(m_contentRect);
    }
}

void KProgressCirclePrivate::dawColoredCircle(QPainter *painter)
{
    Q_Q(KProgressCircle);
    QPen pen;
    int angleSpan = m_value*360/m_maximum;

    if(m_state == ProgressBarState::NormalProgress){
        m_color = q->palette().color(QPalette::Highlight);
    }

    if(m_isTextVisiable)
    {
        pen.setColor(m_color);
        pen.setWidth(m_cirleWidth);
        if(ThemeController::widgetTheme() == ClassicTheme)
            pen.setCapStyle(Qt::SquareCap);
        else
            pen.setCapStyle(Qt::RoundCap);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->setRenderHint(QPainter::Antialiasing);
        if(ThemeController::widgetTheme() == ClassicTheme)
            painter->drawArc(m_baseRect.adjusted(1,1,-1,-1),m_startAngel*16,-angleSpan*16);
        else
            painter->drawArc(m_baseRect,m_startAngel*16,-angleSpan*16);
    }
    else
    {
        pen.setColor(m_color);
        pen.setWidth(m_cirleWidth);
        if(ThemeController::widgetTheme() == ClassicTheme)
            pen.setCapStyle(Qt::SquareCap);
        else
            pen.setCapStyle(Qt::RoundCap);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawArc(m_contentRect,m_startAngel*16,-angleSpan*16);
    }
}

void KProgressCirclePrivate::drawText(QPainter *painter)
{
    Q_Q(KProgressCircle);
    QRect pixRect;
    pixRect.setLeft(q->rect().center().x()-m_pixWidth/2);
    pixRect.setTop(q->rect().center().y()-m_pixWidth/2);
    pixRect.setRight(q->rect().center().x()+m_pixWidth/2);
    pixRect.setBottom(q->rect().center().y()+m_pixWidth/2);
    QRect textRect;
    textRect.setLeft(q->rect().center().x()-m_textWidth/2);
    textRect.setTop(q->rect().center().y()-m_textWidth/2);
    textRect.setRight(q->rect().center().x()+m_textWidth/2);
    textRect.setBottom(q->rect().center().y()+m_textWidth/2);
    if(m_isTextVisiable)
    {
        switch (m_state)
        {
        case NormalProgress:
        {
            painter->setRenderHint(QPainter::Antialiasing);
            painter->drawText(textRect,Qt::AlignCenter,q->text());
        }
            break;
        case SuccessProgress:
        {
            QColor green;
            if(ThemeController::widgetTheme() == FashionTheme)
            {
                if(ThemeController::themeMode() == LightTheme)
                    green = QColor("#65E944");
                else
                    green = QColor("#52C429");
            }
            else
                 green = QColor("#52C429");
            QPixmap pixmap = ThemeController::drawColoredPixmap(QIcon::fromTheme("object-select-symbolic").pixmap(m_pixWidth,m_pixWidth),green);
            painter->setRenderHint(QPainter::Antialiasing);
            painter->drawPixmap(pixRect,pixmap);
            break;
        }
        case FailedProgress:
        {
            QColor red;
            if(ThemeController::widgetTheme() == FashionTheme)
            {
                if(ThemeController::themeMode() == LightTheme)
                    red = QColor("#EC334C");
                else
                    red = QColor("#FF4D4F");
            }
            else
                red = QColor("#FF4D4F");
            QPixmap pixmap = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(20,20),red);
            painter->setRenderHint(QPainter::Antialiasing);
            painter->drawPixmap(pixRect,pixmap);
            break;
        }
        default:
            break;
        }
    }
}

void KProgressCirclePrivate::calculateCircleRect()
{
    Q_Q(KProgressCircle);

    m_baseRect.setTop(1+m_cirleWidth/2);
    m_baseRect.setLeft(1+m_cirleWidth/2);
    m_baseRect.setBottom(q->rect().height()-1-m_cirleWidth/2);
    m_baseRect.setRight(q->rect().right()-1-m_cirleWidth/2);

    m_contentRect.setTop(1+m_cirleWidth*2);
    m_contentRect.setLeft(1+m_cirleWidth*2);
    m_contentRect.setBottom(q->rect().height()-1-m_cirleWidth*2);
    m_contentRect.setRight(q->rect().right()-1-m_cirleWidth*2);
}

void KProgressCirclePrivate::changeTheme()
{
    Q_Q(KProgressCircle);
    initThemeStyle();
    if(ThemeController::themeMode() == LightTheme)
    {
        m_baseColor = QColor("#E6E6E6");
        m_baseCircleColor = QColor("#BFBFBF");
    }
    else
    {
        m_baseColor = QColor("#37373B");
        m_baseCircleColor = QColor("#232426");
    }
    q->update();
}

}
#include "kprogresscircle.moc"
#include "moc_kprogresscircle.cpp"
