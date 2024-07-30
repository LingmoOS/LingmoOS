/*
    SPDX-FileCopyrightText: 2020 Kevin Ottens <kevin.ottens@enioka.com>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "settingstateproxy.h"
#include "kcmutils_debug.h"

#include <QDebug>
#include <QMetaMethod>

KCoreConfigSkeleton *SettingStateProxy::configObject() const
{
    return m_configObject;
}

void SettingStateProxy::setConfigObject(KCoreConfigSkeleton *configObject)
{
    if (m_configObject == configObject) {
        return;
    }

    if (m_configObject) {
        m_configObject->disconnect(this);
    }

    m_configObject = configObject;
    Q_EMIT configObjectChanged();
    updateState();
    connectSetting();
}

QString SettingStateProxy::settingName() const
{
    return m_settingName;
}

void SettingStateProxy::setSettingName(const QString &settingName)
{
    if (m_settingName == settingName) {
        return;
    }

    if (m_configObject) {
        m_configObject->disconnect(this);
    }

    m_settingName = settingName;
    Q_EMIT settingNameChanged();
    updateState();
    connectSetting();
}

bool SettingStateProxy::isImmutable() const
{
    return m_immutable;
}

bool SettingStateProxy::isDefaulted() const
{
    return m_defaulted;
}

void SettingStateProxy::updateState()
{
    const auto item = m_configObject ? m_configObject->findItem(m_settingName) : nullptr;
    const auto immutable = item ? item->isImmutable() : false;
    const auto defaulted = item ? item->isDefault() : true;

    if (m_immutable != immutable) {
        m_immutable = immutable;
        Q_EMIT immutableChanged();
    }

    if (m_defaulted != defaulted) {
        m_defaulted = defaulted;
        Q_EMIT defaultedChanged();
    }
}

void SettingStateProxy::connectSetting()
{
    const auto item = m_configObject ? m_configObject->findItem(m_settingName) : nullptr;
    if (!item) {
        return;
    }

    const auto updateStateSlotIndex = metaObject()->indexOfMethod("updateState()");
    Q_ASSERT(updateStateSlotIndex >= 0);
    const auto updateStateSlot = metaObject()->method(updateStateSlotIndex);
    Q_ASSERT(updateStateSlot.isValid());

    const auto itemHasSignals = dynamic_cast<KConfigCompilerSignallingItem *>(item) || dynamic_cast<KPropertySkeletonItem *>(item);
    if (!itemHasSignals) {
        qCWarning(KCMUTILS_LOG) << "Attempting to use SettingStateProxy with a non signalling item:" << m_settingName;
        return;
    }

    const auto propertyName = [this] {
        auto name = m_settingName;
        if (name.at(0).isUpper()) {
            name[0] = name[0].toLower();
        }
        return name.toUtf8();
    }();

    const auto metaObject = m_configObject->metaObject();
    const auto propertyIndex = metaObject->indexOfProperty(propertyName.constData());
    Q_ASSERT(propertyIndex >= 0);
    const auto property = metaObject->property(propertyIndex);
    Q_ASSERT(property.isValid());
    if (!property.hasNotifySignal()) {
        qCWarning(KCMUTILS_LOG) << "Attempting to use SettingStateProxy with a non notifying property:" << propertyName;
        return;
    }

    const auto changedSignal = property.notifySignal();
    Q_ASSERT(changedSignal.isValid());
    connect(m_configObject, changedSignal, this, updateStateSlot);
    connect(m_configObject, &KCoreConfigSkeleton::configChanged, this, &SettingStateProxy::updateState);
}

#include "moc_settingstateproxy.cpp"
