/*
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kabstractconfigmodule.h"
#include <kpluginmetadata.h>

class KAbstractConfigModulePrivate
{
public:
    KAbstractConfigModulePrivate(const KPluginMetaData &data)
        : m_data(data)
    {
    }
    const KPluginMetaData m_data;

    QString m_rootOnlyMessage;
    QString m_errorString;

    bool m_useRootOnlyMessage = false;
    bool m_needsSave = false;
    bool m_representsDefaults = false;
    bool m_defaultsIndicatorVisible = false;
    QString m_authActionName;
    KAbstractConfigModule::Buttons m_buttons = KAbstractConfigModule::Help | KAbstractConfigModule::Default | KAbstractConfigModule::Apply;
};

KAbstractConfigModule::KAbstractConfigModule(QObject *parent, const KPluginMetaData &metaData)
    : QObject(parent)
    , d(new KAbstractConfigModulePrivate(metaData))
{
}

KAbstractConfigModule::~KAbstractConfigModule() = default;

KAbstractConfigModule::Buttons KAbstractConfigModule::buttons() const
{
    return d->m_buttons;
}

void KAbstractConfigModule::setButtons(const KAbstractConfigModule::Buttons buttons)
{
    if (d->m_buttons == buttons) {
        return;
    }

    d->m_buttons = buttons;
    Q_EMIT buttonsChanged();
}

bool KAbstractConfigModule::needsAuthorization() const
{
    return !d->m_authActionName.isEmpty();
}

QString KAbstractConfigModule::name() const
{
    return d->m_data.name();
}

QString KAbstractConfigModule::description() const
{
    return d->m_data.description();
}

void KAbstractConfigModule::setAuthActionName(const QString &name)
{
    if (d->m_authActionName == name) {
        return;
    }

    d->m_authActionName = name;
    Q_EMIT authActionNameChanged();
}

QString KAbstractConfigModule::authActionName() const
{
    return d->m_authActionName;
}

void KAbstractConfigModule::load()
{
    setNeedsSave(false);
}

void KAbstractConfigModule::save()
{
    setNeedsSave(false);
}

void KAbstractConfigModule::defaults()
{
}

void KAbstractConfigModule::setNeedsSave(bool needs)
{
    if (needs == d->m_needsSave) {
        return;
    }

    d->m_needsSave = needs;
    Q_EMIT needsSaveChanged();
}

bool KAbstractConfigModule::needsSave() const
{
    return d->m_needsSave;
}

void KAbstractConfigModule::setRepresentsDefaults(bool defaults)
{
    if (defaults == d->m_representsDefaults) {
        return;
    }

    d->m_representsDefaults = defaults;
    Q_EMIT representsDefaultsChanged();
}

bool KAbstractConfigModule::representsDefaults() const
{
    return d->m_representsDefaults;
}

bool KAbstractConfigModule::defaultsIndicatorsVisible() const
{
    return d->m_defaultsIndicatorVisible;
}

void KAbstractConfigModule::setDefaultsIndicatorsVisible(bool visible)
{
    if (d->m_defaultsIndicatorVisible == visible) {
        return;
    }
    d->m_defaultsIndicatorVisible = visible;
    Q_EMIT defaultsIndicatorsVisibleChanged();
}

KPluginMetaData KAbstractConfigModule::metaData() const
{
    return d->m_data;
}

#include "moc_kabstractconfigmodule.cpp"
