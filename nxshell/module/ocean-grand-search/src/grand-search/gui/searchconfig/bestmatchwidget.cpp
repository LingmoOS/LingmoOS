// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bestmatchwidget.h"
#include "switchwidget/switchwidget.h"
#include "gui/datadefine.h"
#include "business/config/searchconfig.h"
#include "utils/utils.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace GrandSearch;

BestMatchWidget::BestMatchWidget(QWidget *parent)
    : DWidget(parent)
{
    m_groupLabel = new QLabel(tr("Best match"), this);
    m_groupLabel->adjustSize();

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainLayout->addWidget(m_groupLabel);

    m_groupName = {{GRANDSEARCH_CUSTOM_BESTMATCH, GroupName_Best}};

    QStringList displayOrder;
    displayOrder << GRANDSEARCH_CUSTOM_BESTMATCH;

    m_displayIcons << QStringLiteral("bestmatch");

    for (int i = 0; i < displayOrder.count(); ++i) {
        const QString &searchGroup = displayOrder.at(i);
        bool enable = SearchConfig::instance()->getConfig(GRANDSEARCH_CUSTOM_GROUP, searchGroup, true).toBool();
        SwitchWidget *switchWidget = new SwitchWidget(this);
        switchWidget->setFixedSize(SWITCHWIDGETWIDTH, SWITCHWIDGETHEIGHT);
        switchWidget->setEnableBackground(true);
        switchWidget->setTitle(m_groupName.value(searchGroup));
        switchWidget->setChecked(enable);
        switchWidget->setProperty(GRANDSEARCH_CUSTOM_GROUP, searchGroup);

        m_mainLayout->addWidget(switchWidget);
        m_switchWidgets.append(switchWidget);

        connect(switchWidget, &SwitchWidget::checkedChanged, this, &BestMatchWidget::onSwitchStateChanged);
    }

    setLayout(m_mainLayout);
    updateIcons();

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &BestMatchWidget::updateIcons);

}

BestMatchWidget::~BestMatchWidget()
{
    m_switchWidgets.clear();
}

void BestMatchWidget::onSwitchStateChanged(const bool checked)
{
    QObject *obj = sender();
    SwitchWidget *switchWidget = qobject_cast<SwitchWidget *>(obj);

    if (switchWidget) {
        QString group = switchWidget->property(GRANDSEARCH_CUSTOM_GROUP).toString();
        SearchConfig::instance()->setConfig(GRANDSEARCH_CUSTOM_GROUP, group, checked);
    }
}

void BestMatchWidget::updateIcons()
{
    Q_ASSERT(m_switchWidgets.count() == m_displayIcons.count());

    QString suffix = Utils::iconThemeSuffix();

    for (int i = 0; i < m_switchWidgets.count(); ++i) {

        QString iconName = m_displayIcons.at(i);
        QIcon icon = QIcon(QString(":/icons/%1%2.svg").arg(iconName).arg(suffix));

        auto switchWidget = m_switchWidgets.at(i);
        Q_ASSERT(switchWidget);

        switchWidget->setIcon(icon, QSize(SWITCHWIDGETICONSIZE, SWITCHWIDGETICONSIZE));
    }

    update();
}

