// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkboxitem.h"

#include <QVBoxLayout>

#define RADIUS 8

using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

CheckBoxItem::CheckBoxItem(const QString &text, QWidget *parent)
    : RoundedBackground(parent)
{
    m_checkBox = new DCheckBox(text, parent);
    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_checkBox);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 0, 10, 0);
    setLayout(mainLayout);

    connect(m_checkBox, &DCheckBox::toggled, this, &CheckBoxItem::toggled);
}

CheckBoxItem::~CheckBoxItem()
{
}

void CheckBoxItem::setChecked(bool checked)
{
    m_checkBox->setChecked(checked);
}

QString CheckBoxItem::text() const
{
    return m_checkBox->text();
}
