/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "setFontSizePage.h"
#include "ui_setFontSizePage.h"

#include <QDesktopWidget>

SetFontSize::SetFontSize(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SetFontSize),
    m_parentWindow(parent)
{
    ui->setupUi(this);

    initSetup();
}

SetFontSize::~SetFontSize()
{
    delete ui;
}

void SetFontSize::initSetup()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    // setAttribute(Qt::WA_TranslucentBackground);

    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget->setItemDelegate(new ListItemDelegate());
    QFont font(ui->listWidget->font());
    font.setPointSize(8);
    ui->listWidget->setFont(font);

    for(int i=10; i<37 ; i++ )
    {
        ui->listWidget->addItem(QString::number(i));
    }

    for(int i = 0; i < ui->listWidget->count(); i++) {
        ui->listWidget->item(i)->setTextAlignment(Qt::AlignCenter);
    }
}

void SetFontSize::show()
{
    if (m_parentWindow != nullptr) {
        QPoint temp = QPoint(m_parentWindow->mapToGlobal(QPoint(0,0)).x(), m_parentWindow->mapToGlobal(QPoint(0,0)).y() + m_parentWindow->height());

        QDesktopWidget* desktopWidget = QApplication::desktop();
        QRect clientRect = desktopWidget->availableGeometry();

        if(height() + temp.y() < clientRect.height()) {
            this->move(temp);
        }
        else {
            this->move(QPoint(m_parentWindow->mapToGlobal(QPoint(0,0)).x(), m_parentWindow->mapToGlobal(QPoint(0,0)).y() - this->height()));
        }

    }
    QWidget::show();
}
