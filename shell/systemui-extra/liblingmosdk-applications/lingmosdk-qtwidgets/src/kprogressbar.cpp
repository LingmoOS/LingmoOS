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

#include "kprogressbar.h"
#include "themeController.h"
#include <QStyleOptionProgressBar>
#include <QPainter>
#include <QRect>
#include <QFontMetrics>
#include <QApplication>
#include <QDebug>

namespace  kdk
{
class KProgressBarPrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KProgressBar)

public:
    KProgressBarPrivate(KProgressBar*parent);
    void calculateTextRect();
    void calculateRect();
    void calculateContenteRect();
    void changeTheme();


Q_SIGNALS:
    void progressStateChanged();

private:
    KProgressBar* q_ptr;
    ProgressBarState m_state;
    int m_contentMargin;
    QRect m_textRect;
    QRect m_contentRect;
    QRect m_rect;
    int m_bodyWidth;
};

KProgressBar::KProgressBar(QWidget *parent)
    :QProgressBar(parent),
      d_ptr(new KProgressBarPrivate(this))
{
    Q_D(KProgressBar);
    d->changeTheme();
    connect(d->m_gsetting,&QGSettings::changed,d,&KProgressBarPrivate::changeTheme);
    connect(this,&KProgressBar::valueChanged,this,[=](){
        if(this->value()==this->maximum())
            setState(SuccessProgress);
    });
    setContentsMargins(6,6,6,6);
    setValue(0);
}

ProgressBarState KProgressBar::state()
{
    Q_D(KProgressBar);
    return d->m_state;
}

void KProgressBar::setState(ProgressBarState state)
{
    Q_D(KProgressBar);
    d->m_state = state;
    update();
}

