// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingitem.h"
#include "gui/utils/cooperationguihelper.h"
#include "global_defines.h"

#include <QPainter>
#include <QPainterPath>

using namespace cooperation_core;

SettingItem::SettingItem(QWidget *parent)
    : QFrame(parent)
{
    mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(10, 6, 10, 6);
    setLayout(mainLayout);
#ifndef linux
    setFixedHeight(48);
#endif
}

void SettingItem::setItemInfo(const QString &text, QWidget *w)
{
    CooperationLabel *label = new CooperationLabel(text, this);
    auto font = label->font();
    font.setWeight(QFont::Medium);
    label->setFont(font);

    mainLayout->addWidget(label, 0, Qt::AlignLeft);
    mainLayout->addWidget(w, 0, Qt::AlignRight);
}

void SettingItem::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    const int radius = 8;
    QRect paintRect = this->rect();
    QPainterPath path;
    path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
    path.lineTo(paintRect.topRight() + QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                     QSize(radius * 2, radius * 2)),
               0, 90);
    path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                     QSize(radius * 2, radius * 2)),
               180, 90);
    path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                     QSize(radius * 2, radius * 2)),
               270, 90);
#ifdef linux
    QColor color(0, 0, 0, static_cast<int>(255 * 0.03));
#else
    QColor color(0, 0, 0, static_cast<int>(255 * 0.09));
#endif
    if (CooperationGuiHelper::isDarkTheme())
        color.setRgb(255, 255, 255, static_cast<int>(255 * 0.05));

    painter.fillPath(path, color);

    return QFrame::paintEvent(event);
}
