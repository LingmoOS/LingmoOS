// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingsheaderitem.h"

#include "labels/normallabel.h"

namespace dcc_fcitx_configtool {
namespace widgets {

FcitxSettingsHeaderItem::FcitxSettingsHeaderItem(QWidget *parent)
    : FcitxSettingsItem(parent)
    , m_mainLayout(new QHBoxLayout)
    , m_headerText(new FcitxTitleLabel)
{
    m_headerText->setObjectName("SettingsHeaderItemTitle");

    m_mainLayout->addSpacing(20);
    m_mainLayout->addWidget(m_headerText);
    m_mainLayout->addStretch();

    setFixedHeight(24);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setMargin(0);

    setLayout(m_mainLayout);
}

void FcitxSettingsHeaderItem::setTitle(const QString &title)
{
    m_headerText->setText(title);
}

void FcitxSettingsHeaderItem::setRightWidget(QWidget *widget)
{
    Q_ASSERT(widget);

    m_mainLayout->addWidget(widget, 0, Qt::AlignRight);
}

} // namespace widgets
} // namespace dcc
