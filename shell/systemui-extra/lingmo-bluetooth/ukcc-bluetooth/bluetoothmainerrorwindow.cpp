/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "bluetoothmainerrorwindow.h"

BluetoothMainErrorWindow::BluetoothMainErrorWindow(QString str_error , QWidget * parent) :
    m_str_error(str_error),
    QWidget(parent)
{
    InitErrorWindow();
}

void BluetoothMainErrorWindow::InitErrorWindow()
{
    QVBoxLayout *errorWidgetLayout = new QVBoxLayout(this);
    QLabel      *errorWidgetIcon   = new QLabel(this);
    errorWidgetTip    = new QLabel(this);

    this->setObjectName("ErrorWidget");

    errorWidgetLayout->setSpacing(10);
    errorWidgetLayout->setMargin(0);
    errorWidgetLayout->setContentsMargins(0,0,0,0);

    errorWidgetIcon->setFixedSize(56,56);
    errorWidgetTip->resize(200,30);
    errorWidgetTip->setFont(QFont("Noto Sans CJK SC",18,QFont::Bold));

    errorWidgetLayout->addStretch(10);

    if (QIcon::hasThemeIcon("dialog-warning")) {
        errorWidgetIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(56,56));
        errorWidgetLayout->addWidget(errorWidgetIcon,1,Qt::AlignCenter);
    }

    errorWidgetTip->setText(m_str_error);
    errorWidgetLayout->addWidget(errorWidgetTip,1,Qt::AlignCenter);
    errorWidgetLayout->addStretch(10);
}

void BluetoothMainErrorWindow::setErrorText(QString error_str)
{
    m_str_error = error_str;
    errorWidgetTip->setText(error_str);
}
