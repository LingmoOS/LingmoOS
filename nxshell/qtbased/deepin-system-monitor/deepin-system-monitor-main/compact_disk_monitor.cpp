// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compact_disk_monitor.h"

#include "common/common.h"
#include "system/system_monitor.h"
#include "system/device_db.h"
#include "system/diskio_info.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>

#include <QDebug>
#include <QtMath>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE
using namespace core::system;
using namespace common;
using namespace common::format;

const int gridSize = 10;
const int pointsNumber = 30;
CompactDiskMonitor::CompactDiskMonitor(QWidget *parent)
    : QWidget(parent)
{
    // TODO: use more elegent way to set width
    int statusBarMaxWidth = common::getStatusBarMaxWidth();
    setFixedWidth(statusBarMaxWidth);
    setFixedHeight(160);

    readSpeeds = new QList<qreal>();
    for (int i = 0; i <= pointsNumber; i++) {
        readSpeeds->append(0);
    }

    writeSpeeds = new QList<qreal>();
    for (int i = 0; i <= pointsNumber; i++) {
        writeSpeeds->append(0);
    }

    connect(SystemMonitor::instance(), &SystemMonitor::statInfoUpdated, this, &CompactDiskMonitor::updateStatus);

    changeFont(DApplication::font());
    connect(dynamic_cast<QGuiApplication *>(DApplication::instance()), &DApplication::fontChanged,
            this, &CompactDiskMonitor::changeFont);
}

CompactDiskMonitor::~CompactDiskMonitor()
{
    delete readSpeeds;
    delete writeSpeeds;
}

void CompactDiskMonitor::updateStatus()
{
    m_readBps = DeviceDB::instance()->diskIoInfo()->diskIoReadBps();
    m_writeBps = DeviceDB::instance()->diskIoInfo()->diskIoWriteBps();

    // Init read path.
    readSpeeds->append(m_readBps);

    if (readSpeeds->size() > pointsNumber + 1) {
        readSpeeds->pop_front();
    }
    double readMaxHeight = *std::max_element(readSpeeds->begin(), readSpeeds->end()) * 1.1;

    writeSpeeds->append(m_writeBps);

    if (writeSpeeds->size() > pointsNumber + 1) {
        writeSpeeds->pop_front();
    }
    double writeMaxHeight = *std::max_element(writeSpeeds->begin(), writeSpeeds->end()) * 1.1;

    double maxHeight = qMax(readMaxHeight, writeMaxHeight);

    QPainterPath tmpReadpath;
    getPainterPathByData(readSpeeds, tmpReadpath, maxHeight);
    readPath = tmpReadpath;

    QPainterPath tmpWritepath;
    getPainterPathByData(writeSpeeds, tmpWritepath, maxHeight);
    writePath = tmpWritepath;

    update();
}

void CompactDiskMonitor::getPainterPathByData(QList<double> *listData, QPainterPath &path, qreal maxVlaue)
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

void CompactDiskMonitor::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int margin = 10;

    // init colors
    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();
#ifndef THEME_FALLBACK_COLOR
    QColor tagColor = palette.color(DPalette::TextTitle);
#else
    QColor tagColor = palette.color(DPalette::Text);
