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

#include "kborderlessbutton.h"
#include "themeController.h"
#include <QStyleOptionButton>
#include <QPainter>

namespace kdk
{
class KBorderlessButtonPrivate:public QObject,public ThemeController
{
    Q_DECLARE_PUBLIC(KBorderlessButton)
    Q_OBJECT
public:
    KBorderlessButtonPrivate(KBorderlessButton*parent):
    q_ptr(parent),m_hoverd(false)
    {
        setParent(parent);
    }

protected:
    void changeTheme();
private:
    KBorderlessButton *q_ptr;
    bool m_hoverd;
};

KBorderlessButton::KBorderlessButton(QWidget* parent):
    QPushButton(parent)
    ,d_ptr(new KBorderlessButtonPrivate(this))
{
    Q_D(KBorderlessButton);
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    setFocusPolicy(Qt::NoFocus);
}

KBorderlessButton::KBorderlessButton(const QString &text, QWidget *parent):KBorderlessButton(parent)
{
    setText(text);
}

KBorderlessButton::KBorderlessButton(const QIcon &icon, const QString &text, QWidget *parent):KBorderlessButton(parent)
{
    setIcon(icon);
    setText(text);
}

KBorderlessButton::KBorderlessButton(const QIcon &icon, QWidget *parent):KBorderlessButton(parent)
{
    setIcon(icon);
}

KBorderlessButton::~KBorderlessButton()
{
}

void KBorderlessButton::setIcon(const QIcon &icon)
{
    Q_D(KBorderlessButton);
    QPushButton::setIcon(icon);
    d->changeTheme();
}

bool KBorderlessButton::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KBorderlessButton);
    return QPushButton::eventFilter(watched,event);
}

