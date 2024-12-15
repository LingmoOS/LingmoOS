// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window/interface/frameproxyinterface.h"
#include "safetyprotectionmodule.h"
#include "safetyprotectionwidget.h"
#include "window/modules/common/invokers/invokerfactory.h"

DEF_USING_NAMESPACE
DEF_USING_ANTIVIRUS_NAMESPACE

SafetyProtectionModule::SafetyProtectionModule(FrameProxyInterface *frameProxy, QObject *parent)
    : QObject(parent)
    , ModuleInterface(frameProxy)
    , m_safetyProtectionWidget(nullptr)
{
}

SafetyProtectionModule::~SafetyProtectionModule()
{
    if (nullptr != m_safetyProtectionWidget) {
        m_safetyProtectionWidget->deleteLater();
        m_safetyProtectionWidget = nullptr;
    }
}

void SafetyProtectionModule::preInitialize()
{
}

void SafetyProtectionModule::initialize()
{
}

const QString SafetyProtectionModule::name() const
{
    return MODULE_SAFETY_PROTECT_NAME;
}

void SafetyProtectionModule::active(int index)
{
    Q_UNUSED(index);
    m_safetyProtectionWidget = new SafetyProtectionWidget(new InvokerFactory(this));
    m_safetyProtectionWidget->show();
    m_frameProxy->pushWidget(this, m_safetyProtectionWidget);
}

void SafetyProtectionModule::deactive()
{
    if (nullptr != m_safetyProtectionWidget) {
        m_safetyProtectionWidget->deleteLater();
        m_safetyProtectionWidget = nullptr;
    }
}
