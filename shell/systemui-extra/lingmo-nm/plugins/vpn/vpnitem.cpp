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
#include "vpnitem.h"
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#define FRAME_SPEED 150
#define LIMIT_TIME 60*1000
#define TOTAL_PAGE 8
#define RADIUS 6.0

#define THEME_QT_SCHEMA  "org.lingmo.style"
#define MODE_QT_KEY      "style-name"

VpnItem::VpnItem(bool bAcitve, QWidget *parent)
    : m_isAcitve(bAcitve), QPushButton(parent)
{
    this->setMinimumSize(550, 58);
    this->setProperty("useButtonPalette", true);
    this->setFlat(true);
    QHBoxLayout *mLanLyt = new QHBoxLayout(this);
    mLanLyt->setContentsMargins(16,0,16,0);
    mLanLyt->setSpacing(16);
    m_iconLabel = new QLabel(this);
    m_iconLabel->setProperty("useIconHighlightEffect", 0x2);
    m_titileLabel = new FixLabel(this);
    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_infoLabel = new GrayInfoButton(this);

    m_moreButton = new QToolButton(this);
    m_moreButton->setProperty("useButtonPalette", true);
    m_moreButton->setPopupMode(QToolButton::InstantPopup);
    m_moreButton->setAutoRaise(true);
    m_moreButton->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
    m_moreMenu = new QMenu(m_moreButton);
    m_connectAction = new QAction(m_moreMenu);
    m_deleteAction = new QAction(tr("Delete"), m_moreMenu);
    setConnectActionText(m_isAcitve);

    m_moreMenu->addAction(m_connectAction);
    m_moreMenu->addAction(m_deleteAction);
    m_moreButton->setMenu(m_moreMenu);
    mLanLyt->addWidget(m_iconLabel);
    mLanLyt->addWidget(m_titileLabel,Qt::AlignLeft);
    mLanLyt->addStretch();
    mLanLyt->addWidget(m_statusLabel);
    mLanLyt->addWidget(m_infoLabel);
    mLanLyt->addWidget(m_moreButton);

    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-1-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-2-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-3-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-4-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-5-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-6-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-7-symbolic"));

    m_waitTimer = new QTimer(this);
    connect(m_waitTimer, &QTimer::timeout, this, &VpnItem::updateIcon);

    connect(m_connectAction, &QAction::triggered, this, &VpnItem::onConnectTriggered);
    connect(m_deleteAction, &QAction::triggered, this, &VpnItem::onDeletetTriggered);
    m_moreMenu->installEventFilter(this);
}

void VpnItem::updateIcon()
{
    if (m_currentIconIndex > 6) {
        m_currentIconIndex = 0;
    }
    m_statusLabel->setPixmap(m_loadIcons.at(m_currentIconIndex).pixmap(16,16));
    m_currentIconIndex ++;
}

void VpnItem::startLoading()
{
    m_waitTimer->start(FRAME_SPEED);
    m_loading = true;
}

void VpnItem::stopLoading(){
    m_waitTimer->stop();
    m_loading = false;
}

void VpnItem::setConnectActionText(bool isAcitve)
{
    if (isAcitve) {
        m_connectAction->setText(tr("Disconnect"));
    } else {
        m_connectAction->setText(tr("Connect"));
    }
}

void VpnItem::onConnectTriggered()
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

void VpnItem::onDeletetTriggered()
{
    if (!m_deleteAction) {
        return;
    }
    Q_EMIT deleteActionTriggered();
}

void VpnItem::paintEvent(QPaintEvent *event)
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

#if 0
    if (!m_useHalfFillet) {
        painter.drawRect(rect);
    } else {
        QPainterPath path;
//        path.addRoundedRect (rect, RADIUS, RADIUS);
//        QRect temp_rect(rect.left(), rect.top(), rect.width(), rect.height()/2);
//        path.addRect(temp_rect);
        //设置起点
        path.moveTo(rect.topLeft().x(), rect.topLeft().y());
        path.lineTo(rect.bottomLeft().x(), rect.bottomLeft().y() - RADIUS);
        //绘制圆角 圆弧以外切圆的270度位置为起点，逆时针画圆弧运行90度结束
        path.arcTo(QRect(QPoint(rect.bottomLeft().x(), rect.bottomLeft().y() - (RADIUS * 2)), QSize(RADIUS * 2, RADIUS * 2)), 180, 90);
        path.lineTo(rect.bottomRight().x()  - RADIUS, rect.bottomRight().y());
        //画圆弧
        path.arcTo(QRect(QPoint(rect.bottomRight().x() - (RADIUS * 2), rect.bottomRight().y() - (RADIUS * 2)), QSize(RADIUS * 2, RADIUS * 2)), 270, 90);
        path.lineTo(rect.topRight());
        path.lineTo(rect.topLeft());
        painter.drawPath(path);
    }
#endif

    painter.drawRect(rect);
    QPushButton::paintEvent(event);
}

bool VpnItem::eventFilter(QObject *watched, QEvent *event)
{
    //菜单右边界与按钮右边界对齐
    if (event->type() == QEvent::Show && watched == m_moreMenu) {
        int menuWidth = m_moreMenu->size().width();
        int btnWidth = m_moreButton->size().width();
        int btnGlobalXPos = mapToGlobal(m_moreButton->pos()).x();

        QPoint pos = QPoint (btnGlobalXPos - menuWidth + btnWidth, m_moreMenu->pos().y());
        m_moreMenu->move(pos);
        return true;
    }
    return false;
}