void KProgressBar::paintEvent(QPaintEvent *event)
{
    Q_D(KProgressBar);
    d->calculateTextRect();
    d->calculateRect();
    d->calculateContenteRect();
    //以上三个函数有逻辑关系，相对位置不能改变

    QPainter painter(this);
    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        if(painter.pen().width() == 1)
            painter.translate(0.5,0.5);
    }
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    if(this->orientation() == Qt::Horizontal)
    {
        if(ThemeController::widgetTheme() == ClassicTheme)
        {
            if(ThemeController::themeMode() == LightTheme)
            {
                painter.setBrush(QColor(250,250,250));
                painter.setPen(QColor(187,187,187));
            }
            else
            {
                painter.setBrush(QColor(68, 68, 68));
                painter.setPen(QColor(89,89,89));
            }
            painter.drawRoundedRect(d->m_rect.adjusted(0,0,0,-1),0,0);
        }
        else
        {
            if(ThemeController::themeMode() == LightTheme)
                painter.setBrush(QColor(230,230,230));
            else
                painter.setBrush(QColor(55,55,59));
            painter.drawRoundedRect(d->m_rect,6,6);
        }
        QLinearGradient linear(this->rect().topLeft(), this->rect().bottomRight());
        QColor color = palette().color(QPalette::Highlight);
        switch (d->m_state)
        {
        case NormalProgress:
            if(ThemeController::widgetTheme() == FashionTheme)
            {
                QColor startColor = ThemeController::mixColor(color,Qt::white,0.2);
                QColor endColor = ThemeController::mixColor(color,Qt::white,0.05);
                linear.setColorAt(0, startColor);
                linear.setColorAt(1, endColor);
            }
            else if(ThemeController::widgetTheme() == ClassicTheme)
            {
                QColor startColor = ThemeController::highlightClick(false,palette());
                startColor = ThemeController::adjustH(startColor,-6);
                startColor = ThemeController::adjustS(startColor,-13);
                startColor = ThemeController::adjustL(startColor,-33);
                linear.setColorAt(0, startColor);
                linear.setColorAt(1, startColor);
            }
            else
            {
                QColor startColor = ThemeController::mixColor(color,Qt::white,0.2);
                QColor endColor = ThemeController::mixColor(color,Qt::white,0.05);
                linear.setColorAt(0, startColor);
                linear.setColorAt(1, endColor);
            }
            linear.setSpread(QGradient::PadSpread);
            painter.setBrush(linear);
            if(ThemeController::widgetTheme() == ClassicTheme)
                painter.drawRoundedRect(d->m_contentRect,0,0);
            else
                painter.drawRoundedRect(d->m_contentRect,6,6);
            if(isTextVisible())
            {
                painter.setPen(QWidget::palette().color(QPalette::Text));
                painter.drawText(d->m_textRect,Qt::AlignCenter,text());
            }
            break;
        case FailedProgress:
        {
            if(ThemeController::widgetTheme() == ClassicTheme)
            {
                linear.setColorAt(0, ThemeController::lanhuHSLToQtHsl(359, 100, 65));
                linear.setColorAt(1, ThemeController::lanhuHSLToQtHsl(359, 100, 65));
            }
            else
            {
                linear.setColorAt(0, QColor(255,77,79));
                linear.setColorAt(1, QColor(243,34,45));
            }
            linear.setSpread(QGradient::PadSpread);
            painter.setBrush(linear);
            if(ThemeController::widgetTheme() == ClassicTheme)
                painter.drawRoundedRect(d->m_contentRect,0,0);
            else
                painter.drawRoundedRect(d->m_contentRect,6,6);
            if(isTextVisible())
            {
                QPixmap pixmap = QIcon::fromTheme("dialog-error").pixmap(16,16);
                QRect rect(0,0,16,16);
                rect.moveCenter(d->m_textRect.center());
                painter.drawPixmap(rect,pixmap);
            }
            break;
        }
        case SuccessProgress:
        {
            if(ThemeController::widgetTheme() == FashionTheme)
            {
               if(ThemeController::themeMode() == LightTheme)
               {
                   linear.setColorAt(0, QColor("#8DF063"));
                   linear.setColorAt(1, QColor("#4ED42D"));
               }
               else
               {
                   linear.setColorAt(0, QColor("#75D14D"));
                   linear.setColorAt(1, QColor("#52C429"));
               }
            }
            else if(ThemeController::widgetTheme() == ClassicTheme)
            {
                linear.setColorAt(0, ThemeController::lanhuHSLToQtHsl(104, 65, 46));
                linear.setColorAt(1, ThemeController::lanhuHSLToQtHsl(104, 65, 46));
            }
            else
            {
                linear.setColorAt(0, QColor(117,209,77));
                linear.setColorAt(1, QColor(82,196,41));
            }
            linear.setSpread(QGradient::PadSpread);
            painter.setBrush(linear);
            if(ThemeController::widgetTheme() == ClassicTheme)
                painter.drawRoundedRect(d->m_contentRect,0,0);
            else
                painter.drawRoundedRect(d->m_contentRect,6,6);
            if(isTextVisible())
            {
                QPixmap pixmap = QIcon::fromTheme("lingmo-dialog-success").pixmap(16,16);
                QRect rect(0,0,16,16);
                rect.moveCenter(d->m_textRect.center());
                painter.drawPixmap(rect,pixmap);
            }
            break;
        }
        default:
            break;
        }
    }
    else
    {
        if(ThemeController::widgetTheme() == ClassicTheme)
        {
            if(ThemeController::themeMode() == LightTheme)
            {
                painter.setBrush(QColor(250,250,250));
                painter.setPen(QColor(187,187,187));
            }
            else
            {
                painter.setBrush(QColor(68, 68, 68));
                painter.setPen(QColor(89,89,89));
            }
            painter.drawRoundedRect(d->m_rect.adjusted(0,0,-1,0),0,0);
        }
        else
        {
            if(ThemeController::themeMode() == LightTheme)
                painter.setBrush(QColor(230,230,230));
            else
                painter.setBrush(QColor(55,55,59));
            painter.drawRoundedRect(d->m_rect,6,6);
        }
        QLinearGradient linear(this->rect().topLeft(), this->rect().bottomRight());
        QColor color = palette().color(QPalette::Highlight);
        switch (d->m_state)
        {
        case NormalProgress:
            if(ThemeController::widgetTheme() == FashionTheme)
            {
                QColor startColor = ThemeController::mixColor(color,Qt::white,0.2);
                QColor endColor = ThemeController::mixColor(color,Qt::white,0.05);
                linear.setColorAt(0, startColor);
                linear.setColorAt(1, endColor);
            }
            else if(ThemeController::widgetTheme() == ClassicTheme)
            {
                QColor startColor = ThemeController::highlightClick(false,palette());
                startColor = ThemeController::adjustH(startColor,-6);
                startColor = ThemeController::adjustS(startColor,-13);
                startColor = ThemeController::adjustL(startColor,-33);
                linear.setColorAt(0, startColor);
                linear.setColorAt(1, startColor);
            }
            else
            {
                QColor startColor = ThemeController::mixColor(color,Qt::white,0.2);
                QColor endColor = ThemeController::mixColor(color,Qt::white,0.05);
                linear.setColorAt(0, startColor);
                linear.setColorAt(1, endColor);
            }
            linear.setSpread(QGradient::PadSpread);
            painter.setBrush(linear);
            if(ThemeController::widgetTheme() == ClassicTheme)
                painter.drawRoundedRect(d->m_contentRect,0,0);
            else
                painter.drawRoundedRect(d->m_contentRect,6,6);
            if(isTextVisible())
            {
                painter.setPen(QWidget::palette().color(QPalette::Text));
                painter.drawText(d->m_textRect,Qt::AlignCenter,text());
            }
            break;
        case FailedProgress:
        {
            if(ThemeController::widgetTheme() == ClassicTheme)
            {
                linear.setColorAt(0, ThemeController::lanhuHSLToQtHsl(359, 100, 65));
                linear.setColorAt(1, ThemeController::lanhuHSLToQtHsl(359, 100, 65));
            }
            else
            {
                linear.setColorAt(0, QColor(255,77,79));
                linear.setColorAt(1, QColor(243,34,45));
            }
            linear.setSpread(QGradient::PadSpread);
            painter.setBrush(linear);
            if(ThemeController::widgetTheme() == ClassicTheme)
                painter.drawRoundedRect(d->m_contentRect,0,0);
            else
                painter.drawRoundedRect(d->m_contentRect,6,6);
            if(isTextVisible())
            {
                QPixmap pixmap = QIcon::fromTheme("dialog-error").pixmap(16,16);
                QRect rect(0,0,16,16);
                rect.moveCenter(d->m_textRect.center());
                painter.drawPixmap(rect,pixmap);
            }
            break;
        }
        case SuccessProgress:
        {
            if(ThemeController::widgetTheme()== FashionTheme)
            {
               if(ThemeController::themeMode() == LightTheme)
               {
                   linear.setColorAt(0, QColor("#8DF063"));
                   linear.setColorAt(1, QColor("#4ED42D"));
               }
               else
               {
                   linear.setColorAt(0, QColor("#75D14D"));
                   linear.setColorAt(1, QColor("#52C429"));
               }
            }
            else if(ThemeController::widgetTheme() == ClassicTheme)
            {
                linear.setColorAt(0, ThemeController::lanhuHSLToQtHsl(104, 65, 46));
                linear.setColorAt(1, ThemeController::lanhuHSLToQtHsl(104, 65, 46));
            }
            else
            {
                linear.setColorAt(0, QColor(117,209,77));
                linear.setColorAt(1, QColor(82,196,41));
            }
            linear.setSpread(QGradient::PadSpread);
            painter.setBrush(linear);
            if(ThemeController::widgetTheme() == ClassicTheme)
                painter.drawRoundedRect(d->m_contentRect,0,0);
            else
                painter.drawRoundedRect(d->m_contentRect,6,6);
            if(isTextVisible())
            {
                QPixmap pixmap = QIcon::fromTheme("lingmo-dialog-success").pixmap(16,16);
                //painter.drawPixmap(d->m_textRect,pixmap);
                QRect rect(0,0,16,16);
                rect.moveCenter(d->m_textRect.center());
                painter.drawPixmap(rect,pixmap);
            }
            break;
        }
        default:
            break;
        }
    }
}

