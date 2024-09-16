// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compact_network_monitor.h"

#include "smooth_curve_generator.h"
#include "common/common.h"
#include "system/device_db.h"
#include "system/net_info.h"
#include "system/system_monitor.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyle>

#include <QDebug>
#include <QPainter>
#include <QtMath>
#include <QPainterPath>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

using namespace core::system;
using namespace common;
using namespace common::format;

const int gridSize = 10;
const int pointsNumber = 30;
CompactNetworkMonitor::CompactNetworkMonitor(QWidget *parent)
    : QWidget(parent)
{
    auto *dAppHelper = DApplicationHelper::instance();

    int statusBarMaxWidth = common::getStatusBarMaxWidth();
    setFixedWidth(statusBarMaxWidth);
    setFixedHeight(150);

    downloadSpeeds = new QList<double>();
    for (int i = 0; i <= pointsNumber; i++) {
        downloadSpeeds->append(0);
    }

    uploadSpeeds = new QList<double>();
    for (int i = 0; i <= pointsNumber; i++) {
        uploadSpeeds->append(0);
    }

    connect(dAppHelper, &DApplicationHelper::themeTypeChanged, this,
            &CompactNetworkMonitor::changeTheme);
    changeTheme(dAppHelper->themeType());

    connect(SystemMonitor::instance(), &SystemMonitor::statInfoUpdated, this, &CompactNetworkMonitor::updateStatus);

    changeFont(DApplication::font());
    connect(dynamic_cast<QGuiApplication *>(DApplication::instance()),
            &DApplication::fontChanged,
            this,
            &CompactNetworkMonitor::changeFont);
}

CompactNetworkMonitor::~CompactNetworkMonitor()
{
    delete downloadSpeeds;
    delete uploadSpeeds;
}

void CompactNetworkMonitor::getPainterPathByData(QList<double> *listData, QPainterPath &path, qreal maxVlaue)
{
    qreal offsetX = 0;
    qreal distance = (this->width() - 2) * 1.0 / pointsNumber;
    int dataCount = listData->size();

    for (int i = 0;  i < dataCount - 1; i++) {
        QPointF sp = QPointF(offsetX, renderMaxHeight * listData->at(i) / maxVlaue);;
        QPointF ep = QPointF(offsetX + distance, renderMaxHeight * listData->at(i + 1) / maxVlaue);;

        offsetX += distance;

        QPointF c1 = QPointF((sp.x() + ep.x()) / 2.0, sp.y());
        QPointF c2 = QPointF((sp.x() + ep.x()) / 2.0, ep.y());
        path.cubicTo(c1, c2, ep);
    }
}

void CompactNetworkMonitor::updateStatus()
{
    auto netInfo = DeviceDB::instance()->netInfo();

    m_totalRecvBytes = netInfo->totalRecvBytes();
    m_totalSentBytes = netInfo->totalSentBytes();
    m_recvBps = netInfo->recvBps();
    m_sentBps = netInfo->sentBps();

    // Init download path.
    downloadSpeeds->append(m_recvBps);

    if (downloadSpeeds->size() > pointsNumber + 1) {
        downloadSpeeds->pop_front();
    }
    double downloadMaxHeight = *std::max_element(downloadSpeeds->begin(), downloadSpeeds->end()) * 1.1;

    // Init upload path.
    uploadSpeeds->append(m_sentBps);

    if (uploadSpeeds->size() > pointsNumber + 1) {
        uploadSpeeds->pop_front();
    }
    double uploadMaxHeight = *std::max_element(uploadSpeeds->begin(), uploadSpeeds->end()) * 1.1;

    double maxHeight = qMax(downloadMaxHeight, uploadMaxHeight);

    QPainterPath tmpDownloadpath;
    getPainterPathByData(downloadSpeeds, tmpDownloadpath, maxHeight);
    downloadPath = tmpDownloadpath;

    QPainterPath tmpUploadpath;
    getPainterPathByData(uploadSpeeds, tmpUploadpath, maxHeight);
    uploadPath = tmpUploadpath;

    update();
}

