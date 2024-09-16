// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compact_cpu_monitor.h"

#include "smooth_curve_generator.h"
#include "common/common.h"
#include "process/process_db.h"
#include "system/device_db.h"
#include "model/cpu_info_model.h"
#include "model/cpu_list_model.h"
#include "gui/base/base_commandlink_button.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyleHelper>
#include <DFontSizeManager>

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE
using namespace common;

// additonPoses to align with the cpu text
const int additionCPUPosX = 3;
const int additionCPUPoxY = 3;

// 通过 UseTotalCpuCurve 宏, 控制绘制CPU曲线方式
// UseTotalCpuCurve: true , 绘制一条总的CPU占用曲线
// UseTotalCpuCurve: false, 绘制每个独立CPU的占用曲线
#define UseTotalCpuCurve true

CompactCpuMonitor::CompactCpuMonitor(QWidget *parent)
    : QWidget(parent)
{
    int statusBarMaxWidth = common::getStatusBarMaxWidth();
    setFixedWidth(statusBarMaxWidth);
    setFixedHeight(160);

    pointsNumber = int(statusBarMaxWidth / 10);

    numCPU = int(sysconf(_SC_NPROCESSORS_ONLN));

    // 使用 cpuPercents[numCPU] 位置存储总的CPU占用率曲线数据
    // [0, numCPU-1] 存储各独立子CPU曲线数据
    for (int i = 0; i <= numCPU; i++) {
        QList<qreal> cpuPercent;
        for (int j = 0; j < pointsNumber; j++) {
            cpuPercent.append(0);
        }

        cpuPercents.append(cpuPercent);
    }

    cpuColors << "#1094D8"
              << "#F7B300"
              << "#55D500"
              << "#C362FF"
              << "#FF2997"
              << "#00B4C7"
              << "#F8E71C"
              << "#FB1818"
              << "#8544FF"
              << "#00D7AB"
              << "#00D7AB"
              << "#FF00FF"
              << "#30BF03"
              << "#7E41F1"
              << "#2CA7F8"
              << "#A005CE";

    m_cpuInfomodel = CPUInfoModel::instance();
    connect(m_cpuInfomodel, &CPUInfoModel::modelUpdated, this, &CompactCpuMonitor::updateStatus);

    m_detailText = tr("Details");
    m_detailButton = new BaseCommandLinkButton(m_detailText, this);
    DFontSizeManager::instance()->bind(m_detailButton, DFontSizeManager::T8, QFont::Medium);
    m_detailButton->setToolTip(m_detailText);
    connect(m_detailButton, &BaseCommandLinkButton::clicked, this, &CompactCpuMonitor::onDetailInfoClicked);

    changeFont(DApplication::font());
    connect(dynamic_cast<QGuiApplication *>(DApplication::instance()), &DApplication::fontChanged,
            this, &CompactCpuMonitor::changeFont);
}

CompactCpuMonitor::~CompactCpuMonitor() {}

void CompactCpuMonitor::onDetailInfoClicked()
{
    setDetailButtonVisible(false);
    emit signalDetailInfoClicked();
}