QSize KProgressBar::sizeHint() const
{
    QSize size = QProgressBar::sizeHint();
    if(this->orientation()==Qt::Horizontal)
        size.setHeight(30);
    else
        size.setWidth(30);
    return size;
}


QString KProgressBar::text() const
{
    Q_D(const KProgressBar);
    if ((maximum() == 0 && minimum() == 0) || value() < minimum()
            || (minimum() == INT_MIN && minimum() == INT_MIN))
        return QString();
    qint64 totalSteps = qint64(maximum()) - minimum();

    QString result = format();
    QLocale locale = this->locale(); // Omit group separators for compatibility with previous versions that were non-localized.
    locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);
    result.replace(QLatin1String("%m"), locale.toString(totalSteps));
    result.replace(QLatin1String("%v"), locale.toString(value()));

    // If max and min are equal and we get this far, it means that the
    // progress bar has one step and that we are on that step. Return
    // 100% here in order to avoid division by zero further down.
    if (totalSteps == 0) {
        result.replace(QLatin1String("%p"), locale.toString(100));
        return result;
    }

    const auto progress = static_cast<int>((qint64(value()) - minimum()) * 100.0 / totalSteps);
    result.replace(QLatin1String("%p"), locale.toString(progress));
    return result;
}

void KProgressBar::setOrientation(Qt::Orientation orientation)
{
    if(orientation == Qt::Vertical)
        this->setMinimumHeight(200);
    QProgressBar::setOrientation(orientation);
}

