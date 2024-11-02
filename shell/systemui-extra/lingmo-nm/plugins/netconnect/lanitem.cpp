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
#include "lanitem.h"
#include <QApplication>

#define FRAME_SPEED 150
#define LIMIT_TIME 60*1000
#define TOTAL_PAGE 8

#define THEME_QT_SCHEMA  "org.lingmo.style"
#define MODE_QT_KEY      "style-name"

LanItem::LanItem(bool isAcitve, QWidget *parent)
    : QPushButton(parent),isAcitve(isAcitve)
{
    this->setMinimumSize(550, 58);
    this->setProperty("useButtonPalette", true);
    this->setFlat(true);
//    setStyleSheet("QPushButton:!checked{background-color: palette(base)}");
    QHBoxLayout *mLanLyt = new QHBoxLayout(this);
    mLanLyt->setContentsMargins(16,0,16,0);
    mLanLyt->setSpacing(16);
    iconLabel = new QLabel(this);
    iconLabel->setProperty("useIconHighlightEffect", 0x2);
    titileLabel = new FixLabel(this);
    statusLabel = new QLabel(this);
    statusLabel->setProperty("useIconHighlightEffect", 0x2);
    statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
//    statusLabel->setMinimumSize(36,36);
    infoLabel = new GrayInfoButton(this);

    //【更多】菜单
    m_moreButton = new QToolButton(this);
    m_moreButton->setProperty("useButtonPalette", true);
    m_moreButton->setPopupMode(QToolButton::InstantPopup);
    m_moreButton->setAutoRaise(true);
    m_moreButton->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
    m_moreMenu = new QMenu(m_moreButton);
    m_connectAction = new QAction(m_moreMenu);
    m_deleteAction = new QAction(tr("Delete"), m_moreMenu);
    setConnectActionText(isAcitve);

    m_moreMenu->addAction(m_connectAction);
    m_moreMenu->addAction(m_deleteAction);
    m_moreButton->setMenu(m_moreMenu);

    mLanLyt->addWidget(iconLabel);
    mLanLyt->addWidget(titileLabel,Qt::AlignLeft);
    mLanLyt->addStretch();
    mLanLyt->addWidget(statusLabel);
    mLanLyt->addWidget(infoLabel);
    mLanLyt->addWidget(m_moreButton);

    loadIcons.append(QIcon::fromTheme("lingmo-loading-1-symbolic"));
    loadIcons.append(QIcon::fromTheme("lingmo-loading-2-symbolic"));
    loadIcons.append(QIcon::fromTheme("lingmo-loading-3-symbolic"));
    loadIcons.append(QIcon::fromTheme("lingmo-loading-4-symbolic"));
    loadIcons.append(QIcon::fromTheme("lingmo-loading-5-symbolic"));
    loadIcons.append(QIcon::fromTheme("lingmo-loading-6-symbolic"));
    loadIcons.append(QIcon::fromTheme("lingmo-loading-7-symbolic"));
    waitTimer = new QTimer(this);
    connect(waitTimer, &QTimer::timeout, this, &LanItem::updateIcon);
    connect(m_connectAction, &QAction::triggered, this, &LanItem::onConnectTriggered);
    connect(m_deleteAction, &QAction::triggered, this, &LanItem::onDeletetTriggered);
    m_moreMenu->installEventFilter(this);
}

LanItem::~LanItem()
{

}

void LanItem::updateIcon()
{
    if (currentIconIndex > 6) {
        currentIconIndex = 0;
    }
    statusLabel->setPixmap(loadIcons.at(currentIconIndex).pixmap(16,16));
    currentIconIndex ++;
}

void LanItem::startLoading()
{
    waitTimer->start(FRAME_SPEED);
    loading = true;
}

void LanItem::stopLoading(){
    waitTimer->stop();
    loading = false;
}

/**
 * @brief LanItem::setConnectActionText
 * 【更多】菜单状态切换 连接/断开
 * @param isAcitve
 */
void LanItem::setConnectActionText(bool isAcitve)
{
    if (isAcitve) {
        m_connectAction->setText(tr("Disconnect"));
    } else {
        m_connectAction->setText(tr("Connect"));
    }
}

void LanItem::onConnectTriggered()
{
    if (!m_connectAction) {
        return;
    }
    if (m_connectAction->text() == tr("Connect")) {
        Q_EMIT connectActionTriggered();
    } else if (m_connectAction->text() == tr("Disconnect")) {
        Q_EMIT disconnectActionTriggered();
    }
}

void LanItem::onDeletetTriggered()
{
    if (!m_deleteAction) {
        return;
    }
    Q_EMIT deleteActionTriggered();
}

void LanItem::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter:: Antialiasing, true);  //设置渲染,启动反锯齿
    painter.setPen(Qt::NoPen);
    painter.setBrush(this->palette().base().color());

    QPalette pal = qApp->palette();
    QColor color = pal.color(QPalette::Button);
    color.setAlphaF(0.5);
    pal.setColor(QPalette::Button, color);
    this->setPalette(pal);

    QRect rect = this->rect();

    painter.drawRect(rect);
    QPushButton::paintEvent(event);
}

bool LanItem::eventFilter(QObject *watched, QEvent *event)
{
    //菜单右边界与按钮右边界对齐
    if (event->type() == QEvent::Show && watched == m_moreMenu) {
        int menuXPos = mapToGlobal(m_moreButton->pos()).x();
        int menuWidth = m_moreMenu->size().width();
        int btnWidth = m_moreButton->size().width();

        QPoint pos = QPoint (menuXPos - menuWidth + btnWidth, m_moreMenu->pos().y());
        m_moreMenu->move(pos);
        return true;
    }
    return false;
}
