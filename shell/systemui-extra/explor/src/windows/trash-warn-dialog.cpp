/*
 * Peony-Qt
 *
 * Copyright (C) 2021, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenfei He <hewenfei@kylinos.cn>
 *
 */

#include "trash-warn-dialog.h"

#include <QPushButton>
#include <QPainter>
#include <QBoxLayout>
#include <QLabel>

using namespace Peony;

void TrashWarnDialog::paintEvent(QPaintEvent *event)
{
    const QPalette &palette = this->palette();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setBrush(palette.brush(QPalette::Background));
    painter.setPen(palette.color(QPalette::Background));

    QRect rect = this->rect();
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);
    painter.drawRoundedRect(rect, 16, 16);

    QWidget::paintEvent(event);
}

TrashWarnDialog::TrashWarnDialog(QWidget *parent) : QDialog(parent)
{
    //调整大小
    this->setFixedSize(320,344);
    //去掉窗口图标和标题，只显示关闭按钮,模态窗口
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

    this->setWindowModality(Qt::ApplicationModal);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    //清除默认边距
    dialogLayout->setMargin(0);
    this->setLayout(dialogLayout);

    QHBoxLayout *floor1 = new QHBoxLayout(this);
    floor1->setContentsMargins(0, 16, 16, 0);
    floor1->setAlignment(Qt::AlignRight);

    //去掉窗口标题后手动设置一个关闭按钮
    QPushButton *closeButton = new QPushButton(this);
    closeButton->setFixedSize(16, 16);

    closeButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
    //设置按钮的悬停事件和点击事件颜色
    closeButton->setStyleSheet("QPushButton:hover{background-color: #FF9E9E;border-radius:4px;border:none;}"
                               "QPushButton:pressed{background-color:#FB5050;border-radius:4px;border:none;}");

    floor1->addWidget(closeButton);
    dialogLayout->addLayout(floor1);

    QHBoxLayout *floor2 = new QHBoxLayout(this);
    floor2->setContentsMargins(0, 40, 0, 0);
    floor2->setAlignment(Qt::AlignHCenter);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setMargin(0);
    iconLabel->setAlignment(Qt::AlignHCenter);
    QPixmap pixmap = QIcon::fromTheme("user-trash-full").pixmap(64, 64);
    iconLabel->setPixmap(pixmap);

    floor2->addWidget(iconLabel);
    dialogLayout->addLayout(floor2);

    QHBoxLayout *floor3 = new QHBoxLayout(this);
    floor3->setContentsMargins(24, 24, 24, 0);

    QLabel *textLineLabel = new QLabel(this);
    textLineLabel->setMargin(0);
    textLineLabel->setWordWrap(true);
    textLineLabel->setAlignment(Qt::AlignHCenter);

    textLineLabel->setText(tr("Are you sure that you want to delete these files? Once you start a deletion, "
                                       "the files deleting will never be restored again."));
    floor3->addWidget(textLineLabel);
    dialogLayout->addLayout(floor3);

    QHBoxLayout *floor4 = new QHBoxLayout(this);
    floor4->setContentsMargins(24, 0, 24, 24);
    QPushButton *deleteButton = new QPushButton(this);
    QPushButton *cancelButton = new QPushButton(this);

    deleteButton->setFixedSize(124, 48);
    cancelButton->setFixedSize(124, 48);

    deleteButton->setText(tr("Delete"));
    cancelButton->setText(tr("Cancel"));

    floor4->addWidget(cancelButton);
    floor4->addStretch(1);
    floor4->addWidget(deleteButton);

    dialogLayout->addLayout(floor4);

    connect(closeButton, &QPushButton::clicked, this, [=]{
        Q_EMIT rejected();
        close();
    });

    connect(cancelButton, &QPushButton::clicked, this, [=]{
        Q_EMIT rejected();
        close();
    });

    connect(deleteButton, &QPushButton::clicked, this, [=]{
        Q_EMIT accepted();
        close();
    });
}
