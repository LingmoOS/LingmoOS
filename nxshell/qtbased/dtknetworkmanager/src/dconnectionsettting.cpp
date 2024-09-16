// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dconnectionsetting.h"
#include "dconnectionsettting_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DUnexpected;
using DCORE_NAMESPACE::emplace_tag;

DConnectionSettingPrivate::DConnectionSettingPrivate(const quint64 id, DConnectionSetting *parent)
    : QObject(parent)
#ifdef USE_FAKE_INTERFACE
    , m_setting(new DConnectionSettingInterface("/com/deepin/FakeNetworkManager/Settings/" + QByteArray::number(id), this))
#else
    , m_setting(new DConnectionSettingInterface("/org/freedesktop/NetworkManager/Settings/" + QByteArray::number(id), this))
#endif
{
}

DConnectionSetting::DConnectionSetting(const quint64 id, QObject *parent)
    : QObject(parent)
    , d_ptr(new DConnectionSettingPrivate(id, this))
{
    Q_D(const DConnectionSetting);

    connect(d->m_setting, &DConnectionSettingInterface::UnsavedChanged, this, &DConnectionSetting::unsavedChanged);

    connect(d->m_setting, &DConnectionSettingInterface::FilenameChanged, this, [this](const QString &filename) {
        Q_EMIT this->filenameChanged(filename);
    });

    connect(d->m_setting, &DConnectionSettingInterface::FlagsChanged, this, [this](const quint32 flags) {
        Q_EMIT this->flagsChanged(static_cast<NMSCFlags>(flags));
    });

    connect(d->m_setting, &DConnectionSettingInterface::Removed, this, &DConnectionSetting::Removed);

    connect(d->m_setting, &DConnectionSettingInterface::Updated, this, &DConnectionSetting::Updated);
}

DConnectionSetting::~DConnectionSetting() = default;

bool DConnectionSetting::unsaved() const
{
    Q_D(const DConnectionSetting);
    return d->m_setting->unsaved();
}

QUrl DConnectionSetting::filename() const
{
    Q_D(const DConnectionSetting);
    return d->m_setting->filename();
}

DConnectionSetting::NMSCFlags DConnectionSetting::flags() const
{
    Q_D(const DConnectionSetting);
    return NMSCFlags{d->m_setting->flags()};
}

DExpected<void> DConnectionSetting::updateSetting(const SettingDesc &settings) const
{
    Q_D(const DConnectionSetting);
    auto reply = d->m_setting->UpdateSetting(settings);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DExpected<void> DConnectionSetting::deleteSetting() const
{
    Q_D(const DConnectionSetting);
    auto reply = d->m_setting->deleteSetting();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DExpected<SettingDesc> DConnectionSetting::settings() const
{
    Q_D(const DConnectionSetting);
    auto reply = d->m_setting->getSettigns();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return reply.value();
}

DNETWORKMANAGER_END_NAMESPACE
