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
#include "firewalldialog.h"

#include <QApplication>
#include "windowmanager/windowmanager.h"
#include "kwindowsystem.h"
#include "kwindowsystem_export.h"

#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"
#define ICON_SIZE 16,16

FirewallDialog::FirewallDialog(QWidget *parent): KDialog(parent)
{
    initUI();
    this->setWindowIcon(QIcon::fromTheme("lingmo-network"));
    this->setFixedSize(480, 204);
    KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);
    setAttribute(Qt::WA_DeleteOnClose);
//    centerToScreen();
    connect(qApp, &QApplication::paletteChanged, this, &FirewallDialog::onPaletteChanged);
}

FirewallDialog::~FirewallDialog()
{

}

void FirewallDialog::initUI()
{
    m_iconLabel = new QLabel(this);
    m_contentLabel = new FixLabel(this);
    m_suggestLabel = new FixLabel(this);
    m_PublicBtn = new QPushButton(this);
    m_PrivateBtn = new QPushButton(this);
    m_dialogLayout = new QVBoxLayout(this);

    m_contentLabel->setFixedWidth(405);
    m_suggestLabel->setFixedWidth(405);

    QWidget *contentWidget = new QWidget(this);
    QGridLayout *contentLayout = new QGridLayout(contentWidget);
    contentLayout->setHorizontalSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->addWidget(m_iconLabel, 0, 0);
    contentLayout->addWidget(m_contentLabel, 0, 1);
    contentLayout->addWidget(m_suggestLabel, 1, 1);
    m_iconLabel->setFixedWidth(24);

    QWidget *btnWidget = new QWidget(this);
    QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(16);
    btnLayout->addStretch();
    btnLayout->addWidget(m_PublicBtn);
    btnLayout->addWidget(m_PrivateBtn);

    m_dialogLayout->setContentsMargins(22, 16, 22, 24);
    m_dialogLayout->setSpacing(0);
    m_dialogLayout->addWidget(contentWidget);
    m_dialogLayout->addStretch();
    m_dialogLayout->addWidget(btnWidget);

    QIcon icon = QIcon::fromTheme("dialog-info");
    m_iconLabel->setPixmap(icon.pixmap(ICON_SIZE));

    QFont font = m_contentLabel->font();
    font.setWeight(57);
    m_contentLabel->setFont(font);
    //是否允许此网络上的其他设备发现这台电脑？
    m_contentLabel->setLabelText(tr("Allow other devices on this network to discover this computer?"));
    //不建议在公共网络上开启此功能
    m_suggestLabel->setLabelText(tr("It is not recommended to enable this feature on public networks"));

    m_PublicBtn->setText(tr("Not allowed (recommended)"));
    m_PrivateBtn->setText(tr("Allowed"));

    this->closeButton();
    this->mainWidget()->setLayout(m_dialogLayout);
    onPaletteChanged();

    connect(m_PublicBtn, &QPushButton::clicked, this, &FirewallDialog::setPublicNetMode);
    connect(m_PrivateBtn, &QPushButton::clicked, this, &FirewallDialog::setPrivateNetMode);
}

void FirewallDialog::centerToScreen()
{
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = this->width();
    int y = this->height();
    this->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
}

void FirewallDialog::onPaletteChanged()
{
    QPalette pal = qApp->palette();

    QGSettings * styleGsettings = nullptr;
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
       styleGsettings = new QGSettings(style_id);
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "lingmo-default"){
           pal = lightPalette(this);
       }
    }
    this->setPalette(pal);

    if (styleGsettings != nullptr) {
        delete styleGsettings;
        styleGsettings = nullptr;
    }
}
