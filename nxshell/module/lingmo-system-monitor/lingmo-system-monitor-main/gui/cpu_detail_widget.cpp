// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cpu_detail_widget.h"
#include "model/cpu_info_model.h"
#include "smooth_curve_generator.h"
#include "common/common.h"
#include "process/process_db.h"
#include "system/device_db.h"
#include "model/cpu_info_model.h"
#include "model/cpu_list_model.h"
#include "system/cpu_set.h"
#include "cpu_summary_view_widget.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyleHelper>
#include <DLabel>
#include <DFontSizeManager>

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QMouseEvent>
#include <QGridLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollArea>
#include <QPaintEvent>

DWIDGET_USE_NAMESPACE

using namespace common;

CPUDetailGrapTableItem::CPUDetailGrapTableItem(CPUInfoModel *model, int index, QWidget *parent): QWidget(parent), m_cpuInfomodel(model), m_index(index)
{
    m_cpuInfomodel = CPUInfoModel::instance();
    connect(m_cpuInfomodel, &CPUInfoModel::modelUpdated, this, &CPUDetailGrapTableItem::updateStat);
}

CPUDetailGrapTableItem::~CPUDetailGrapTableItem()
{

}

void CPUDetailGrapTableItem::setMode(int mode)
{
    m_mode = mode;
    setToolTip(3 == m_mode ? ("CPU" + QString::number(m_index)) : "");
    update();
}

void CPUDetailGrapTableItem::sethorizontal(bool isHorizontalLast)
{
    m_isHorizontalLast = isHorizontalLast;
}

void CPUDetailGrapTableItem::setVerticalLast(bool isVerticalLast)
{
    m_isVerticalLast = isVerticalLast;
}

void CPUDetailGrapTableItem::setColor(QColor color)
{
    m_color = color;
    update();
}

void CPUDetailGrapTableItem::updateStat()
{
    // 多核模式时保留原来逻辑
    if (m_isMutliCoreMode) {
        if (std::isnan(m_cpuInfomodel->cpuPercentList().value(m_index)))
            m_cpuPercents.insert(0, 0);
        else
            m_cpuPercents.insert(0, m_cpuInfomodel->cpuPercentList().value(m_index) / 100.0);
    } else {
        if (std::isnan(m_cpuInfomodel->cpuAllPercent()))
            m_cpuPercents.insert(0, 0);
        else
            m_cpuPercents.insert(0, m_cpuInfomodel->cpuAllPercent() / 100.0);
    }
    while (m_cpuPercents.count() > 31)
        m_cpuPercents.pop_back();

    update();
}

void CPUDetailGrapTableItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);

    if (1 == m_mode)
        drawNormalMode(painter);
    else if (2 == m_mode)
        drawSimpleMode(painter);
    else if (3 == m_mode)
        drawTextMode(painter);
}

