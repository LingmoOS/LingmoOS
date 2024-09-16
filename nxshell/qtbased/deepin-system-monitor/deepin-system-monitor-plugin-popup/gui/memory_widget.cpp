// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ddlog.h"
#include "memory_widget.h"
#include "../common/utils.h"
#include "common/datacommon.h"
#include "datadealsingleton.h"
#include "dbus/dbuscallmaininterface.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyleHelper>

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QBrush>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QProcess>

DWIDGET_USE_NAMESPACE

using namespace Utils;
using namespace DDLog;
MemoryWidget::MemoryWidget(QWidget *parent)
    : QWidget(parent)
{
    m_width = parent->width() - 20;
    setFixedSize(m_width, 153);
    setContentsMargins(0, 0, 0, 0);

    auto *dAppHelper = DApplicationHelper::instance();
    connect(dAppHelper, &DApplicationHelper::themeTypeChanged, this, &MemoryWidget::changeTheme);
    changeTheme(dAppHelper->themeType());

    changeFont(DApplication::font());
    connect(dynamic_cast<QGuiApplication *>(DApplication::instance()), &DApplication::fontChanged,
            this, &MemoryWidget::changeFont);

    connect(&DataDealSingleton::getInstance(), &DataDealSingleton::sigDataUpdate, this, &MemoryWidget::updateStatus);

    installEventFilter(this);
}
MemoryWidget::~MemoryWidget() {}

void MemoryWidget::updateStatus()
{
    QString memUsage, memTotal, memPercent, swapUsage, swapTotal, swapPercent;
    if (!DataDealSingleton::getInstance().readMemInfo(memUsage, memTotal, memPercent, swapUsage, swapTotal, swapPercent))
        qCInfo(app) << "failed";
    //    qCInfo(app)<<"swapUsage: "<<swapUsage;
    //    qCInfo(app)<<"swapTotal: "<<swapTotal;

    m_memPercent = memPercent;
    m_swapPercent = swapPercent;

    QStringList strsMemUsage = memUsage.split(" ");
    if (strsMemUsage.size() == 2) {
        m_memUsage = strsMemUsage.at(0);
        m_memUsageUnit = strsMemUsage.at(1);
    } else
        return;

    m_memTotal = memTotal;
    //去除字符串空格 bug #111050
    m_memTotal = memTotal;
    QStringList memList = m_memTotal.split(' ');
    m_memTotal = "";
    for (auto str : memList) {
        m_memTotal += str;
    }

    QStringList strsSwapUsage = swapUsage.split(" ");
    if (strsSwapUsage.size() == 2) {
        m_swapUsage = strsSwapUsage.at(0);
        m_swapUnit = strsSwapUsage.at(1);
    } else
        return;

    m_swapTotal = swapTotal;
    //去除字符串空格 bug #111050
    QStringList Swaplist = m_swapTotal.split(' ');
    m_swapTotal = "";
    for (auto str : Swaplist) {
        m_swapTotal += str;
    }

    update();
}

void MemoryWidget::changeTheme(DApplicationHelper::ColorType themeType)
{
    switch (themeType) {
    case DApplicationHelper::LightType:
        numberColor.setRgb(0, 26, 46);
        m_titleTrans = Globals::TitleTransLight;
        m_contentTrans = Globals::contentTransLight;
        m_hoverTrans = Globals::hoverTransLight;
        m_icon = QIcon(QString(":/icons/deepin/builtin/light/icon_memory.png"));
        break;
    case DApplicationHelper::DarkType:
        numberColor.setRgb(192, 198, 212);
        m_titleTrans = Globals::TitleTransDark;
        m_contentTrans = Globals::contentTransDark;
        m_hoverTrans = Globals::hoverTransDark;
        m_icon = QIcon(QString(":/icons/deepin/builtin/dark/icon_memory.png"));
        break;
    default:
        break;
    }

    // init colors
    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();

    textColor = palette.color(DPalette::Text);
    summaryColor = palette.color(DPalette::TextTips);
}