void CompactCpuMonitor::updateStatus()
{
    totalCpuPercent = m_cpuInfomodel->cpuAllPercent();

    // 增加判断，如果返回的值是非数，则不更新界面，也不保存数据
    if (std::isnan(totalCpuPercent))
        return;

    // 将cpuPercents 和传过来最新的cPercents做对比，arm开核关核时，对应的系统监视器会出现飞线或波形图不动的情况
    QList<int> appendIndex;
    const auto &cpulist = m_cpuInfomodel->cpuPercentList();

    // 更新追加，各个独立CPU占用曲线数据
    if(cpulist.size() < cpuPercents.size() - 1) {
        for(int i = cpulist.size(); i < cpuPercents.size() - 1; i++) {
            cpuPercents[i].append(0.0);
            appendIndex.append(i);
        }
    } else if (cpulist.size() > cpuPercents.size() - 1){
        QList<qreal> addPointList;
        for (int j = 0; j < pointsNumber; j++) {
            addPointList.append(0);
        }
        for(int i = cpuPercents.size() - 1; i < cpulist.size(); i++) {
            cpuPercents.append(addPointList);
            appendIndex.append(i);
        }
    } else {
        appendIndex.append(-1);
    }

    for (int i = 0; i < cpulist.size(); i++) {
        //防止QList内存溢出
        if (!cpuPercents[i].isEmpty()) {
            QList<qreal> cpuPercent = cpuPercents[i];
            if (!appendIndex.contains(i)) {
                if (!std::isnan(cpulist[i]))
                    cpuPercent.append(cpulist[i] / 100.0);
            }

            if (cpuPercent.size() > pointsNumber) {
                cpuPercent.pop_front();
            }

            cpuPercents[i] = cpuPercent;
        }
    }

    // 更新追加，总的CPU占用率数据
    QList<qreal> calcTotalCpuPercent = cpuPercents[numCPU];

    calcTotalCpuPercent.append(totalCpuPercent / 100.0);

    if (calcTotalCpuPercent.size() > pointsNumber) {
        calcTotalCpuPercent.pop_front();
    }

    cpuPercents[numCPU] = calcTotalCpuPercent;

    update();
}

void CompactCpuMonitor::setDetailButtonVisible(bool visible)
{
    m_detailButton->setVisible(visible);
}

void CompactCpuMonitor::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void CompactCpuMonitor::resizeItemRect()
{
    m_detailButton->setFixedSize(m_detailButton->fontMetrics().width(m_detailButton->text()) + 12, m_detailButton->fontMetrics().height() + 4);
    const QSize &detailtextSize = m_detailButton->size();
    m_detailButton->setGeometry(this->width() - detailtextSize.width() + additionCPUPosX, additionCPUPoxY, detailtextSize.width(), detailtextSize.height());
}

void CompactCpuMonitor::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    auto *dAppHelper = DApplicationHelper::instance();

    // init colors
    auto palette = dAppHelper->applicationPalette();
    QColor frameColor  = palette.color(DPalette::TextTips);
    frameColor.setAlphaF(0.3);

#ifndef THEME_FALLBACK_COLOR
    QColor cpuColor = palette.color(DPalette::TextTitle);
#else
    QColor cpuColor = palette.color(DPalette::Text);
