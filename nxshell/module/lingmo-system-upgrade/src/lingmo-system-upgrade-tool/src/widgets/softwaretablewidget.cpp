// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DApplicationHelper>

#include <QScrollBar>
#include <QPainter>
#include <QPainterPath>

#include "../core/constants.h"
#include "../core/utils.h"
#include "softwaretablewidget.h"
#include "../tool/application.h"

void SoftwareTableWidget::initUI()
{
    this->setColumnCount(2);

    // Set no focus on table
    setFocusPolicy(Qt::NoFocus);

    // Hide borders
    this->setFrameStyle(QFrame::NoFrame);

    // Set background
    setAutoFillBackground(true);

    // Disable editing on table items.
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Set no selection
    this->setSelectionMode(QAbstractItemView::NoSelection);

    // No Grid
    this->setShowGrid(false);

    // Set alternating row colors
    setAlternatingRowColors(true);
    setRowColorStyles();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, [this] {
        setRowColorStyles();
    });
    connect(verticalScrollBar(), &QScrollBar::valueChanged, viewport(), [this]() {
        viewport()->update();
    });

    // Hide headers
    this->verticalHeader()->setVisible(false);
    setHorizontalHeader(m_headerView);
    this->horizontalHeader()->setVisible(false);

    // Disable sorting
    this->setSortingEnabled(false);

    // Setting stretching
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader()->setDefaultSectionSize(SOFTWARE_TABLE_ROW_H);
    horizontalHeader()->stretchLastSection();

    // Set row height
    this->verticalHeader()->setDefaultSectionSize(SOFTWARE_TABLE_ROW_H);

    // Set transparent background for window.
    this->setAttribute(Qt::WA_TranslucentBackground);
    // Set frameless window
    this->setWindowFlags(Qt::FramelessWindowHint);
}

void SoftwareTableWidget::paintEvent(QPaintEvent *event)
{
    QWidget *wnd = Application::getInstance()->activeWindow();
    DPalette::ColorGroup cg;
    if (!wnd) {
        cg = DPalette::Inactive;
    } else {
        cg = DPalette::Active;
    }

    QRect rect = this->rect();
    rect.setY(rect.y() - m_headerView->height());
    rect.setHeight(SOFTWARE_TABLE_ROW_H * 5);
    QPainter painter(this->viewport());
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);

    DPalette palette = DApplicationHelper::instance()->applicationPalette();

    int width = 1;
    int radius = 8;
    QPainterPath paintPath, paintPathOut, paintPathIn;
    paintPathOut.addRoundedRect(rect, radius, radius);

    QRect rectIn = QRect(rect.x() + width, rect.y() + width, rect.width() - width * 2, rect.height() - width * 2);
    paintPathIn.addRoundedRect(rectIn, radius, radius);
    paintPath = paintPathOut.subtracted(paintPathIn);

    QBrush bgBrush(QColor("#1A000000"));
    painter.fillPath(paintPath, bgBrush);
    painter.fillRect(rect.x() + rect.width() / 2, rect.y(), width, rect.height(), bgBrush);

    QPainterPath cornerOutPath;
    cornerOutPath.addRect(rect);
    cornerOutPath = cornerOutPath.subtracted(paintPathOut);
    painter.fillPath(cornerOutPath, QBrush(QColor(isDarkMode() ? "#232323" : "#FFFFFF")));

    painter.restore();
    DTableWidget::paintEvent(event);
}

void SoftwareTableWidget::wheelEvent(QWheelEvent *event)
{
    DTableWidget::wheelEvent(event);
    viewport()->update();
}

void SoftwareTableWidget::setRowColorStyles()
{
    QPalette palette = this->palette();
    if (isDarkMode())
    {
        palette.setColor(QPalette::Base, QColor("#0DFFFFFF"));
        palette.setColor(QPalette::AlternateBase, QColor("#FF232323"));
    }
    else
    {
        palette.setColor(QPalette::Base, QColor("#FFFFFFFF"));
        palette.setColor(QPalette::AlternateBase, QColor("#08000000"));
    }
    setPalette(palette);
}

BorderRadiusHeaderView *SoftwareTableWidget::getHeaderView()
{
    return m_headerView;
}
