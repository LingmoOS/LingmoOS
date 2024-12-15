// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tailerwidget.h"
#include "business/config/searchconfig.h"
#include "global/searchconfigdefine.h"
#include "tipslabel.h"

#include <DFontSizeManager>
#include <DDialog>

using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

TailerWidget::TailerWidget(QWidget *parent)
    : DWidget(parent)
{
    m_groupLabel = new QLabel(tr("Tailer settings"), this);
    m_groupLabel->adjustSize();
    m_groupLabel->setMargin(0);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_groupLabel);
    m_mainLayout->addSpacerItem(new QSpacerItem(10, 4));

    QString content = tr("It is displayed at the end of search results for better "
                         "identification and distinction of items with duplicate names.");

    m_contentLabel = new TipsLabel(content, this);
    m_contentLabel->setWordWrap(true);
    m_mainLayout->addWidget(m_contentLabel);
    m_mainLayout->addSpacerItem(new QSpacerItem(10, 10));

    m_checkBoxWidget = new CheckBoxWidget(this);
    m_checkBoxWidget->addCheckBox(tr("Parent directory"),
                                  SearchConfig::instance()->getConfig(GRANDSEARCH_TAILER_GROUP, GRANDSEARCH_TAILER_PARENTDIR, false).toBool());
    m_checkBoxWidget->addCheckBox(tr("Time modified"),
                                  SearchConfig::instance()->getConfig(GRANDSEARCH_TAILER_GROUP, GRANDSEARCH_TAILER_TIMEMODEFIED, true).toBool());
    m_mainLayout->addWidget(m_checkBoxWidget);

    connect(m_checkBoxWidget, &CheckBoxWidget::checkedChanged, this, &TailerWidget::onCheckBoxStateChanged);
}

TailerWidget::~TailerWidget()
{
}

void TailerWidget::onCheckBoxStateChanged(int index, bool checked)
{
    switch (index) {
    case ParentDirectory:
        SearchConfig::instance()->setConfig(GRANDSEARCH_TAILER_GROUP, GRANDSEARCH_TAILER_PARENTDIR, checked);
        break;
    case TimeModified:
        SearchConfig::instance()->setConfig(GRANDSEARCH_TAILER_GROUP, GRANDSEARCH_TAILER_TIMEMODEFIED, checked);
        break;
    default:
        break;
    }
}
