/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

/********************************************************************************
** Form generated from reading UI file 'frmlunarcalendarwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FRMLUNARCALENDARWIDGET_H
#define UI_FRMLUNARCALENDARWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "lunarcalendarwidget.h"

QT_BEGIN_NAMESPACE

class Ui_frmLunarCalendarWidget
{
public:
    QVBoxLayout *verticalLayout;
    LunarCalendarWidget *lunarCalendarWidget;
    QWidget *widgetBottom;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *frmLunarCalendarWidget)
    {
        if (frmLunarCalendarWidget->objectName().isEmpty())
            frmLunarCalendarWidget->setObjectName(QString::fromUtf8("frmLunarCalendarWidget"));
        frmLunarCalendarWidget->resize(600, 500);
        verticalLayout = new QVBoxLayout(frmLunarCalendarWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        lunarCalendarWidget = new LunarCalendarWidget(frmLunarCalendarWidget);
        lunarCalendarWidget->setObjectName(QString::fromUtf8("lunarCalendarWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lunarCalendarWidget->sizePolicy().hasHeightForWidth());
        lunarCalendarWidget->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(lunarCalendarWidget);

        widgetBottom = new QWidget(frmLunarCalendarWidget);
        widgetBottom->setObjectName(QString::fromUtf8("widgetBottom"));
        horizontalLayout = new QHBoxLayout(widgetBottom);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        horizontalLayout->addItem(horizontalSpacer);
        retranslateUi(frmLunarCalendarWidget);

        QMetaObject::connectSlotsByName(frmLunarCalendarWidget);
    } // setupUi

    void retranslateUi(QWidget *frmLunarCalendarWidget)
    {
        frmLunarCalendarWidget->setWindowTitle(QApplication::translate("frmLunarCalendarWidget", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class frmLunarCalendarWidget: public Ui_frmLunarCalendarWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FRMLUNARCALENDARWIDGET_H
