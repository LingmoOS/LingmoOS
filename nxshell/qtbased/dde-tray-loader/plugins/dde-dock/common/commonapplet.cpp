// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "commonapplet.h"
#include "constants.h"

#include <DSwitchButton>
#include <DBlurEffectWidget>
#include <DFontSizeManager>

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

CommonApplet::CommonApplet(QWidget *parent)
    : QWidget(parent)
    , m_title(new DLabel(this))
    , m_switchBtn(new DSwitchButton(this))
    , m_settingButton(new JumpSettingButton(this))
{
    m_switchBtn->setFocusPolicy(Qt::NoFocus);
    setFixedWidth(Dock::DOCK_POPUP_WIDGET_WIDTH);

    DFontSizeManager::instance()->bind(m_title, DFontSizeManager::T5);
    m_title->setForegroundRole(QPalette::BrightText);

    QHBoxLayout *appletLayout = new QHBoxLayout;
    appletLayout->setMargin(0);
    appletLayout->setSpacing(0);
    appletLayout->setContentsMargins(20, 0, 10, 0);
    appletLayout->addWidget(m_title);
    appletLayout->addStretch();
    appletLayout->addWidget(m_switchBtn);

    QHBoxLayout *settingLayout = new QHBoxLayout;
    settingLayout->addWidget(m_settingButton);
    settingLayout->setContentsMargins(10, 0, 10, 0);
    auto mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addLayout(appletLayout, 0);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(settingLayout, 0);
    mainLayout->setContentsMargins(0, 10, 0, 10);
    setLayout(mainLayout);

    connect(m_switchBtn, &DSwitchButton::checkedChanged, this, &CommonApplet::enableChanged);
    connect(m_settingButton, &JumpSettingButton::showPageRequestWasSended, this, &CommonApplet::requestHideApplet);
}

void CommonApplet::setEnabled(bool enable)
{
    m_switchBtn->blockSignals(true);
    m_switchBtn->setChecked(enable);
    m_switchBtn->blockSignals(false);
}

void CommonApplet::setTitle(const QString &title)
{
    m_title->setText(title);
}

void CommonApplet::setIcon(const QIcon &icon)
{
    m_settingButton->setIcon(icon);
}

void CommonApplet::setDescription(const QString &description)
{
    m_settingButton->setDescription(description);
}

void CommonApplet::setDccPage(const QString &module, const QString &page)
{
    m_settingButton->setDccPage(module, page);
}

void CommonApplet::hideSettingButton()
{
    m_settingButton->setVisible(false);
}