void KProgressBar::setBodyWidth(int width)
{
    Q_D(KProgressBar);
    d->m_bodyWidth = width;
    update();
}


KProgressBarPrivate::KProgressBarPrivate(KProgressBar *parent)
    :q_ptr(parent)
{
    m_contentMargin = 2;
    m_state = NormalProgress;
    setParent(parent);
    m_bodyWidth = 0;
}

void KProgressBarPrivate::calculateTextRect()
{
    Q_Q(KProgressBar);
    if(!q->isTextVisible())
        m_textRect =  QRect();
    else
    {
        QFont font(QApplication::font());
        QFontMetrics fm(font);
        m_textRect = QRect(0,0,fm.width(q->text()),fm.height());
        m_textRect.moveCenter(q->rect().center());
        if(q->orientation()==Qt::Horizontal)
        {
            if(q->alignment() & Qt::AlignCenter)
                return;
            else
            {
                if(ThemeController::systemLang())
                    m_textRect.moveLeft(q->rect().left());
                else
                    m_textRect.moveRight(q->rect().right());
            }
        }
        else
        {
            if(q->alignment() & Qt::AlignCenter)
                return;
            else
                m_textRect.moveTop(q->rect().top());
        }
    }
}

void KProgressBarPrivate::calculateRect() //背景矩形
{
    Q_Q(KProgressBar);
    QMargins margins  = q->contentsMargins();
    m_rect = q->rect();
    if(q->orientation() == Qt::Horizontal)
    {
        if(m_bodyWidth != 0)
            m_rect.setHeight(m_bodyWidth);
        if(!q->isTextVisible())
            return;
        else
        {
            m_rect.moveCenter(q->rect().center());
            if(q->alignment() & Qt::AlignCenter)
                return;
            else
            {
                if(ThemeController::systemLang())
                    m_rect.setLeft(margins.right() + m_textRect.width()/*-gSpace*/);
                else
                    m_rect.setRight(q->rect().width() - margins.right() - m_textRect.width()/*-gSpace*/);
            }
        }
    }
    else
    {
        if(m_bodyWidth != 0)
            m_rect.setWidth(m_bodyWidth);
        if(!q->isTextVisible())
            return;
        else
        {
            m_rect.moveCenter(q->rect().center());
            if(q->alignment() & Qt::AlignCenter)
                return;
            else
            {
                m_rect.setTop(margins.top() + m_textRect.height()/* + gSpace*/);
            }
        }
    }
}

void KProgressBarPrivate::calculateContenteRect()//填充矩形
{
    Q_Q(KProgressBar);
    m_contentRect = m_rect;
    if(q->orientation() == Qt::Horizontal)
    {
        int width;
        qint64 totalSteps = qint64(q->maximum()) - q->minimum();
        width = m_rect.width() * (q->value() - q->minimum()) /totalSteps;
        if(!width)
        {
            m_contentRect = QRect();
        }
        //没设置反方向的情况下，即普通情况下
        if(!q->invertedAppearance())
        {
            m_contentRect.setRight(width+m_rect.left());
        }
        else
        {
            m_contentRect.setLeft(m_rect.width() - width);
        }
    }
    else
    {
        int height;
        qint64 totalSteps = q->maximum() - q->minimum();
        height = m_rect.height() * (q->value() - q->minimum()) /totalSteps;
        if(!height)
        {
            m_contentRect = QRect();
        }
        //没设置反方向的情况下，即普通情况下
        if(!q->invertedAppearance())
        {
            m_contentRect.setTop(m_rect.top() + m_rect.height() - height);
        }
        else
        {
            m_contentRect.setBottom(m_rect.top() +height);
        }
    }
}

void KProgressBarPrivate::changeTheme()
{
    Q_Q(KProgressBar);
    initThemeStyle();
    q->repaint();
}
}

#include "kprogressbar.moc"
#include "moc_kprogressbar.cpp"
