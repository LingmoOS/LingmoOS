// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QApplication>
#include <QDesktopWidget>

#include <string>

#include "../mainwindow.h"
#include "upgradecheckresultwidget.h"
#include "../../core/utils.h"

UpgradeCheckResultWidget::UpgradeCheckResultWidget(QWidget *parent)
    : QWidget(parent)
    , m_dbusWorker(DBusWorker::getInstance(this))
    , m_mainVerticalLayout(new QVBoxLayout(this))
    , m_resultContentLayout(new QStackedLayout(this))
    , m_titleIconLabel(new IconLabel(this))
    , m_titleTextLabel(new DLabel(this))
    , m_conditionCheckWidget(new ConditionChecklistWidget(this))
{
    initUI();
    initConnections();
}

void UpgradeCheckResultWidget::initUI()
{
    m_mainVerticalLayout->setObjectName(QStringLiteral("mainVerticalLayout"));
    m_mainVerticalLayout->setContentsMargins(0, 24, 0, 0);
    m_titleIconLabel->addSpacing(10);
    m_titleIconLabel->addWidget(m_titleTextLabel);

    // Check result title
    m_titleTextLabel->setForegroundRole(DPalette::TextTitle);
    DFontSizeManager::instance()->bind(m_titleTextLabel, DFontSizeManager::T3, QFont::DemiBold);
    m_titleTextLabel->setAlignment(Qt::AlignCenter);

    m_resultContentLayout->addWidget(m_conditionCheckWidget);

    m_mainVerticalLayout->addWidget(m_titleIconLabel, 0, Qt::AlignCenter);
    m_mainVerticalLayout->addSpacing(7);
    m_mainVerticalLayout->addLayout(m_resultContentLayout);

    m_isCheckPassed = m_conditionCheckWidget->passed();

    setLayout(m_mainVerticalLayout);
}

void UpgradeCheckResultWidget::updateIcons()
{
    if (m_conditionCheckWidget->passed())
    {
        if (isDarkMode())
        {
            m_titleIconLabel->setIcon(":/icons/check_passed_dark_icon.svg", 60, 60);
            m_titleTextLabel->setText(tr("Passed"));
        }
        else
        {
            m_titleIconLabel->setIcon(":/icons/check_passed_icon.svg", 60, 60);
            m_titleTextLabel->setText(tr("Passed"));
        }
    }
    else
    {
        if (isDarkMode())
        {
            m_titleIconLabel->setIcon(":/icons/check_failed_dark_icon.svg", 60, 60);
            m_titleTextLabel->setText(tr("Failed"));
        }
        else
        {
            m_titleIconLabel->setIcon(":/icons/check_failed_icon.svg", 60, 60);
            m_titleTextLabel->setText(tr("Failed"));
        }
    }
}

void UpgradeCheckResultWidget::onUpdateUI()
{
    m_isCheckPassed = m_conditionCheckWidget->passed();

    updateIcons();

    Q_EMIT updateCompleted();
}

void UpgradeCheckResultWidget::initConnections()
{
    connect(this, &UpgradeCheckResultWidget::updateUI, m_conditionCheckWidget, &ConditionChecklistWidget::updateUI);
    connect(m_conditionCheckWidget, SIGNAL(updateCompleted()), this, SLOT(onUpdateUI()));
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &UpgradeCheckResultWidget::updateIcons);
}
