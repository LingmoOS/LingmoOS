// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "planwidget.h"
#include "business/config/searchconfig.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"
#include "switchwidget/switchwidget.h"
#include "hyperlinklabel/hyperlinklabel.h"
#include "utils/utils.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DSysInfo>

#include <QLabel>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
using namespace GrandSearch;

PlanWidget::PlanWidget(QWidget *parent)
    : DWidget(parent)
{
    m_groupLabel = new QLabel(tr("Search experience program"), this);
    m_groupLabel->adjustSize();

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_groupLabel);

    QString display = tr("Join search experience program");
    m_displayIcon = "experienceplan";

    bool enable = SearchConfig::instance()->getConfig(GRANDSEARCH_PLAN_GROUP, GRANDSEARCH_PLAN_EXPERIENCE, false).toBool();
    m_switchWidget = new SwitchWidget(this);
    m_switchWidget->setFixedSize(SWITCHWIDGETWIDTH, SWITCHWIDGETHEIGHT);
    m_switchWidget->setEnableBackground(true);
    m_switchWidget->setTitle(display);
    m_switchWidget->setChecked(enable);
    m_mainLayout->addSpacerItem(new QSpacerItem(10, 10));
    m_mainLayout->addWidget(m_switchWidget);
    m_switchWidget->setProperty(GRANDSEARCH_SEARCH_GROUP, GRANDSEARCH_PLAN_EXPERIENCE);

    QString content = tr("Joining the search experience program means that "
                         "you grant and authorize us to collect the information of "
                         "your device and system, file icons, content and properties, "
                         "applications and their configurations, the contents you search while using the Application, "
                         "the time of search, the type of requested large model."
                         "If you refuse our collection "
                         "and use of the aforementioned information, do not join the program.");
    m_contentLabel = new QLabel(content, this);
    m_contentLabel->setWordWrap(true);
    DFontSizeManager::instance()->bind(m_contentLabel, DFontSizeManager::T10);

    const QString leadintText = tr("To know more about the management of your data, "
                                            "please refer to the UnionTech Software Privacy Policy (");
    const QString uosHyperlink = tr("https://www.uniontech.com/agreement/privacy-en");
    const QString communityHyperlink = tr("https://www.deepin.org/en/agreement/privacy/");

    const QString endText = tr(").");

    QString showHyperlink;
    if (DSysInfo::uosEditionType() == DSysInfo::UosCommunity)
        showHyperlink = communityHyperlink;
    else
        showHyperlink = uosHyperlink;

    m_tipsLabel = new HyperlinkLabel(leadintText, showHyperlink, endText, this);
    DFontSizeManager::instance()->bind(m_tipsLabel, DFontSizeManager::T10);

    m_mainLayout->addSpacerItem(new QSpacerItem(10, 10));
    m_mainLayout->addWidget(m_contentLabel);
    m_mainLayout->addSpacerItem(new QSpacerItem(6, 6));
    m_mainLayout->addWidget(m_tipsLabel);

    updateIcons();

    connect(m_switchWidget, &SwitchWidget::checkedChanged, this, &PlanWidget::onSwitchStateChanged);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &PlanWidget::updateIcons);
}

PlanWidget::~PlanWidget()
{

}

void PlanWidget::onSwitchStateChanged(const bool checked)
{
    QObject *obj = sender();
    SwitchWidget *switchWidget = static_cast<SwitchWidget *>(obj);

    if (switchWidget) {
        SearchConfig::instance()->setConfig(GRANDSEARCH_PLAN_GROUP, GRANDSEARCH_PLAN_EXPERIENCE, checked);
    }
}

void PlanWidget::updateIcons()
{
    QString suffix = Utils::iconThemeSuffix();
    QIcon icon = QIcon(QString(":/icons/%1%2.svg").arg(m_displayIcon).arg(suffix));
    m_switchWidget->setIcon(icon, QSize(SWITCHWIDGETICONSIZE, SWITCHWIDGETICONSIZE));

    update();
}