void KBorderlessButton::paintEvent(QPaintEvent *event)
{
    Q_D(KBorderlessButton);
    QStyleOptionButton option;
    initStyleOption(&option);
    QPainter p(this);
    QColor borderColor;
    QColor fontColor;
    QColor highlight = option.palette.highlight().color();
    QColor mix = option.palette.brightText().color();
    if(!option.state.testFlag(QStyle::State_Enabled))
    {
        borderColor = option.palette.color(QPalette::Disabled,QPalette::Button);
        fontColor = option.palette.color(QPalette::Disabled,QPalette::HighlightedText);
    }
    else if(ThemeController::themeMode() == ThemeFlag::LightTheme){

        if(option.state.testFlag(QStyle::State_MouseOver))
        {
            if(option.state.testFlag(QStyle::State_Sunken))
            {
                if(ThemeController::widgetTheme() == ClassicTheme)
                {
                    fontColor =ThemeController::highlightClick(false,palette());

                    fontColor = ThemeController::adjustH(fontColor,2);
                    fontColor = ThemeController::adjustS(fontColor,-30);
                    fontColor = ThemeController::adjustL(fontColor,-40);
                }
                else
                {
                    borderColor = ThemeController::mixColor(highlight,mix,0.2);
                    fontColor = ThemeController::mixColor(highlight,mix,0.2);
                }
            }
            else
            {
                if(ThemeController::widgetTheme() == ClassicTheme)
                {
                    fontColor =ThemeController::highlightHover(false,palette());
                    fontColor = ThemeController::adjustH(fontColor,-3);
                    fontColor = ThemeController::adjustS(fontColor,-2);
                    fontColor = ThemeController::adjustL(fontColor,-36);
                }
                else
                {
                    borderColor = ThemeController::mixColor(highlight,mix,0.1);
                    fontColor = ThemeController::mixColor(highlight,mix,0.1);
                }
            }
        }
        else
        {
            if(!this->icon().isNull()){
                fontColor = option.palette.buttonText().color();
                borderColor = option.palette.button().color();
            }
            else {
                borderColor = highlight;
                fontColor = highlight;
            }
        }
    }
    else{
        if(option.state.testFlag(QStyle::State_MouseOver))
        {
            if(option.state.testFlag(QStyle::State_Sunken))
            {
                if(ThemeController::widgetTheme() == ClassicTheme)
                {
                    fontColor =ThemeController::highlightClick(true,palette());

                    fontColor = ThemeController::adjustH(fontColor,5);
                    fontColor = ThemeController::adjustS(fontColor,-0);
                    fontColor = ThemeController::adjustL(fontColor,26);
                }
                else
                {
                    borderColor = ThemeController::mixColor(highlight,mix,0.05);
                    fontColor = ThemeController::mixColor(highlight,mix,0.05);
                }
            }
            else
            {
                if(ThemeController::widgetTheme() == ClassicTheme)
                {
                    fontColor =ThemeController::highlightHover(true,palette());

                    fontColor = ThemeController::adjustH(fontColor,-6);
                    fontColor = ThemeController::adjustS(fontColor,27);
                    fontColor = ThemeController::adjustL(fontColor,42);
                }
                else
                {
                    borderColor = ThemeController::mixColor(highlight,mix,0.2);
                    fontColor = ThemeController::mixColor(highlight,mix,0.2);
                }
            }
        }
        else
        {
            if(!this->icon().isNull()){
                fontColor = option.palette.buttonText().color();
                borderColor = option.palette.button().color();
            }
            else {
                borderColor = highlight;
                fontColor = highlight;
            }
        }
    }
    p.setBrush(Qt::NoBrush);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setWidth(1);
    pen.setColor(fontColor);
    p.setPen(pen);
    QPoint point;
    // bug 170516 修正1px
    QRect ir = option.rect.adjusted(0,0,-1,0);
    uint tf = Qt::AlignVCenter;
    if (!option.icon.isNull()) {
        //Center both icon and text
        QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal
                                                         : QIcon::Disabled;

        if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
            mode = QIcon::Active;
        QIcon::State state = QIcon::Off;
        if (option.state & QStyle::State_On)
            state = QIcon::On;

        QPixmap pixmap = option.icon.pixmap(option.iconSize, mode, state);
        pixmap = ThemeController::drawColoredPixmap(this->icon().pixmap(iconSize()),fontColor);
        int w = pixmap.width() / pixmap.devicePixelRatio();
        int h = pixmap.height() / pixmap.devicePixelRatio();

        if (!option.text.isEmpty())
            w += option.fontMetrics.boundingRect(option.rect, tf, option.text).width() + 2;

        point = QPoint(ir.x() + ir.width() / 2 - w / 2,
                       ir.y() + ir.height() / 2 - h / 2);

        w = pixmap.width() / pixmap.devicePixelRatio();

        if (option.direction == Qt::RightToLeft)
            point.rx() += w;


        if (option.direction == Qt::RightToLeft)
            ir.translate(-point.x() - 2, 0);
        else
            ir.translate(point.x() + w + 4, 0);
        // left-align text if there is
        if (!option.text.isEmpty())
            tf |= Qt::AlignLeft;
        QFontMetrics fm=option.fontMetrics;
        int width = option.iconSize.width();
        int height = option.iconSize.height();
        if(fm.width(option.text) > option.rect.width()-width-4)
        {
            QRect rect(option.rect.x(),(option.rect.height()-height)/2,width,height);
            p.drawPixmap(rect,pixmap);
            QRect rect1(width+4,0,option.rect.width()-width-4,option.rect.height());
            QString elidedText1 = fm.elidedText(option.text,Qt::ElideRight,option.rect.width()-width-4);
            p.drawText(rect1,tf,elidedText1);
            if(toolTip().isNull())
                setToolTip(option.text);
        }
        else
        {
            p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);
            p.drawText(ir,tf,option.text);
        }
    }
    else
    {
        tf |= Qt::AlignHCenter;
        QFontMetrics fm=option.fontMetrics;
        QString elidedText = fm.elidedText(option.text,Qt::ElideRight,option.rect.width());
        p.drawText(option.rect,tf,elidedText);
        if(fm.width(option.text) > option.rect.width())
            setToolTip(option.text);
    }

}

QSize KBorderlessButton::sizeHint() const
{
    QFontMetrics fm(this->font());
    QSize size;
    if(this->icon().isNull())
        size = QSize(fm.width(this->text()) + 2, fm.height());

    else
    {
        int height = std::max(fm.height(),this->iconSize().height());
        int width  = fm.width(this->text()) + this->iconSize().width() + 4;
        size = QSize(width,height);
    }
    return size;
}

void KBorderlessButtonPrivate::changeTheme()
{
    Q_Q(KBorderlessButton);
    initThemeStyle();
}
}
#include "kborderlessbutton.moc"
#include "moc_kborderlessbutton.cpp"
