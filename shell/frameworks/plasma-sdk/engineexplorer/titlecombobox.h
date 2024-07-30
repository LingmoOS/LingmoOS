/*
 *   SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TITLECOMBOBOX_H
#define TITLECOMBOBOX_H

#include <KLocalizedString>

#include <QComboBox>
#include <QPainter>

#include <QDebug>

class TitleComboBox : public QComboBox
{
public:
    explicit TitleComboBox(QWidget *parent = nullptr)
        : QComboBox(parent)
    {
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QComboBox::paintEvent(event);

        if (currentIndex() != -1) {
            return;
        }

        QPainter p(this);
        /*QFont bold = p.font();
        bold.setBold(true);
        p.setFont(bold);*/
        p.setPen(palette().color(QPalette::Disabled, QPalette::WindowText));
        int frameWidth = style()->pixelMetric(QStyle::PM_ComboBoxFrameWidth);
        QRect r = rect().adjusted(frameWidth, frameWidth, frameWidth, frameWidth);
        p.drawText(QStyle::visualRect(layoutDirection(), rect(), r), i18n("Data Engines"));
    }
};

#endif
