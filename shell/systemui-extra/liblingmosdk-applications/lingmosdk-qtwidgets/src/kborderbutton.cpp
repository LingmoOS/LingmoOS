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

#include "kborderbutton.h"
#include "themeController.h"
#include <QStyleOptionButton>
#include <QPainter>
#include <QDebug>
#include <QIcon>
#include "parmscontroller.h"

namespace kdk
{
class KBorderButtonPrivate:public QObject,public ThemeController
{
    Q_DECLARE_PUBLIC(KBorderButton)
    Q_OBJECT
public:
    KBorderButtonPrivate(KBorderButton*parent):q_ptr(parent)
    ,m_hoverd(false)
      ,m_radius(6)
    {setParent(parent);}
protected:
    void changeTheme();
private:
    KBorderButton* q_ptr;
    bool m_hoverd;
    int m_radius;
};
KBorderButton::KBorderButton(QWidget* parent)
    :QPushButton(parent)
    ,d_ptr(new KBorderButtonPrivate(this))
{
    Q_D(KBorderButton);
    connect(Parmscontroller::self(),&Parmscontroller::modeChanged,this,[=](){
        updateGeometry();
    });
}

KBorderButton::~KBorderButton()
{
}

QSize KBorderButton::sizeHint() const
{
    auto size = QPushButton::sizeHint();
    size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_PushButtonHeight));
    return size;
}
bool KBorderButton::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KBorderButton);
    return QPushButton::eventFilter(watched,event);
}

