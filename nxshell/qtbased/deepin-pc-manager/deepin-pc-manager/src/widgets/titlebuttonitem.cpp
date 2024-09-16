// Copyright (C) 2019 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebuttonitem.h"

#include <QHBoxLayout>

namespace def {
namespace widgets {

TitleButtonItem::TitleButtonItem(QWidget *parent)
    : SettingsItem(parent)
    , m_titleLabel(new TipsLabel(this))
    , m_button(new QPushButton(this))
{
    addBackground();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(10, 5, 10, 5);
    setLayout(layout);

    layout->addWidget(m_titleLabel, 0, Qt::AlignLeft);

    layout->addWidget(m_button, 0, Qt::AlignRight);
}

// 设置相关标签名称
void TitleButtonItem::setAccessibleParentText(QString sAccessibleName)
{
    m_titleLabel->setAccessibleName(sAccessibleName + "_titleLable");
    m_button->setAccessibleName(sAccessibleName + "_settingButton");
}

void TitleButtonItem::setTitle(QString str)
{
    m_titleLabel->setText(str);
}

void TitleButtonItem::setButtonText(QString str)
{
    m_button->setText(str);
}

} // namespace widgets
} // namespace def
