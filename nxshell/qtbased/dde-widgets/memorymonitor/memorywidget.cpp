// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "memorywidget.h"
#include "common/utils.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DPaletteHelper>

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QBrush>
#include <QPaintEvent>
#include <QFontMetrics>

DWIDGET_USE_NAMESPACE

using namespace Utils;

MemoryWidget::MemoryWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *dAppHelper = DApplicationHelper::instance();
    connect(dAppHelper, &DApplicationHelper::themeTypeChanged, this, &MemoryWidget::changeTheme);
    changeTheme(dAppHelper->themeType());

    changeFont(DApplication::font());
    connect(dynamic_cast<QGuiApplication *>(DApplication::instance()), &DApplication::fontChanged,
            this, &MemoryWidget::changeFont);
}
MemoryWidget::~MemoryWidget() {}

void MemoryWidget::updateMemoryInfo(const QString &memPercent,
                                    const QString &swapPercent)
{
    m_memPercent = memPercent;
    m_swapPercent = swapPercent;
}

void MemoryWidget::changeTheme(DApplicationHelper::ColorType themeType)
{
    switch (themeType) {
    case DApplicationHelper::LightType:
        memoryBackgroundColor = "#000000";
        swapBackgroundColor = "#000000";

        break;
    case DApplicationHelper::DarkType:
        memoryBackgroundColor = "#FFFFFF";
        swapBackgroundColor = "#FFFFFF";

        break;
    default:
        break;
    }

    // init colors
    auto palette = DPaletteHelper::instance()->palette(this);
#ifndef THEME_FALLBACK_COLOR
    numberColor = palette.color(QPalette::BrightText); // was DPalette::TextTitle
#else
    numberColor = palette.color(DPalette::Text);
#endif

    summaryColor = palette.color(DPalette::TextTips);
    backgroundBase = palette.color(QPalette::Base);
    backgroundBase.setAlpha(100);
}

void MemoryWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter;
    painter.begin(this);

    painter.setRenderHint(QPainter::Antialiasing);
    //裁剪绘制区域
    QPainterPath path;
    path.addRoundedRect(rect(), roundedCornerRadius(), roundedCornerRadius());
    painter.setClipPath(path);
    //背景
    QRect contentRect(rect());
    painter.fillRect(contentRect, QBrush(backgroundBase));

    int sectionSize = 6;
    bool drawSwapControl = !m_swapPercent.isEmpty();

    QString memoryContent = QString("%1 (%2%)")
                          .arg(tr("Memory"))//Memory
                          .arg(m_memPercent);

    QString swapContent;
    if (drawSwapControl) {
        swapContent = QString("%1 (%2%)")
                            .arg(tr("SW Memory"))
                            .arg(m_swapPercent);
    }

    QFontMetrics fmMemTxt(m_memTxtFont);
    const int ContentTextWidth = qMax(fmMemTxt.size(Qt::TextSingleLine, memoryContent).width(),
                                      fmMemTxt.size(Qt::TextSingleLine, swapContent).width());

    int leftMargin = (contentRect.width() - ContentTextWidth) / 2;
    int topMargin = 15;
    int margin = 8;
    //内存txt
    QRect memTxtRect(contentRect.left() + leftMargin, contentRect.top() + topMargin,
                      fmMemTxt.size(Qt::TextSingleLine, memoryContent).width(),
                      fmMemTxt.height());
    QRectF memIndicatorRect(memTxtRect.x() - margin, memTxtRect.y() + qCeil((memTxtRect.height() - sectionSize) / 2.),
                            sectionSize, sectionSize);


    QPainterPath section;
    section.addEllipse(memIndicatorRect);
    painter.fillPath(section, memoryColor);

    painter.setFont(m_memTxtFont);
    painter.setPen(QPen(numberColor));
    painter.drawText(memTxtRect, Qt::AlignLeft | Qt::AlignVCenter, memoryContent);

    QRect swapTxtRect(memTxtRect.left(), memTxtRect.bottom() + margin,//+ topsize
                      fmMemTxt.size(Qt::TextSingleLine, swapContent).width(),
                      fmMemTxt.height());
    QRectF swapIndicatorRect(swapTxtRect.x() - margin, swapTxtRect.y() + qCeil((swapTxtRect.height() - sectionSize) / 2.),
                             sectionSize, sectionSize);

    if (drawSwapControl) {
        QPainterPath section2;
        section2.addEllipse(swapIndicatorRect);
        painter.fillPath(section2, swapColor);

        painter.setFont(m_memTxtFont);
        painter.setPen(QPen(numberColor));
        painter.drawText(swapTxtRect, Qt::AlignLeft | Qt::AlignVCenter, swapContent);
    }

    const int outsideRingRadius = (contentRect.bottom() - swapTxtRect.bottom() - topMargin) / 2;
    const int insideRingRadius = outsideRingRadius - ringWidth - 1;
    const auto &ringCenter = QPoint(contentRect.center().x(), swapIndicatorRect.bottom() + margin + outsideRingRadius);
    // Draw memory ring.
    drawLoadingRing(painter, ringCenter.x(), ringCenter.y(),
                    outsideRingRadius, ringWidth, 270, 270, memoryForegroundColor,
                    memoryForegroundOpacity, memoryBackgroundColor, memoryBackgroundOpacity,
                    m_memPercent.toDouble()/100);

    // Draw swap ring.
    if (drawSwapControl) {
        drawLoadingRing(painter, ringCenter.x(), ringCenter.y(),
                        insideRingRadius, ringWidth, 270, 270, swapForegroundColor,
                        swapForegroundOpacity, swapBackgroundColor, swapBackgroundOpacity, m_swapPercent.toDouble()/100);
    }

    // Draw percent text.
    painter.setFont(m_memPercentFont);
    painter.setPen(QPen(numberColor));
    painter.drawText(QRect(ringCenter.x() - insideRingRadius,
                           ringCenter.y() - insideRingRadius, insideRingRadius * 2,
                           insideRingRadius * 2),
                     Qt::AlignHCenter | Qt::AlignVCenter,
                     QString("%1%").arg(QString::number(m_memPercent.toDouble(), 'f', 1)));
}

void MemoryWidget::changeFont(const QFont &font)
{
    m_memTxtFont = font;
    m_memTxtFont.setFamily("SourceHanSansSC");
    m_memTxtFont.setWeight(QFont::ExtraLight);
    m_memTxtFont.setPointSizeF(m_memTxtFont.pointSizeF()-2 );
}

int MemoryWidget::roundedCornerRadius() const
{
    return m_roundedCornerRadius;
}

void MemoryWidget::setRoundedCornerRadius(int newRoundedCornerRadius)
{
    if (m_roundedCornerRadius == newRoundedCornerRadius)
        return;
    m_roundedCornerRadius = newRoundedCornerRadius;
    emit roundedCornerRadiusChanged();
}