void KBorderButton::paintEvent(QPaintEvent *event)
{
    Q_D(KBorderButton);
    QStyleOptionButton option;
    initStyleOption(&option);
    QPainter p(this);
    QColor borderColor;
    QColor fontColor;
    QColor mix = option.palette.brightText().color();
    QColor highlight = option.palette.highlight().color();
    if(!option.state.testFlag(QStyle::State_Enabled))
    {
        borderColor = option.palette.color(QPalette::Disabled,QPalette::Button);
        fontColor = option.palette.color(QPalette::Disabled,QPalette::HighlightedText);
    }
    else if(option.state.testFlag(QStyle::State_MouseOver))
    {

        if(option.state.testFlag(QStyle::State_Sunken))
        {
            if(ThemeController::widgetTheme() == ClassicTheme)
            {
                if(ThemeController::themeMode() == LightTheme)
                {
                    borderColor =ThemeController::highlightClick(false,palette());
                    fontColor =ThemeController::highlightClick(false,palette());

                    borderColor = ThemeController::adjustH(borderColor,-2);
                    borderColor = ThemeController::adjustS(borderColor,-27);
                    borderColor = ThemeController::adjustL(borderColor,-19);
                    fontColor = ThemeController::adjustH(fontColor,4);
                    fontColor = ThemeController::adjustS(fontColor,-30);
                    fontColor = ThemeController::adjustL(fontColor,-40);
                }
                else
                {
                    borderColor =ThemeController::highlightClick(true,palette());
                    fontColor =ThemeController::highlightClick(true,palette());

                    borderColor = ThemeController::adjustH(borderColor,5);
                    borderColor = ThemeController::adjustS(borderColor,0);
                    borderColor = ThemeController::adjustL(borderColor,26);
                    fontColor = ThemeController::adjustH(fontColor,5);
                    fontColor = ThemeController::adjustS(fontColor,0);
                    fontColor = ThemeController::adjustL(fontColor,26);
                }
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
                if(ThemeController::themeMode() == LightTheme)
                {
                    borderColor =ThemeController::highlightHover(false,palette());
                    fontColor =ThemeController::highlightHover(false,palette());

                    borderColor = ThemeController::adjustH(borderColor,-2);
                    borderColor = ThemeController::adjustS(borderColor,-24);
                    borderColor = ThemeController::adjustL(borderColor,-24);
                    fontColor = ThemeController::adjustH(fontColor,-8);
                    fontColor = ThemeController::adjustS(fontColor,-1);
                    fontColor = ThemeController::adjustL(fontColor,-34);
                }
                else
                {
                    borderColor =ThemeController::highlightHover(true,palette());
                    fontColor =ThemeController::highlightHover(true,palette());

                    borderColor = ThemeController::adjustH(borderColor,-6);
                    borderColor = ThemeController::adjustS(borderColor,27);
                    borderColor = ThemeController::adjustL(borderColor,43);
                    fontColor = ThemeController::adjustH(fontColor,-6);
                    fontColor = ThemeController::adjustS(fontColor,27);
                    fontColor = ThemeController::adjustL(fontColor,43);
                }
            }
            else
            {
                borderColor = ThemeController::mixColor(highlight,mix,0.05);
                fontColor = ThemeController::mixColor(highlight,mix,0.05);
            }
        }
    }
    else if(option.state.testFlag(QStyle::State_HasFocus))
    {
        if(ThemeController::widgetTheme() == ClassicTheme)
        {
            if(ThemeController::themeMode() == LightTheme)
            {
                borderColor = ThemeController::highlightClick(false,palette());
                borderColor = ThemeController::adjustH(borderColor,-2);
                borderColor = ThemeController::adjustS(borderColor,-24);
                borderColor = ThemeController::adjustL(borderColor,-24);

                fontColor = option.palette.color(QPalette::ButtonText);;

            }
            else
            {
                borderColor = ThemeController::highlightClick(true,palette());
                borderColor = ThemeController::adjustH(borderColor,5);
                borderColor = ThemeController::adjustS(borderColor,0);
                borderColor = ThemeController::adjustL(borderColor,26);

                fontColor = option.palette.color(QPalette::ButtonText);;

            }
        }
        else
        {
            fontColor = option.palette.buttonText().color();
            borderColor = ThemeController::mixColor(highlight,mix,0.2);
        }
    }
    else
    {
        if(ThemeController::widgetTheme() == ClassicTheme)
        {
            fontColor = option.palette.buttonText().color();
            if(ThemeController::themeMode() == LightTheme )
                borderColor = ThemeController::lanhuHSLToQtHsl(0, 0, 73);
            else
                borderColor = ThemeController::lanhuHSLToQtHsl(0, 0, 25);
        }
        else
        {
            fontColor = option.palette.buttonText().color();
            borderColor = option.palette.button().color();
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

    p.save();
    pen.setWidth(1);
    pen.setColor(borderColor);
    p.setPen(pen);
    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        if(p.pen().width() == 1)
            p.translate(0.5,0.5);
        p.drawRoundedRect(option.rect.adjusted(1,1,-1,-1),0,0);
    }
    else
        p.drawRoundedRect(option.rect.adjusted(1,1,-1,-1),d->m_radius,d->m_radius);
    p.restore();
    pen.setWidth(1);
    pen.setColor(fontColor);
    p.setPen(pen);
    QPoint point;
    QRect ir = option.rect;
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

        p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);

        if (option.direction == Qt::RightToLeft)
            ir.translate(-point.x() - 2, 0);
        else
            ir.translate(point.x() + w + 4, 0);

        // left-align text if there is
        if (!option.text.isEmpty())
            tf |= Qt::AlignLeft;
    }
    else
    {
        tf |= Qt::AlignHCenter;
    }
    p.drawText(ir,tf,option.text);
}

KBorderButton::KBorderButton(const QString &text, QWidget *parent ):KBorderButton(parent)
{
    setText(text);
}

KBorderButton::KBorderButton(const QIcon &icon, const QString &text, QWidget *parent):KBorderButton(parent)
{
    setIcon(icon);
    setText(text);
}

void KBorderButton::setIcon(const QIcon &icon)
{
    Q_D(KBorderButton);
    QPushButton::setIcon(icon);
}

void KBorderButtonPrivate::changeTheme()
{
    Q_Q(KBorderButton);
    initThemeStyle();
}

}
#include "kborderbutton.moc"
#include "moc_kborderbutton.cpp"
