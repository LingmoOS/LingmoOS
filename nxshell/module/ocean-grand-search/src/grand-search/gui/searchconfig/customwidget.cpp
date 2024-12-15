// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customwidget.h"
#include "switchwidget/switchwidget.h"
#include "gui/datadefine.h"
#include "business/config/searchconfig.h"
#include "utils/utils.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"
#include "planwidget.h"
#include "bestmatchwidget.h"
#include "searchenginewidget.h"
#include "tailerwidget.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

CustomWidget::CustomWidget(QWidget *parent)
    : DWidget(parent)
{
    m_groupLabel = new QLabel(tr("Custom search"), this);
    DFontSizeManager::instance()->bind(m_groupLabel, DFontSizeManager::T5, QFont::Bold);
    m_groupLabel->setMargin(0);

    m_mainLayout = new QVBoxLayout();
    setLayout(m_mainLayout);

    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainLayout->addWidget(m_groupLabel);

    m_innerLayout = new QVBoxLayout();
    m_mainLayout->addLayout(m_innerLayout);

    m_innerLayout->setSpacing(20);
    m_innerLayout->setContentsMargins(0, 0, 0, 0);

    m_tailerWidget = new TailerWidget(this);
    m_bestMatchWidget = new BestMatchWidget(this);
    m_searchEngineWidget = new SearchEngineWidget(this);
    m_searchPlanWidget = new PlanWidget(this);
    m_innerLayout->addWidget(m_tailerWidget);
    m_innerLayout->addWidget(m_bestMatchWidget);
    m_innerLayout->addWidget(m_searchEngineWidget);
    m_innerLayout->addWidget(m_searchPlanWidget);
}

CustomWidget::~CustomWidget()
{
    m_switchWidgets.clear();
}

void CustomWidget::onSwitchStateChanged(const bool checked)
{
    QObject *obj = sender();
    SwitchWidget *switchWidget = static_cast<SwitchWidget *>(obj);

    if (switchWidget) {
        QString group = switchWidget->property(GRANDSEARCH_CUSTOM_GROUP).toString();
        SearchConfig::instance()->setConfig(GRANDSEARCH_CUSTOM_GROUP, group, checked);
    }
}