#endif

    QColor statColor = palette.color(DPalette::TextTips);
    QColor frameColor  = palette.color(DPalette::TextTips);
    frameColor.setAlphaF(0.3);

    // Draw disk summary.
    QFontMetrics fm(m_tagFont);
    QFontMetrics fmStat(m_statFont);

    QString rtag = DApplication::translate("Process.Graph.View", "Disk read");
    QString wtag = DApplication::translate("Process.Graph.View", "Disk write");
    QString rstat = QString("%1").arg(formatUnit_memory_disk(m_readBps, B, 1, true));
    QString wstat = QString("%1").arg(formatUnit_memory_disk(m_writeBps, B, 1, true));

    auto rtagWidth = fm.size(Qt::TextSingleLine, rtag).width();
    auto wtagWidth = fm.size(Qt::TextSingleLine, wtag).width();
    auto maxTagWidth = std::max(rtagWidth, wtagWidth);
    maxTagWidth = std::min(maxTagWidth, int(width() * 0.75));
    QRect rcol1(m_bulletSize * 2 + 2, 0, maxTagWidth, fm.height());
    QRect rcol2(rcol1.x() + rcol1.width() + margin, rcol1.y(),
                fmStat.size(Qt::TextSingleLine, rstat).width(), fmStat.height());
    QRect wcol1(rcol1.x(), rcol1.y() + rcol1.height(), maxTagWidth, fm.height());
    QRect wcol2(rcol2.x(), wcol1.y(), fmStat.size(Qt::TextSingleLine, wstat).width(), fmStat.height());
    QRect bulletRect1(0, rcol1.y() + qCeil((rcol1.height() - m_bulletSize) / 2.), m_bulletSize,
                      m_bulletSize);
    QRect bulletRect2(0, wcol1.y() + qCeil((wcol1.height() - m_bulletSize) / 2.), m_bulletSize,
                      m_bulletSize);

    QPainterPath path1, path2;
    path1.addEllipse(bulletRect1);
    path2.addEllipse(bulletRect2);
    painter.fillPath(path1, m_diskReadColor);
    painter.fillPath(path2, m_diskWriteColor);

    painter.setPen(QPen(tagColor));
    painter.setFont(m_tagFont);
    painter.drawText(rcol1, Qt::AlignLeft | Qt::AlignVCenter, fm.elidedText(rtag, Qt::ElideRight, maxTagWidth));
    painter.drawText(wcol1, Qt::AlignLeft | Qt::AlignVCenter, fm.elidedText(wtag, Qt::ElideRight, maxTagWidth));

    painter.setPen(QPen(statColor));
    painter.setFont(m_statFont);
    painter.drawText(rcol2, Qt::AlignLeft | Qt::AlignVCenter, rstat);
    painter.drawText(wcol2, Qt::AlignLeft | Qt::AlignVCenter, wstat);

    // Draw background grid.
    painter.setRenderHint(QPainter::Antialiasing, false);

    QPen framePen;
    int penSize = 1;
    framePen.setColor(frameColor);
    framePen.setWidth(penSize);
    painter.setPen(framePen);

    int gridX = rect().x() + penSize;
    int gridY = rect().y() + wcol1.y() + wcol1.height() + margin;
    int gridWidth = this->width() - 2 * penSize;
    int gridHeight = renderMaxHeight + renderMaxHeight + 4 * penSize;

    QPainterPath framePath;
    QRect gridFrame(gridX, gridY, gridWidth, gridHeight);
    framePath.addRect(gridFrame);
    painter.drawPath(framePath);

    // Draw grid.
    QPen gridPen;
    QVector<qreal> dashes;
    qreal space = 2;
    dashes << 2 << space;
    gridPen.setColor(frameColor);
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
    clip.addRect(gridFrame);
    painter.setClipPath(clip);

    qreal diskCurveWidth = 1.2;

    painter.translate(gridFrame.x() + 2, gridFrame.y() + gridFrame.height() / 2 - 2);
    painter.scale(1, -1);
    painter.setPen(QPen(m_diskReadColor, diskCurveWidth));
    painter.drawPath(readPath);

    painter.translate(0, -5);
    painter.scale(1, -1);
    painter.setPen(QPen(m_diskWriteColor, diskCurveWidth));
    painter.drawPath(writePath);

    setFixedHeight(gridFrame.bottom() + 2 * penSize);
}

void CompactDiskMonitor::changeFont(const QFont &font)
{
    m_tagFont = font;
    m_tagFont.setPointSizeF(m_tagFont.pointSizeF() - 1);
    m_tagFont.setWeight(QFont::Medium);
    m_statFont = font;
    m_statFont.setPointSizeF(m_statFont.pointSizeF() - 1);
}

void CompactDiskMonitor::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        emit clicked("MSG_DISK");
}

void CompactDiskMonitor::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return;
}
