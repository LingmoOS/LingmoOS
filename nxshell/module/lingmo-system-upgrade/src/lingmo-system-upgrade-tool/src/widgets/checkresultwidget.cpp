// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DFontSizeManager>

#include <QSpacerItem>

#include "checkresultwidget.h"

#define ICON_SIZE 15
#define RESULT_TEXT_SIZE 9

CheckResultWidget::CheckResultWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(new QVBoxLayout(this))
    , m_tipLayout(new QHBoxLayout(this))
    , m_checkResultIconLabel(new IconLabel(this))
    , m_requirementTitleLabel(new DLabel(this))
    , m_requirementTipLabel(new SimpleLabel)
    , m_resultTextLabel(new DLabel(this))
    , m_statusLabel(new DLabel(this))
{
    initUI();
}

void CheckResultWidget::initUI()
{
    DFontSizeManager::instance()->bind(m_resultTextLabel, DFontSizeManager::T8, QFont::Normal);

    m_requirementTitleLabel->setForegroundRole(DPalette::TextTitle);
    DFontSizeManager::instance()->bind(m_requirementTitleLabel, DFontSizeManager::T6, QFont::Medium);

    m_requirementTipLabel->setForegroundRole(DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_requirementTipLabel, DFontSizeManager::T8, QFont::Thin);

    m_checkResultIconLabel->addSpacerItem(new QSpacerItem(8, 0));
    m_checkResultIconLabel->addWidget(m_requirementTitleLabel);
    m_checkResultIconLabel->addSpacing(666);
    m_checkResultIconLabel->addWidget(m_resultTextLabel);
    m_checkResultIconLabel->addSpacerItem(new QSpacerItem(8, 0));
    m_checkResultIconLabel->addWidget(m_statusLabel);
    m_checkResultIconLabel->addSpacing(10);
    m_checkResultIconLabel->layout()->setContentsMargins(0, 0, 0, 0);

    m_tipLayout->setContentsMargins(0, 0, 0, 0);
    m_tipLayout->addSpacing(26);
    m_tipLayout->addWidget(m_requirementTipLabel);
    m_tipLayout->setSpacing(0);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_checkResultIconLabel);
    m_mainLayout->addLayout(m_tipLayout);

    setLayout(m_mainLayout);
}

QVBoxLayout *CheckResultWidget::getMainLayout() const
{
    return m_mainLayout;
}

void CheckResultWidget::setMainLayout(QVBoxLayout *value)
{
    m_mainLayout = value;
}

void CheckResultWidget::setIcon(QString iconPath)
{
    m_checkResultIconLabel->setIcon(iconPath);
}

void CheckResultWidget::setIcon(QString iconPath, int w, int h)
{
    m_checkResultIconLabel->setIcon(iconPath, w, h);
}

void CheckResultWidget::setStatus(CheckResultType status)
{
    switch(status)
    {
    case CheckResultType::PASSED:
    {
        m_statusLabel->setToolTip(QString(""));
        m_resultTextLabel->setForegroundRole(DPalette::ToolTipText);
    }
    break;
    case CheckResultType::FAILED:
    {
        m_resultTextLabel->setForegroundRole(DPalette::TextWarning);
    }
    break;
    default:
    {
        m_statusLabel->setToolTip(m_toolTipText);
        m_resultTextLabel->setForegroundRole(DPalette::ToolTipText);
    }
    }
    m_statusLabel->setPixmap(getIcon(status).pixmap(ICON_SIZE * 2, ICON_SIZE * 2).scaled(ICON_SIZE, ICON_SIZE));
}

void CheckResultWidget::setTitle(QString text)
{
    m_requirementTitleLabel->setText(text);
}

void CheckResultWidget::setToolTip(QString text)
{
    m_toolTipText = text;
}

void CheckResultWidget::setRequirement(QString text)
{
    m_requirementTipLabel->setText(text);
}

void CheckResultWidget::setResult(QString text)
{
    m_resultTextLabel->setText(text);
}

void CheckResultWidget::scale(int w, int h)
{
    m_checkResultIconLabel->scale(w, h);
}

void CheckResultWidget::addSpacerItem(QSpacerItem *item)
{
    m_checkResultIconLabel->addSpacerItem(item);
}

void CheckResultWidget::addWidget(QWidget *widget)
{
    m_checkResultIconLabel->addWidget(widget);
}
