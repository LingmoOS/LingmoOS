// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contenttitle.h"

#include <QHBoxLayout>

ContentTitle::ContentTitle(QWidget *parent) : QWidget(parent)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    titleLabel = new QLabel;
    layout->addWidget(titleLabel, 0, Qt::AlignLeft);

    auto lineLabel = new QLabel;
    layout->addWidget(lineLabel);
    lineLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QLabel *ContentTitle::label() const
{
    return titleLabel;
}

void ContentTitle::setTitle(const QString &title)
{
    titleLabel->setText(title);
}
