// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "abstractsettings.h"
#include "sections/genericsection.h"
#include "sections/abstractsection.h"
#include "editpage/connectioneditpage.h"

#include <QVBoxLayout>
#include <QDebug>

#include <networkmanagerqt/settings.h>

using namespace NetworkManager;

AbstractSettings::AbstractSettings(ConnectionSettings::Ptr connSettings, QWidget *parent)
    : QWidget(parent)
    , m_connSettings(connSettings)
    , m_sectionsLayout(new QVBoxLayout(this))
{
    m_sectionsLayout->setSpacing(10);
    m_sectionsLayout->setMargin(0);
    m_sectionsLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(m_sectionsLayout);
}

AbstractSettings::~AbstractSettings()
{
}

bool AbstractSettings::allInputValid()
{
    for (AbstractSection *section : m_settingSections) {
        if (section->isVisible()) {
            if (!section->allInputValid())
                return false;
        }
    }

    return true;
}

void AbstractSettings::saveSettings()
{
    for (auto section : m_settingSections) {
        if (section->isVisible())
            section->saveSettings();
    }
    resetConnectionInterfaceName();
}

bool AbstractSettings::isAutoConnect()
{
    GenericSection *genericSection = dynamic_cast<GenericSection *>(m_settingSections.first());

    if (genericSection)
        return genericSection->autoConnectChecked();

    // auto connect after save connection, like hotspot connection
    return true;
}

void AbstractSettings::resetConnectionInterfaceName()
{
    if (!m_connSettings->interfaceName().isEmpty()) {
        return;
    }

    if (!setInterfaceName() || ConnectionEditPage::devicePath().isEmpty()) {
        // 不设置 interfaceName
        return;
    }

    Device::Ptr dev = findNetworkInterface(ConnectionEditPage::devicePath());
    if (dev)
        m_connSettings->setInterfaceName(dev->interfaceName());
}
