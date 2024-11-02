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

#include "ksecuritylevelbar.h"
#include <QPainter>
#include <QLabel>
#include <QBoxLayout>
#include <QPalette>
#include "themeController.h"

namespace kdk
{
class KSecurityLevelBarPrivate:public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KSecurityLevelBar)
public:
    KSecurityLevelBarPrivate(KSecurityLevelBar* parent);

private:
    KSecurityLevelBar* q_ptr;
    SecurityLevel m_level;
    QLabel* m_pLabel;
    int m_contentMargin = 5;
    int m_textWidth = 0;
    int m_space = 5;
};

KSecurityLevelBar::KSecurityLevelBar(QWidget *parent)
    : QWidget(parent),
      d_ptr(new KSecurityLevelBarPrivate(this))
{
    Q_D(KSecurityLevelBar);
    setFixedSize(300,30);
}

void KSecurityLevelBar::setSecurityLevel(SecurityLevel level)
{
    Q_D(KSecurityLevelBar);
    d->m_level = level;
    switch (level) {
    case Low:
        d->m_pLabel->setText(tr("Low"));
        break;
    case Medium:
        d->m_pLabel->setText(tr("Medium"));
        break;
    case High:
        d->m_pLabel->setText(tr("High"));
        break;
    default:
        break;
    }
    repaint();
}

SecurityLevel KSecurityLevelBar::securityLevel()
{
    Q_D(KSecurityLevelBar);
    return d->m_level;
}

void KSecurityLevelBar::paintEvent(QPaintEvent *event)
{
    Q_D(KSecurityLevelBar);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    d->m_pLabel->adjustSize();
    d->m_textWidth = this->fontMetrics().width(d->m_pLabel->text()) + d->m_space;
    int rectWitdth = (rect().width() - d->m_textWidth - d->m_contentMargin*2)/3 - d->m_space;
    int rectHeight = height() > 6 ? 6 : height();
    int currentPos = d->m_contentMargin + d->m_textWidth + d->m_space;
    if(ThemeController::widgetTheme() == ClassicTheme)
    {            
//        if(painter.pen().width() == 1)
//            painter.translate(0.5,0.5);
//        if(ThemeController::themeMode() == LightTheme)
//            painter.setPen(QColor(187,187,187));
//        else
//            painter.setPen(QColor(89,89,89));
        switch (d->m_level)
        {
        case Low:
            painter.setBrush(QColor(243,34,45));
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,0,0);
            currentPos = currentPos + d->m_space + rectWitdth;
            painter.setBrush(this->palette().color(QPalette::Button));
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,0,0);
            currentPos = currentPos + d->m_space + rectWitdth;
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,0,0);
            break;
        case Medium:
            painter.setBrush(QColor(249,197,61));
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,0,0);
            currentPos = currentPos + d->m_space + rectWitdth;
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,0,0);
            currentPos = currentPos + d->m_space + rectWitdth;
            painter.setBrush(this->palette().color(QPalette::Midlight));
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,0,0);
            break;
        case High:
            painter.setBrush(QColor(82,196,41));
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,0,0);
            currentPos = currentPos + d->m_space + rectWitdth;
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,0,0);
            currentPos = currentPos + d->m_space + rectWitdth;
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,0,0);
            break;
        default:
            break;
        }
    }
    else
    {
        switch (d->m_level)
        {
        case Low:
            painter.setBrush(QColor(243,34,45));
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,rectHeight,rectHeight);
            currentPos = currentPos + d->m_space + rectWitdth;
            painter.setBrush(this->palette().color(QPalette::Button));
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,rectHeight,rectHeight);
            currentPos = currentPos + d->m_space + rectWitdth;
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,rectHeight,rectHeight);
            break;
        case Medium:
            painter.setBrush(QColor(249,197,61));
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,rectHeight,rectHeight);
            currentPos = currentPos + d->m_space + rectWitdth;
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,rectHeight,rectHeight);
            currentPos = currentPos + d->m_space + rectWitdth;
            painter.setBrush(this->palette().color(QPalette::Button));
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,rectHeight,rectHeight);
            break;
        case High:
            painter.setBrush(QColor(82,196,41));
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,rectHeight,rectHeight);
            currentPos = currentPos + d->m_space + rectWitdth;
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,rectHeight,rectHeight);
            currentPos = currentPos + d->m_space + rectWitdth;
            painter.drawRoundedRect(currentPos,(rect().height()-rectHeight)/2,rectWitdth,rectHeight,rectHeight,rectHeight);
            break;
        default:
            break;
        }
    }


}

KSecurityLevelBarPrivate::KSecurityLevelBarPrivate(KSecurityLevelBar *parent)
    :q_ptr(parent)
{
    Q_Q(KSecurityLevelBar);
    m_pLabel = new QLabel(q);
    m_pLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    m_pLabel->setGeometry(q->rect().top(),q->rect().left(),m_textWidth,q->rect().height());
    m_level = Low;
    m_pLabel->setText(tr("Low"));
    setParent(parent);
}
}
#include "ksecuritylevelbar.moc"
#include "moc_ksecuritylevelbar.cpp"