void CPUDetailGrapTableItem::drawNormalMode(QPainter &painter)
{
    const int pensize = 1;
    QFont font = DApplication::font();
    font.setPointSizeF(font.pointSizeF() - 1);
    painter.setFont(font);

    QFont midFont = font;
    midFont.setPointSizeF(font.pointSizeF() - 1);

    int textHeight = painter.fontMetrics().height();
    //draw background
    QRect graphicRect = QRect(pensize, textHeight, this->width() - 2 * pensize, this->height() - textHeight - QFontMetrics(midFont).height());
    drawBackground(painter, graphicRect);

    //draw text
    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();
    painter.setPen(palette.color(DPalette::TextTips));
    painter.setRenderHints(QPainter::Antialiasing);

    if (m_isMutliCoreMode) {
        painter.drawText(QRect(pensize, 0, this->width() - 2 * pensize, textHeight), Qt::AlignLeft | Qt::AlignTop, "CPU" + QString::number(m_index));
    } else {
        painter.drawText(QRect(pensize, 0, this->width() - 2 * pensize, textHeight), Qt::AlignLeft | Qt::AlignTop, "CPU");
    }


    painter.setFont(midFont);
    int midTextHeight = painter.fontMetrics().height();

    painter.save();
    QColor midTextColor(palette.color(DPalette::ToolTipText));
    midTextColor.setAlphaF(0.3);
    QPen midTextPen = painter.pen();
    midTextPen.setColor(midTextColor);
    painter.setPen(midTextPen);
    painter.drawText(QRect(pensize, 0, this->width() - 2 * pensize, textHeight), Qt::AlignRight | Qt::AlignBottom, "100%");
    painter.drawText(QRect(pensize, graphicRect.bottom() + pensize, this->width() - 2 * pensize, midTextHeight), Qt::AlignLeft | Qt::AlignVCenter, tr("60 seconds"));
    painter.drawText(QRect(pensize, graphicRect.bottom() + pensize, this->width() - 2 * pensize, midTextHeight), Qt::AlignRight | Qt::AlignVCenter, "0");
    painter.restore();

    // draw cpu
    painter.setClipRect(graphicRect);
    if (m_cpuPercents.count() > 0) {
        painter.setPen(QPen(m_color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);

        QPainterPath Painterpath;

        QPointF sp = QPointF(graphicRect.width() + graphicRect.x(), (1.0 - m_cpuPercents.value(0)) * graphicRect.height() + graphicRect.y());
        Painterpath.moveTo(sp);

        for (int i = 0; i < 30; ++i) {
            if (m_cpuPercents.count() > i) {
                QPointF ep = QPointF((graphicRect.width() - static_cast<double>(graphicRect.width()) / (30.0 / static_cast<double>(i + 1))) + graphicRect.x(), (1.0 - m_cpuPercents.value(i + 1)) * graphicRect.height() + graphicRect.y());
                QPointF c1 = QPointF((sp.x() + ep.x()) / 2, sp.y());
                QPointF c2 = QPointF((sp.x() + ep.x()) / 2, ep.y());
                Painterpath.cubicTo(c1, c2, ep);
                sp = ep;
            }
        }

        painter.drawPath(Painterpath);
    }
}

void CPUDetailGrapTableItem::drawSimpleMode(QPainter &painter)
{
    //draw background
    const int pensize = 1;
    QRect graphicRect = QRect(pensize, pensize, this->width() - 2 * pensize, this->height() - 2 * pensize);
    drawBackground(painter, graphicRect);

    // draw cpu
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(graphicRect);

    if (m_cpuPercents.count() > 0) {
        painter.setPen(QPen(m_color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);

        QPainterPath Painterpath;

        QPointF sp = QPointF(graphicRect.width() + graphicRect.x(), (1.0 - m_cpuPercents.value(0)) * graphicRect.height() + graphicRect.y());
        Painterpath.moveTo(sp);

        for (int i = 0; i < 30; ++i) {
            if (m_cpuPercents.count() > i) {
                QPointF ep = QPointF((graphicRect.width() - static_cast<double>(graphicRect.width()) / (30.0 / static_cast<double>(i + 1))) + graphicRect.x(), (1.0 - m_cpuPercents.value(i + 1)) * graphicRect.height() + graphicRect.y());
                QPointF c1 = QPointF((sp.x() + ep.x()) / 2, sp.y());
                QPointF c2 = QPointF((sp.x() + ep.x()) / 2, ep.y());
                Painterpath.cubicTo(c1, c2, ep);
                sp = ep;
            }
        }

        painter.drawPath(Painterpath);
    }
}

void CPUDetailGrapTableItem::drawTextMode(QPainter &painter)
{
    QRect rect = QRect(0, 0, this->width() - (m_isHorizontalLast ? 1 : 0), this->height() - (m_isVerticalLast ? 1 : 0));

    // draw frame
    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();
    QColor frameColor = palette.color(DPalette::FrameBorder);
    painter.setPen(frameColor);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect);

    painter.setPen(m_color);
    painter.drawText(rect, Qt::AlignCenter, QString::number(m_cpuPercents.value(m_index) * 100, 'f', 1) + "%");
}

void CPUDetailGrapTableItem::drawSingleCoreMode(QPainter &painter)
{
    const int pensize = 1;
    QFont font = DApplication::font();
    font.setPointSizeF(font.pointSizeF() - 1);
    painter.setFont(font);

    QFont midFont = font;
    midFont.setPointSizeF(font.pointSizeF() - 1);

    int textHeight = painter.fontMetrics().height();
    //draw background
    QRect graphicRect = QRect(pensize, textHeight, this->width() - 2 * pensize, this->height() - textHeight - QFontMetrics(midFont).height());
    drawBackground(painter, graphicRect);

    //draw text
    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();
    painter.setPen(palette.color(DPalette::TextTips));
    painter.setRenderHints(QPainter::Antialiasing);

    painter.drawText(QRect(pensize, 0, this->width() - 2 * pensize, textHeight), Qt::AlignLeft | Qt::AlignTop, "CPU" + QString::number(m_index));

    painter.setFont(midFont);
    int midTextHeight = painter.fontMetrics().height();

    painter.drawText(QRect(pensize, 0, this->width() - 2 * pensize, textHeight), Qt::AlignRight | Qt::AlignBottom, "100%");
    painter.drawText(QRect(pensize, graphicRect.bottom() + pensize, this->width() - 2 * pensize, midTextHeight), Qt::AlignLeft | Qt::AlignVCenter, tr("60 seconds"));
    painter.drawText(QRect(pensize, graphicRect.bottom() + pensize, this->width() - 2 * pensize, midTextHeight), Qt::AlignRight | Qt::AlignVCenter, "0");

    // draw cpu
    painter.setClipRect(graphicRect);
    if (m_cpuPercents.count() > 0) {
        painter.setPen(QPen(m_color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);

        QPainterPath Painterpath;

        QPointF sp = QPointF(graphicRect.width() + graphicRect.x(), (1.0 - m_cpuPercents.value(0)) * graphicRect.height() + graphicRect.y());
        Painterpath.moveTo(sp);

        for (int i = 0; i < 30; ++i) {
            if (m_cpuPercents.count() > 0) {
                QPointF ep = QPointF((graphicRect.width() - static_cast<double>(graphicRect.width()) / (30.0 / static_cast<double>(1))) + graphicRect.x(), (1.0 - m_cpuPercents.value(1)) * graphicRect.height() + graphicRect.y());
                QPointF c1 = QPointF((sp.x() + ep.x()) / 2, sp.y());
                QPointF c2 = QPointF((sp.x() + ep.x()) / 2, ep.y());
                Painterpath.cubicTo(c1, c2, ep);
                sp = ep;
            }
        }

        painter.drawPath(Painterpath);
    }
}

void CPUDetailGrapTableItem::drawBackground(QPainter &painter, const QRect &graphicRect)
{
    // draw frame
    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();
    QColor frameColor = palette.color(DPalette::TextTips);
    frameColor.setAlphaF(0.3);

    painter.setPen(QPen(frameColor, 1));
    painter.setBrush(palette.color(QPalette::Base));
    painter.drawRect(graphicRect);

    // draw grid
    QPen gridPen;
    QVector<qreal> dashes;
    qreal space = 2;
    dashes << space << space;
    gridPen.setDashPattern(dashes);
    gridPen.setColor(frameColor);
    //set to 0 lead to line with always 1px
    gridPen.setWidth(0);
    painter.setPen(gridPen);
    int section = 10;

    int totalHeight = graphicRect.height() - 2;
    int currentHeight = graphicRect.y() + section;
    while (currentHeight < totalHeight + graphicRect.y()) {
        painter.drawLine(graphicRect.x() + 1, currentHeight, graphicRect.x() + graphicRect.width() - 1, currentHeight);
        currentHeight += section;
    }

    int totalWidth = graphicRect.width() - 2;
    int currentWidth = graphicRect.x() + section;
    while (currentWidth < totalWidth + graphicRect.x()) {
        painter.drawLine(currentWidth, graphicRect.y() + 1, currentWidth, graphicRect.y() + graphicRect.height() - 1);
        currentWidth += section;
    }
}

CPUDetailWidget::CPUDetailWidget(QWidget *parent) : BaseDetailViewWidget(parent)
{
    this->setObjectName("CPUDetailWidget");

    TimePeriod period(TimePeriod::kNoPeriod, {2, 0});
    CPUInfoModel *cpuInfomodel = CPUInfoModel::instance();

    m_graphicsTable = new CPUDetailGrapTable(cpuInfomodel, this);
    m_summary  = new  CPUDetailSummaryTable(cpuInfomodel, this);

    m_centralLayout->addWidget(m_graphicsTable);
    m_centralLayout->addWidget(m_summary);

    setTitle(DApplication::translate("Process.Graph.View", "CPU"));
    setDetail(cpuInfomodel->cpuSet()->modelName());

    detailFontChanged(DApplication::font());

    connect(this, &CPUDetailWidget::sigClickSwitchMutliCoreButton, this, [ = ](bool isMutliCoreMode) {
        m_graphicsTable->setMutliCoreMode(isMutliCoreMode);
    });

    connect(dynamic_cast<QGuiApplication *>(DApplication::instance()), &DApplication::fontChanged,
            this, &CPUDetailWidget::detailFontChanged);
}

void CPUDetailWidget::detailFontChanged(const QFont &font)
{
    BaseDetailViewWidget::detailFontChanged(font);
    m_summary->fontChanged(font);
}

CPUDetailGrapTable::CPUDetailGrapTable(CPUInfoModel *model, QWidget *parent): QWidget(parent)
{
    if (m_isMutliCoreMode) {
        setMultiModeLayout(model);
    } else {
        setSingleModeLayout(model);
    }
    m_cpuInfoModel = model;
}

void CPUDetailGrapTable::setMutliCoreMode(bool isMutliCoreMode)
{
    m_isMutliCoreMode = isMutliCoreMode;
    // 获取当前布局所有的子控件，删除子控件
    QLayout *p = this->layout();
    while (p->count()) {
        QWidget *pWidget = p->itemAt(0)->widget();
        pWidget->setParent(nullptr);
        p->removeWidget(pWidget);
        delete pWidget;
    }
    this->layout()->removeItem(this->layout()->itemAt(0));
    delete this->layout();
    if (m_isMutliCoreMode) {
        setMultiModeLayout(m_cpuInfoModel);
    } else {
        setSingleModeLayout(m_cpuInfoModel);
    }
}

void CPUDetailGrapTable::setSingleModeLayout(CPUInfoModel *model)
{
    QGridLayout  *graphicsLayout = new QGridLayout(this);
    graphicsLayout->setMargin(0);
    graphicsLayout->setHorizontalSpacing(10);
    graphicsLayout->setVerticalSpacing(10);

    QColor cpuColor("#1094D8");

    CPUDetailGrapTableItem *item = new CPUDetailGrapTableItem(model, 0, this);
    item->setMode(1);
    item->setMultiCoreMode(false);
    item->setColor(cpuColor);
    graphicsLayout->addWidget(item, 0, 0);

    setLayout(graphicsLayout);
}

void CPUDetailGrapTable::setMultiModeLayout(CPUInfoModel *model)
{
    QGridLayout  *graphicsLayout = new QGridLayout(this);
    graphicsLayout->setMargin(0);
    graphicsLayout->setHorizontalSpacing(10);
    graphicsLayout->setVerticalSpacing(10);

    int cpuCount = int(sysconf(_SC_NPROCESSORS_ONLN));

    QList<QColor> cpuColors;
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

    if (1 == cpuCount) {
        CPUDetailGrapTableItem *item = new CPUDetailGrapTableItem(model, 0, this);
        item->setMode(1);
        item->setMultiCoreMode(false);
        item->setColor(cpuColors[0]);
        graphicsLayout->addWidget(item, 0, 0);
    } else if (2 == cpuCount) {
        for (int i = 0; i < cpuCount; ++i) {
            CPUDetailGrapTableItem *item = new CPUDetailGrapTableItem(model, i, this);
            item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            item->setMode(1);
            item->setMultiCoreMode(true);
            item->setColor(cpuColors[i % cpuColors.size()]);
            graphicsLayout->addWidget(item, i / 2, i % 2);
        }
    } else if (4 == cpuCount) {
        for (int i = 0; i < cpuCount; ++i) {
            CPUDetailGrapTableItem *item = new CPUDetailGrapTableItem(model, i, this);
            item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            item->setMode(1);
            item->setMultiCoreMode(true);
            item->setColor(cpuColors[i % cpuColors.size()]);
            graphicsLayout->addWidget(item, i / 2, i % 2);
        }

    } else if (8 == cpuCount) {
        for (int i = 0; i < cpuCount; ++i) {
            CPUDetailGrapTableItem *item = new CPUDetailGrapTableItem(model, i, this);
            item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            item->setMode(1);
            item->setMultiCoreMode(true);
            item->setColor(cpuColors[i % cpuColors.size()]);
            graphicsLayout->addWidget(item, i / 4, i % 4);
        }
    } else if (16 == cpuCount) {
        for (int i = 0; i < cpuCount; ++i) {
            CPUDetailGrapTableItem *item = new CPUDetailGrapTableItem(model, i, this);
            item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            item->setMode(2);
            item->setMultiCoreMode(true);
            item->setColor(cpuColors[i % cpuColors.size()]);
            graphicsLayout->addWidget(item, i / 4, i % 4);
        }
    } else if (32 == cpuCount) {//8*4
        for (int i = 0; i < cpuCount; ++i) {
            CPUDetailGrapTableItem *item = new CPUDetailGrapTableItem(model, i, this);
            item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            item->setMode(2);
            item->setMultiCoreMode(true);
            item->setColor(cpuColors[i % cpuColors.size()]);
            graphicsLayout->addWidget(item, i / 8, i % 8);
        }

        graphicsLayout->setHorizontalSpacing(6);
        graphicsLayout->setVerticalSpacing(6);
    } else if (32 < cpuCount) {
        for (int i = 0; i < cpuCount; ++i) {
            CPUDetailGrapTableItem *item = new CPUDetailGrapTableItem(model, i, this);
            item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            item->setMode(3);
            item->setMultiCoreMode(true);
            item->setColor(cpuColors[i % cpuColors.size()]);
            if ((i % 8) == 7)
                item->sethorizontal(true);
            if ((i / 8) == (cpuCount / 8 - 1))
                item->setVerticalLast(true);
            graphicsLayout->addWidget(item, i / 8, i % 8);
        }

        graphicsLayout->setHorizontalSpacing(0);
        graphicsLayout->setVerticalSpacing(0);
    } else {
        //模式2
        for (int i = 0; i < cpuCount; ++i) {
            CPUDetailGrapTableItem *item = new CPUDetailGrapTableItem(model, i, this);
            item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            item->setMode(2);
            item->setMultiCoreMode(true);
            item->setColor(cpuColors[i % cpuColors.size()]);
            graphicsLayout->addWidget(item, i / 16, i % 16);
        }
    }

    setLayout(graphicsLayout);
}