void CompactNetworkMonitor::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int bulletSize = 6;
    int padleft = bulletSize * 2 + 2;
    QRect contentRect(padleft, 0, rect().x() + rect().width() - padleft, 1);

    auto spacing = 10;
    auto margin = 10;

    // Draw network summary.
    QString recvTitle = DApplication::translate("Process.Graph.View", "Download");
    QString recvContent = formatUnit_net(m_recvBps * 8, B, 1, true);
    QString recvTotalTitle = DApplication::translate("Process.Graph.View", "Total Received");
    QString recvTotalContent = formatUnit_net(m_totalRecvBytes * 8, B, 1);

    QString sentTitle = DApplication::translate("Process.Graph.View", "Upload");
    QString sentContent = formatUnit_net(m_sentBps * 8, B, 1, true);
    QString sentTotalTitle = DApplication::translate("Process.Graph.View", "Total Sent");
    QString sentTotalContent = formatUnit_net(m_totalSentBytes * 8, B, 1);

    QFontMetrics fmContent(m_contentFont);
    QFontMetrics fmSubContent(m_subContentFont);
    QRect crect11(contentRect.x(), contentRect.y(), qCeil((contentRect.width() - spacing) / 2),
                  fmContent.height());
    QRect crect12(qCeil((contentRect.width() - spacing) / 2) + spacing, crect11.y(),
                  contentRect.width() - crect11.width() - spacing, crect11.height());
    QRect crect21(contentRect.x(), crect11.y() + crect11.height(), crect11.width(),
                  fmSubContent.height());
    QRect crect22(crect12.x(), crect21.y(), crect12.width(), crect21.height());
    QRect crect31(contentRect.x(), crect21.y() + crect21.height() + spacing, crect11.width(),
                  crect11.height());
    QRect crect32(crect12.x(), crect31.y(), crect12.width(), crect11.height());
    QRect crect41(contentRect.x(), crect31.y() + crect31.height(), crect11.width(),
                  crect21.height());
    QRect crect42(crect12.x(), crect41.y(), crect12.width(), crect21.height());
    QRectF r1Ind(0, crect11.y() + qCeil((crect11.height() - bulletSize) / 2.), bulletSize,
                 bulletSize);
    QRectF r2Ind(0, crect31.y() + qCeil((crect31.height() - bulletSize) / 2.), bulletSize,
                 bulletSize);

    painter.setPen(textColor);
    painter.setFont(m_contentFont);
    painter.drawText(crect11, Qt::AlignLeft | Qt::AlignVCenter,
                     fmContent.elidedText(recvTitle, Qt::ElideRight, crect11.width()));
    painter.drawText(crect31, Qt::AlignLeft | Qt::AlignVCenter,
                     fmContent.elidedText(sentTitle, Qt::ElideRight, crect31.width()));
    painter.drawText(crect12, Qt::AlignLeft | Qt::AlignVCenter,
                     fmContent.elidedText(recvTotalTitle, Qt::ElideRight, crect12.width()));
    painter.drawText(crect32, Qt::AlignLeft | Qt::AlignVCenter,
                     fmContent.elidedText(sentTotalTitle, Qt::ElideRight, crect32.width()));

    painter.setPen(summaryColor);
    painter.setFont(m_subContentFont);
    painter.drawText(crect21, Qt::AlignLeft | Qt::AlignVCenter,
                     fmSubContent.elidedText(recvContent, Qt::ElideRight, crect21.width()));
    painter.drawText(crect41, Qt::AlignLeft | Qt::AlignVCenter,
                     fmSubContent.elidedText(sentContent, Qt::ElideRight, crect41.width()));
    painter.drawText(crect22, Qt::AlignLeft | Qt::AlignVCenter,
                     fmSubContent.elidedText(recvTotalContent, Qt::ElideRight, crect22.width()));
    painter.drawText(crect42, Qt::AlignLeft | Qt::AlignVCenter,
                     fmSubContent.elidedText(sentTotalContent, Qt::ElideRight, crect42.width()));

    QPainterPath path1, path2;
    path1.addEllipse(r1Ind);
    path2.addEllipse(r2Ind);

    painter.fillPath(path1, m_recvIndicatorColor);
    painter.fillPath(path2, m_sentIndicatorColor);

    // Draw background grid.
    painter.setRenderHint(QPainter::Antialiasing, false);
    QPen framePen;
    int penSize = 1;
    framePen.setColor(m_frameColor);
    framePen.setWidth(penSize);
    painter.setPen(framePen);

    int gridX = rect().x() + penSize;
    int gridY = rect().y() + crect42.bottom() + margin;
    int gridWidth = this->width() - 2 * penSize;
    int gridHeight = renderMaxHeight + renderMaxHeight + 4 * penSize;

    QPainterPath framePath;
    QRect chartRect(gridX, gridY, gridWidth, gridHeight);
    framePath.addRect(chartRect);
    painter.drawPath(framePath);

    // Draw grid.
    QPen gridPen;
    QVector<qreal> dashes;
    qreal space = 2;
    dashes << 2 << space;
    gridPen.setColor(m_frameColor);
    //set to 0 lead to line with always 1px
    gridPen.setWidth(0);
    gridPen.setDashPattern(dashes);
    painter.setPen(gridPen);

    int gridLineX = gridX;
    while (gridLineX + gridSize + penSize < gridX + gridWidth) {
        gridLineX += gridSize + penSize;
        painter.drawLine(gridLineX, gridY + 1, gridLineX, gridY + gridHeight - 1);
    }
    int gridLineY = gridY;
    while (gridLineY + gridSize + penSize < gridY + gridHeight) {
        gridLineY += gridSize + penSize;
        painter.drawLine(gridX + 1, gridLineY, gridX + gridWidth - 1, gridLineY);
    }

    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath clip;
    clip.addRect(chartRect);
    painter.setClipPath(clip);

    qreal networkCurveWidth = 1.2;

    QBrush recvBrush(recvColor), sentBrush(sentColor);

    painter.translate(chartRect.x() + 2, chartRect.y() + chartRect.height() / 2 - 2);
    painter.scale(1, -1);
    painter.setPen(QPen(recvBrush, networkCurveWidth));
    painter.drawPath(downloadPath);

    painter.translate(0, -5);
    painter.scale(1, -1);
    painter.setPen(QPen(sentBrush, networkCurveWidth));
    painter.drawPath(uploadPath);

    setFixedHeight(chartRect.bottom() + 2 * penSize);
}

void CompactNetworkMonitor::changeTheme(DGuiApplicationHelper::ColorType themeType)
{
    Q_UNUSED(themeType);

    // init colors
    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();
#ifndef THEME_FALLBACK_COLOR
    textColor = palette.color(DPalette::TextTitle);
#else
    textColor = palette.color(DPalette::Text);
#endif

    summaryColor = palette.color(DPalette::TextTips);
    m_frameColor = palette.color(DPalette::TextTips);
    m_frameColor.setAlphaF(0.3);
}

void CompactNetworkMonitor::changeFont(const QFont &font)
{
    m_contentFont = font;
    m_contentFont.setWeight(QFont::Medium);
    m_contentFont.setPointSizeF(m_contentFont.pointSizeF() - 1);
    m_subContentFont = font;
    m_subContentFont.setPointSizeF(m_subContentFont.pointSizeF() - 1);
}

void CompactNetworkMonitor::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        emit clicked("MSG_NET");
}

void CompactNetworkMonitor::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return;
}
