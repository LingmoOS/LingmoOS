/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "listitem.h"
#include <QDebug>
#include "themepalette.h"

#define MAIN_LAYOUT_MARGINS 0,0,0,0
#define MAIN_LAYOUT_SPACING 0
#define ITEM_FRAME_MARGINS 12,6,16,6

#define ITEM_FRAME_SPACING 8
#define FRAME_WIDTH 404
#define INFO_ICON_WIDTH 16
#define INFO_ICON_HEIGHT 16

#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"

ListItem::ListItem(QWidget *parent) : QFrame(parent)
{
    m_connectState = UnknownState;

    initUI();
    initConnection();
    connect(qApp, &QApplication::paletteChanged, this, &ListItem::onPaletteChanged);
//    m_itemFrame->installEventFilter(this);
    onPaletteChanged();
}

ListItem::~ListItem()
{
    if (nullptr != m_netButton) {
        delete m_netButton;
        m_netButton = nullptr;
    }

    if (nullptr != m_infoButton) {
        delete m_infoButton;
        m_infoButton = nullptr;
    }

}

void ListItem::setName(const QString &name)
{
    m_nameLabel->setText(name);
}

//仅无线调用，有线自己获取
void ListItem::setActive(const bool &isActive)
{
    m_netButton->setActive(isActive);
    m_isActive = isActive;
}

void ListItem::setConnectState(ConnectState state)
{
    m_connectState = state;
}

void ListItem::mousePressEvent(QMouseEvent *event)
{
    qDebug()<<"[ListItem]"<<"mousePressEvent";
    if (event->button() == Qt::LeftButton) {
        onNetButtonClicked();
    } else if (event->button() == Qt::RightButton) {
        onRightButtonClicked();
    }
    return QFrame::mousePressEvent(event);
}

void ListItem::enterEvent(QEvent *event)
{
}

void ListItem::leaveEvent(QEvent *event)
{
}

void ListItem::paintEvent(QPaintEvent *event)
{
}

void ListItem::initUI()
{
    m_menu = new QMenu(this);//右键菜单
//    m_menu->setStyleSheet("QMenu::item{border:3px; border-radius:3px}");
//    m_menu->setStyleSheet("QMenu{border-radius:6px; margin:6px 6px 6px 6px}");
    connect(m_menu, &QMenu::triggered, this, &ListItem::onMenuTriggered);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(MAIN_LAYOUT_MARGINS);
    m_mainLayout->setSpacing(MAIN_LAYOUT_SPACING);
    this->setLayout(m_mainLayout);

    m_itemFrame = new QFrame(this);
    m_itemFrame->setFixedWidth(FRAME_WIDTH);

    m_hItemLayout = new QHBoxLayout(m_itemFrame);
    m_hItemLayout->setContentsMargins(ITEM_FRAME_MARGINS);
    m_hItemLayout->setSpacing(ITEM_FRAME_SPACING);
    m_hItemLayout->setAlignment(Qt::AlignHCenter);

    m_netButton = new RadioItemButton(m_itemFrame);
    m_nameLabel = new QLabel(m_itemFrame);
    m_nameLabel->setMinimumWidth(262);
    m_infoButton = new InfoButton(m_itemFrame);
    m_infoButton->setIconSize(QSize(INFO_ICON_WIDTH,INFO_ICON_HEIGHT));

    m_hItemLayout->addWidget(m_netButton);
    m_hItemLayout->addWidget(m_nameLabel);
    m_hItemLayout->addStretch();
    m_hItemLayout->addWidget(m_infoButton);

    m_mainLayout->addWidget(m_itemFrame);

//    this->setAutoFillBackground(true);
//    this->setBackgroundRole(QPalette::Base);
//    QPalette pal = qApp->palette();
//    pal.setColor(QPalette::Window, qApp->palette().base().color());
//    this->setPalette(pal);
}


void ListItem::initConnection()
{
    connect(this->m_netButton, &RadioItemButton::clicked, this, &ListItem::onNetButtonClicked);
    //    connect(this->m_infoButton, &InfoButton::clicked, this, &ListItem::onInfoButtonClicked);
}

void ListItem::onPaletteChanged()
{
    QPalette pal = qApp->palette();
    QGSettings * styleGsettings = nullptr;
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
       styleGsettings = new QGSettings(style_id, QByteArray(), this);
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "lingmo-default"){
           pal = themePalette(true, this);
       }
    }
    this->setPalette(pal);

    if (m_menu != nullptr) {
        pal.setColor(QPalette::Base, pal.color(QPalette::Base));
        pal.setColor(QPalette::Text, pal.color(QPalette::Text));
        m_menu->setPalette(pal);
    }
}

