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

#include "ktag.h"
#include "themeController.h"
#include "ktoolbutton.h"
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>
#include <QToolButton>
#include <QStyleOption>
#include <QDebug>
#include <QPalette>
#include "parmscontroller.h"

namespace kdk {
class KTagPrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KTag)

public:
    KTagPrivate(KTag*parent);
    void changeTheme();

private:
    KTag* q_ptr;
    QString m_text;
    bool m_isClosable;
    KToolButton* m_pDeleteBtn;
    TagStyle m_style;
};

KTag::KTag(QWidget *parent)
    :QPushButton(parent)
    ,d_ptr(new KTagPrivate(this))
{
    Q_D(KTag);
    setClosable(false);
    d->changeTheme();
    connect(Parmscontroller::self(),&Parmscontroller::modeChanged,this,[=](bool flag){
        updateGeometry();
    });
}

void KTag::setClosable(bool flag)
{
    Q_D(KTag);
    d->m_isClosable = flag;
    if(flag)
        setMinimumWidth(88);
    update();
}

bool KTag::closable()
{
    Q_D(KTag);
    return d->m_isClosable;
}

void KTag::setText(const QString &text)
{
    Q_D(KTag);
    d->m_text = text;
    update();
}

void KTag::setTagStyle(TagStyle style)
{
    Q_D(KTag);
    d->m_style = style;
    d->changeTheme();
}

TagStyle KTag::tagStyle()
{
    Q_D(KTag);
    return d->m_style;
}

QString KTag::text()
{
    Q_D(KTag);
    return d->m_text;
}

void KTag::paintEvent(QPaintEvent *event)
{
    Q_D(KTag);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        if(painter.pen().width() == 1)
            painter.translate(0.5,0.5);
    }
    QRect rect = this->rect();
    if(d->m_isClosable)
    {
        d->m_pDeleteBtn->show();
        d->m_pDeleteBtn->move(width()-d->m_pDeleteBtn->width(),(height()-d->m_pDeleteBtn->height())/2+2);
        rect.adjust(0,0,-16,0);
    }
    else
    {
        d->m_pDeleteBtn->hide();
    }
    QColor color = palette().color(QPalette::Highlight);
    switch (d->m_style)
    {
    case HighlightTag:

        if(ThemeController::widgetTheme() == ClassicTheme)
        {
            QColor backColor=color;
            backColor.setAlpha(38);
            painter.setBrush(backColor);
            painter.setPen(color);
            painter.drawRoundedRect(this->rect(),0,0);
            painter.drawText(rect,Qt::AlignCenter,d->m_text);
        }
        else
        {
            painter.setBrush(color);
            painter.setPen(Qt::PenStyle::NoPen);
            painter.drawRoundedRect(this->rect(),6,6);
            painter.setPen(QColor("#FFFFFF"));
            painter.drawText(rect,Qt::AlignCenter,d->m_text);
        }
        break;
    case BoderTag:
        painter.setBrush(QColor(Qt::transparent));
        painter.setPen(color);
        if(ThemeController::widgetTheme() == ClassicTheme)
            painter.drawRoundedRect(this->rect(),0,0);
        else
            painter.drawRoundedRect(this->rect(),6,6);
        painter.drawText(rect,Qt::AlignCenter,d->m_text);
        break;
    case BaseBoderTag:
        {
            QColor backColor=color;
            backColor.setAlpha(38);
            painter.setBrush(backColor);
            painter.setPen(color);
            if(ThemeController::widgetTheme() == ClassicTheme)
                painter.drawRoundedRect(this->rect(),0,0);
            else
                painter.drawRoundedRect(this->rect(),6,6);
            painter.drawText(rect,Qt::AlignCenter,d->m_text);
            break;
        }
    case GrayTag:

        if(ThemeController::widgetTheme() == ClassicTheme)
        {
            painter.setBrush(QColor(Qt::transparent));
            painter.setPen(color);
            painter.drawRoundedRect(this->rect(),0,0);
            painter.drawText(rect,Qt::AlignCenter,d->m_text);
        }
        else
        {
            painter.setPen(Qt::PenStyle::NoPen);
            painter.setBrush(this->palette().color(QPalette::Disabled,QPalette::Highlight));
            painter.drawRoundedRect(this->rect(),6,6);
            painter.setPen(this->palette().color(QPalette::Text));
            painter.drawText(rect,Qt::AlignCenter,d->m_text);
        }
        break;
    default:
        break;

    }
}

QSize KTag::sizeHint() const
{
    auto size= QPushButton::sizeHint();
    size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_TagHeight));
    return size;
}

KTagPrivate::KTagPrivate(KTag *parent)
    :q_ptr(parent),
      m_isClosable(false),
      m_text(""),
      m_style(HighlightTag)
{
    Q_Q(KTag);
    m_pDeleteBtn = new KToolButton(q);
    m_pDeleteBtn->setIconSize(QSize(16,16));
    connect(m_pDeleteBtn,&KToolButton::clicked,q,&KTag::close);
    m_pDeleteBtn->hide();
    QPalette btnPalette;
    btnPalette.setBrush(QPalette::Active, QPalette::Button, Qt::transparent);
    btnPalette.setBrush(QPalette::Inactive, QPalette::Button, Qt::transparent);
    btnPalette.setBrush(QPalette::Disabled, QPalette::Button, Qt::transparent);

    btnPalette.setBrush(QPalette::Active, QPalette::Highlight, Qt::transparent);
    btnPalette.setBrush(QPalette::Inactive, QPalette::Highlight, Qt::transparent);
    btnPalette.setBrush(QPalette::Disabled, QPalette::Highlight, Qt::transparent);
    m_pDeleteBtn->setAutoFillBackground(true);
    m_pDeleteBtn->setPalette(btnPalette);
    m_pDeleteBtn->setFocusPolicy(Qt::NoFocus);
    m_pDeleteBtn->setCursor(Qt::ArrowCursor);
    connect(m_gsetting,&QGSettings::changed,this,[=](){changeTheme();});
}

void KTagPrivate::changeTheme()
{
    Q_Q(KTag);

    initThemeStyle();
    QIcon icon = QIcon::fromTheme("application-exit-symbolic");
    QPalette palette = q->palette();
    QSize size(m_pDeleteBtn->iconSize());
    switch (m_style)
    {
    case HighlightTag:
        m_pDeleteBtn->setIcon(drawColoredPixmap(icon.pixmap(size),QColor("#FFFFFF")));
        break;
    case BoderTag:
        m_pDeleteBtn->setIcon(drawColoredPixmap(icon.pixmap(size),palette.color(QPalette::Text)));
        break;
    case BaseBoderTag:
        m_pDeleteBtn->setIcon(drawColoredPixmap(icon.pixmap(size),palette.color(QPalette::Text)));
        break;
    case GrayTag:
        m_pDeleteBtn->setIcon(drawColoredPixmap(icon.pixmap(size),palette.color(QPalette::Text)));
        break;
    default:
        break;
    }
}
}

#include "ktag.moc"
#include "moc_ktag.cpp"