#endif

    QColor statColor = palette.color(DPalette::TextTips);
    QColor sectionColor {"#0081FF"};

    int spacing = 4;

    // Draw cpu summary.
    QFontMetrics fm(m_cpuFont);
    QFontMetrics fmStat(m_statFont);

    QString cpuText = DApplication::translate("Process.Graph.View", "CPU");
    QString cpuStatText = QString::number(totalCpuPercent, 'f', 1).append('%');

    QRect cpuRect(pointerRadius * 2 + spacing - 2, 0, fm.width(cpuText), fm.height() + 4);
    QRect sectionRect(0, cpuRect.y() + qCeil((cpuRect.height() - pointerRadius) / 2.),
                      pointerRadius, pointerRadius);
    QRect statRect(cpuRect.x() + cpuRect.width() + spacing, cpuRect.y(), fmStat.width(cpuStatText),
                   fmStat.height() + 4);

    m_detailButton->setText(m_detailButton->fontMetrics().elidedText(m_detailText, Qt::ElideRight, this->width() - statRect.right() - 2 * spacing));
    resizeItemRect();

    // draw section
    painter.setPen(sectionColor);
    QPainterPath path;
    path.addRoundedRect(sectionRect, pointerRadius, pointerRadius);
    painter.fillPath(path, sectionColor);

    // draw cpu title
    painter.setPen(cpuColor);
    painter.setFont(m_cpuFont);
    painter.drawText(cpuRect, Qt::AlignLeft | Qt::AlignVCenter, cpuText);

    // draw cpu stat
    painter.setPen(statColor);
    painter.setFont(m_statFont);
    painter.drawText(statRect, Qt::AlignLeft | Qt::AlignVCenter, cpuStatText);

    // draw grid
    QPen framePen;
    int penSize = 1;
    framePen.setColor(frameColor);
    framePen.setWidth(penSize);
    painter.setPen(framePen);

    int gridX = rect().x() + penSize;
    int gridY = cpuRect.y() + cpuRect.height() + 10;
    int gridWidth =
        rect().width() - 3 - ((rect().width() - 3 - penSize) % (gridSize + penSize)) - penSize;
    int gridHeight = cpuRenderMaxHeight + 8 * penSize;

    painter.setRenderHint(QPainter::Antialiasing, false);
    QPainterPath framePath;
    QRect gridFrame(gridX, gridY, gridWidth, gridHeight);
    framePath.addRect(gridFrame);
    painter.drawPath(framePath);

    // Draw grid.
    QPen gridPen;
    QVector<qreal> dashes;
    qreal space = 2;
    dashes << 2 << space;
    gridPen.setDashPattern(dashes);
    gridPen.setColor(frameColor);
    //set to 0 lead to line with always 1px
    gridPen.setWidth(0);
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

    // clip internal area of the region
    QPainterPath clip;
    clip.addRect(gridFrame);
    painter.setClipPath(clip);

    painter.translate(gridFrame.x(), gridFrame.y());

    qreal strokeWidth = 1.2;                            // for now, set as 1.2 temporarily
    int drawWidth = gridFrame.width() - penSize * 2;    // exclude left/right most border
    int drawHeight = gridFrame.height() - penSize * 2;  // exclude top/bottom most border
    qreal offsetX = drawWidth + penSize;
    qreal deltaX = drawWidth * 1.0 / (pointsNumber - 3);

// 通过 UseTotalCpuCurve 宏控制绘制哪些CPU曲线
// 通过 statrpos 和 endpos 实现曲线的选择
#if UseTotalCpuCurve
    int statrpos = numCPU;
    int endpos = statrpos;
#else
    int statrpos = cpuPercents.size() - 2;
    int endpos = 0;
#endif

    // enum cpu
    for (int i = statrpos; i >= endpos && i >= 0; i--) {
        // set stroke color
        QColor c = cpuColors[i % cpuColors.size()];
        painter.setPen(QPen(c, strokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        QPainterPath Painterpath;
        // move to the first point (from right to left)
        qreal y = (1.0 - cpuPercents[i].at(pointsNumber - 1)) * drawHeight + penSize;
        Painterpath.moveTo(offsetX, y);

        for (int j = pointsNumber - 2; j >= 0; j--) {
            // method#1: draw Bezier curve
            Painterpath.cubicTo(offsetX - (pointsNumber - j - 1 - 0.5) * deltaX,
                                (1.0 - cpuPercents[i][j + 1]) * drawHeight + penSize + 0.5,
                                offsetX - (pointsNumber - j - 1 - 0.5) * deltaX,
                                (1.0 - cpuPercents[i][j]) * drawHeight + penSize + 0.5,
                                offsetX - ((pointsNumber - j - 1) * deltaX),
                                (1.0 - cpuPercents[i][j]) * drawHeight + penSize + 0.5);

            // method#2: draw line instead
            // path.lineTo(offsetX - ((pointsNumber - j - 1) * deltaX),
            //            (1.0 - cpuPercents[i][j]) * drawHeight + penSize + 0.5);
        }
        painter.drawPath(Painterpath);
    }

    setFixedHeight(gridFrame.y() + gridFrame.height() + penSize);
}

void CompactCpuMonitor::changeFont(const QFont &font)
{
    m_cpuFont = font;
    m_cpuFont.setWeight(QFont::Medium);
    m_cpuFont.setPointSizeF(m_cpuFont.pointSizeF() - 1);
    m_statFont = font;
    m_statFont.setPointSizeF(m_statFont.pointSizeF() - 1);

    resizeItemRect();
}

void CompactCpuMonitor::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        emit clicked("MSG_CPU");
}

void CompactCpuMonitor::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return;
}