void MemoryWidget::paintEvent(QPaintEvent *e)
{
    setFixedWidth(m_width);
    QPainter painter;
    painter.begin(this);
    painter.setPen(textColor);

    //裁剪绘制区域
    QPainterPath path;
    path.addRoundedRect(rect(), 8, 8);
    painter.setClipPath(path);
    if (m_isHover) {
        painter.fillRect(rect(), QBrush(QColor(255, 255, 255, m_hoverTrans)));
    } else {
        painter.fillRect(rect(), QBrush(QColor(255, 255, 255, 0)));
    }

    //标题栏背景
    QRect titleRect(rect().x(), rect().y(), m_width, 36);
    painter.fillRect(titleRect, QBrush(QColor(255, 255, 255, m_titleTrans)));
    QRect contentRect(rect().x(), rect().y() + 36, m_width, 117);
    painter.fillRect(contentRect, QBrush(QColor(255, 255, 255, m_contentTrans)));

    //标题
    QString strMemory = DApplication::translate("Memory.Widget", "Memory");
    painter.setFont(m_sectionFont);
    QFontMetrics fmTitle = painter.fontMetrics();
    int widthTitleTxt = fmTitle.width(strMemory);
    int heightTitleTxt = fmTitle.descent() + fmTitle.ascent();
    QRect netTitleRect(titleRect.x(), titleRect.y(), widthTitleTxt, heightTitleTxt);
    painter.drawText(titleRect, Qt::AlignHCenter | Qt::AlignVCenter, strMemory);

    //图标
    int iconSize = 20;
    QRect iconRect(titleRect.x() + (titleRect.width() - widthTitleTxt) / 2 - iconSize, titleRect.y() + qCeil((titleRect.height() - iconSize) / 2.) + 2, iconSize, iconSize);
    m_icon.paint(&painter, iconRect);

    int sectionSize = 6;
    QString memoryTitleUnit = QString("%1 / %2")
                                      .arg(m_memUsageUnit)
                                      .arg(m_memTotal);

    QString memoryContent = QString("%1 (%2%)")
                                    .arg(strMemory)
                                    .arg(m_memPercent);

    QString swapTitle = "";
    QString swapContent = "";
    if (m_swapTotal == "0.0 B") {
        // After the memory and swap space text, add a space before the brackets
        m_swapUsage = "-/-";
        swapContent = QString("%1 (%2)")
                              .arg(DApplication::translate("Memory.Widget", "Swap"))
                              .arg(DApplication::translate("Memory.Widget", "Not enabled"));
    } else {
        swapTitle = QString("%1 / %2")
                            .arg(m_swapUnit)
                            .arg(m_swapTotal);

        swapContent = QString("%1 (%2%)")
                              .arg(DApplication::translate("Memory.Widget", "Swap"))
                              .arg(m_swapPercent);
    }

    QFontMetrics fmMem(m_memFont);
    QFontMetrics fmMemUnit(m_memUnitFont);
    QFontMetrics fmMemTxt(m_memTxtFont);
    //    QFontMetrics fmMemTxt(m_memTxtFont);

    int letfsize = 36;
    int margin = 10;
    int fontMargin = 5;
    //内存数字
    QRect memRect(letfsize, contentRect.y() + fontMargin,
                  fmMem.size(Qt::TextSingleLine, m_memUsage).width(), fmMem.height());
    //内存单位
    QRect memRectUnit(memRect.x() + memRect.width() - 1, memRect.y() + margin,
                      fmMemUnit.size(Qt::TextSingleLine, memoryTitleUnit).width(), fmMem.height());
    //内存txt
    QRect memTxtRect(letfsize, memRect.y() + memRect.height(),
                     fmMemTxt.size(Qt::TextSingleLine, memoryContent).width(),
                     fmMemTxt.height());
    QRectF memIndicatorRect(memTxtRect.x() - margin, memTxtRect.y() + qCeil((memTxtRect.height() - sectionSize) / 2.),
                            sectionSize, sectionSize);

    QPainterPath section;
    section.addEllipse(memIndicatorRect);
    painter.fillPath(section, memoryColor);

    painter.setFont(m_memFont);
    painter.drawText(memRect, Qt::AlignLeft | Qt::AlignVCenter,
                     fmMem.elidedText(m_memUsage, Qt::ElideRight,
                                      rect().width() - memRect.x() - outsideRingRadius));

    painter.setFont(m_memUnitFont);
    painter.drawText(memRectUnit, Qt::AlignLeft | Qt::AlignHCenter, memoryTitleUnit);
    painter.setFont(m_memTxtFont);
    painter.setOpacity(0.6);
    painter.drawText(memTxtRect, Qt::AlignLeft | Qt::AlignVCenter, memoryContent);
    painter.setOpacity(1);

    //swap数字
    QRect swapRect(letfsize, memTxtRect.y() + memTxtRect.height() + fontMargin,
                   fmMem.size(Qt::TextSingleLine, m_swapUsage).width(), fmMem.height());
    //swap单位
    QRect swapRectUnit(swapRect.x() + swapRect.width() - 1, swapRect.y() + margin,
                       fmMemUnit.size(Qt::TextSingleLine, swapTitle).width(), fmMem.height());

    QRect swapTxtRect(letfsize, swapRect.y() + swapRect.height(),   //+ topsize
                      fmMemTxt.size(Qt::TextSingleLine, swapContent).width(),
                      fmMemTxt.height());
    QRectF swapIndicatorRect(swapTxtRect.x() - margin, swapTxtRect.y() + qCeil((swapTxtRect.height() - sectionSize) / 2.),
                             sectionSize, sectionSize);

    QPainterPath section2;
    section2.addEllipse(swapIndicatorRect);
    painter.fillPath(section2, swapColor);

    //关闭交换空间时，显示-/-
    if (m_swapTotal == "0.0 B") {
        painter.setFont(m_memUnitFont);
        painter.drawText(swapRect, m_swapUsage);
    } else {
        painter.setFont(m_memFont);
        painter.drawText(swapRect, m_swapUsage);
        painter.setFont(m_memUnitFont);
        painter.drawText(swapRectUnit, swapTitle);
    }

    painter.setFont(m_memTxtFont);
    painter.setOpacity(0.6);
    painter.drawText(swapTxtRect, Qt::AlignLeft | Qt::AlignVCenter, swapContent);
    painter.setOpacity(1);

    ringCenterPointerX = 200;
    // Draw memory ring.
    drawLoadingRing(painter, contentRect.x() + ringCenterPointerX, contentRect.y() + ringCenterPointerY,
                    outsideRingRadius, ringWidth, 270, 270, memoryForegroundColor,
                    memoryForegroundOpacity, memoryBackgroundColor, memoryBackgroundOpacity,
                    m_memPercent.toDouble() / 100);

    // Draw swap ring.
    if (m_swapTotal != "0.0 B") {
        drawLoadingRing(painter, contentRect.x() + ringCenterPointerX, contentRect.y() + ringCenterPointerY,
                        insideRingRadius, ringWidth, 270, 270, swapForegroundColor,
                        swapForegroundOpacity, swapBackgroundColor, swapBackgroundOpacity, m_swapPercent.toDouble() / 100);
    }

    QString memPerUnitTitle = "%";
    int memPerUnitTitleWidth = fmMemUnit.size(Qt::TextSingleLine, memPerUnitTitle).width();

    // Draw percent text.
    painter.setFont(m_memFont);
    painter.setPen(textColor);
    QString memPer = QString::number(m_memPercent.toDouble(), 'f', 1);
    QRect memPerRect(contentRect.x() + ringCenterPointerX - (insideRingRadius + memPerUnitTitleWidth) / 2,
                     contentRect.y() + ringCenterPointerY - (insideRingRadius) / 2 + fmMemUnit.height() / 4,
                     fmMem.size(Qt::TextSingleLine, memPer).width(), fmMem.height());
    painter.drawText(memPerRect, Qt::AlignRight | Qt::AlignVCenter, memPer);

    //Draw %
    painter.setFont(m_memUnitFont);
    QRect memPerUnitRect(memPerRect.x() + memPerRect.width(), memPerRect.y() + fmMemUnit.height() / 2,
                         fmMemUnit.size(Qt::TextSingleLine, memPerUnitTitle).width(), fmMemUnit.height());
    painter.drawText(memPerUnitRect, Qt::AlignHCenter | Qt::AlignVCenter, memPerUnitTitle);
}

