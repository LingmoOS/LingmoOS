// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailcheckbox.h"

#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QLabel>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

DetailCheckBox::DetailCheckBox(const QString &title, QWidget *parent)
    : DetailCheckBox(title, new QLabel, parent)
{

}

DetailCheckBox::DetailCheckBox(const QString &title, QWidget *detail, QWidget *parent)
    : RoundedBackground(parent)
    , m_detail(detail)
{
    m_checkBox = new DCheckBox(title, parent);
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->addWidget(m_checkBox);
    m_mainLayout->setContentsMargins(10, 8, 10, 8);

    m_detail->setParent(this);
    if (auto label = qobject_cast<QLabel *>(m_detail)) {
        label->setWordWrap(true);
        DFontSizeManager::instance()->bind(m_detail, DFontSizeManager::T8);
        label->setIndent(27); // 37 - 10
    }

    m_mainLayout->addWidget(m_detail);

    setLayout(m_mainLayout);
}

void DetailCheckBox::setDetail(const QString &detail)
{
    if (auto label = qobject_cast<QLabel *>(m_detail))
        label->setText(detail);
}
