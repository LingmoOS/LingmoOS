// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netif_item_view_widget.h"
#include "chart_view_widget.h"
#include "common/common.h"
#include "system/netif.h"

#include <QtMath>
#include <QPainter>

#include <DApplication>
#include <DApplicationHelper>

using namespace common::format;
using namespace core::system;

const int margin = 6;
const int spacing = 6;
const int sectionSize = 6;
const int TextSpacing = 12;

DWIDGET_USE_NAMESPACE
NetifItemViewWidget::NetifItemViewWidget(QWidget *parent, const QByteArray &mac) : QWidget(parent)
{
    m_mac = mac;
    m_ifname = "Ethernet";

    m_ChartWidget = new ChartViewWidget(ChartViewWidget::ChartViewTypes::NET_CHART, this);
    m_ChartWidget->setData1Color(m_recvColor);
    m_ChartWidget->setData2Color(m_sentColor);
    m_ChartWidget->setSpeedAxis(true);
    updateWidgetGeometry();
}

void NetifItemViewWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    int curXMargin = m_mode == TITLE_HORIZONTAL ? 0 : margin;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setFont(m_font);

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    const DPalette &palette = dAppHelper->applicationPalette();

    const QColor &textColor = palette.color(DPalette::TextTips);
    if (m_isActive) {
        painter.setPen(Qt::NoPen);
        QColor selectColor = palette.color(DPalette::Highlight);
        selectColor.setAlphaF(0.1);

        painter.setBrush(selectColor);
        painter.drawRoundedRect(rect(), 8, 8);

        painter.setPen(palette.color(DPalette::Highlight));
    } else {
        painter.setPen(textColor);
    }


    QRect ifnameRect(curXMargin, margin / 2, painter.fontMetrics().width(m_ifname), painter.fontMetrics().height());
    painter.drawText(ifnameRect, Qt::AlignLeft | Qt::AlignVCenter, m_ifname);

    painter.setPen(textColor);
    if (m_mode == TITLE_HORIZONTAL) {
        QString strLeftBrackets = "(";
        QRect leftBracketsRect(ifnameRect.right() + spacing, ifnameRect.y(), painter.fontMetrics().width(strLeftBrackets), painter.fontMetrics().height());
        painter.drawText(leftBracketsRect, Qt::AlignLeft | Qt::AlignVCenter, strLeftBrackets);

        QString strRecvData = DApplication::translate("Process.Graph.Title", "Receive");
        strRecvData = strRecvData + " " + m_recv_bps;
        QRect recvRect(leftBracketsRect.right() + spacing * 2 + sectionSize, ifnameRect.y(), painter.fontMetrics().width(strRecvData), painter.fontMetrics().height());
        painter.drawText(recvRect, Qt::AlignLeft | Qt::AlignVCenter, strRecvData);

        QString strSentData = DApplication::translate("Process.Graph.View", "Send");
        strSentData = strSentData + " " + m_sent_bps + " )";
        QRect sentRect(sectionSize + recvRect.right() + 3 * spacing, ifnameRect.y(), painter.fontMetrics().width(strSentData), painter.fontMetrics().height());
        painter.drawText(sentRect, Qt::AlignLeft | Qt::AlignVCenter, strSentData);

        painter.setPen(Qt::NoPen);
        painter.setBrush(m_recvColor);
        painter.drawEllipse(leftBracketsRect.right() + spacing, leftBracketsRect.y() + qCeil((leftBracketsRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);

        painter.setBrush(m_sentColor);
        painter.drawEllipse(recvRect.right() + 2 * spacing, recvRect.y() + qCeil((recvRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);
    } else {

        QString strRecvData = DApplication::translate("Process.Graph.Title", "Receive");
        strRecvData = strRecvData + " " + m_recv_bps;
        QRect recvRect(margin + spacing + sectionSize, ifnameRect.bottom(), painter.fontMetrics().width(strRecvData), painter.fontMetrics().height());
        painter.drawText(recvRect, Qt::AlignLeft | Qt::AlignVCenter, strRecvData);

        QString strSentData = DApplication::translate("Process.Graph.View", "Send");
        strSentData = strSentData  + " " + m_sent_bps;
        QRect sentRect(margin + spacing + sectionSize, recvRect.bottom(), painter.fontMetrics().width(strSentData), painter.fontMetrics().height());
        painter.drawText(sentRect, Qt::AlignLeft | Qt::AlignVCenter, strSentData);


        painter.setPen(Qt::NoPen);
        painter.setBrush(m_recvColor);
        painter.drawEllipse(margin, recvRect.y() + qCeil((recvRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);

        painter.setBrush(m_sentColor);
        painter.drawEllipse(margin, sentRect.y() + qCeil((sentRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);
    }
}

void NetifItemViewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateWidgetGeometry();
}

void NetifItemViewWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    emit clicked(m_mac);
}

void NetifItemViewWidget::fontChanged(const QFont &font)
{
    m_font = font;
    this->setFont(m_font);
    updateWidgetGeometry();
}

void NetifItemViewWidget::setMode(int mode)
{
    m_mode = mode;
    this->update();
}

void NetifItemViewWidget::updateData(const std::shared_ptr<class core::system::NetifInfo> &netifInfo)
{
    m_ChartWidget->addData1(netifInfo->recv_bps());
    m_ChartWidget->addData2(netifInfo->sent_bps());

    if (!netifInfo->ifname().isNull()) {m_ifname = netifInfo->ifname();}

    m_recv_bps = formatUnit_net(netifInfo->recv_bps() * 8, B, 1, true);
    m_sent_bps = formatUnit_net(netifInfo->sent_bps() * 8, B, 1, true);

    this->update();
}

void NetifItemViewWidget::updateActiveStatus(bool active)
{
    m_isActive = active;
    update();
}

void NetifItemViewWidget::updateWidgetGeometry()
{
    int fontHeight = QFontMetrics(m_font).height();
    int curXMargin = m_mode == TITLE_HORIZONTAL ? 0 : margin;
    if (m_mode == TITLE_HORIZONTAL) {
        m_ChartWidget->setGeometry(curXMargin, TextSpacing, this->width() - 2 * curXMargin, this->height() - TextSpacing - margin);
    } else {
        m_ChartWidget->setGeometry(curXMargin, fontHeight * 2 + TextSpacing, this->width() - 2 * curXMargin, this->height() - fontHeight * 2 - TextSpacing - margin);
    }
}