bool MemoryWidget::eventFilter(QObject *target, QEvent *event)
{
    if (target == this) {
        if (event->type() == QEvent::Enter) {
            m_isHover = true;
            update();
            return true;
        }
        if (event->type() == QEvent::Leave) {
            m_isHover = false;
            update();
            return true;
        }
    }
    return QWidget::eventFilter(target, event);
}

void MemoryWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        DataDealSingleton::getInstance().sendJumpWidgetMessage("MSG_MEM");
        return;
    }
    return QWidget::mouseDoubleClickEvent(event);
}

void MemoryWidget::changeFont(const QFont &font)
{
    m_sectionFont = font;
    m_sectionFont.setWeight(QFont::DemiBold);
    m_sectionFont.setPointSizeF(m_sectionFont.pointSizeF());

    m_memFont = font;
    m_memFont.setWeight(QFont::Normal);
    m_memFont.setPointSizeF(Globals::ContentFont);

    m_memUnitFont = font;
    m_memUnitFont.setWeight(QFont::Normal);
    m_memUnitFont.setPointSizeF(Globals::ContentUnitFont);

    m_memTxtFont = font;
    m_memTxtFont.setWeight(QFont::ExtraLight);
    m_memTxtFont.setPointSizeF(Globals::subContentFont);
}
