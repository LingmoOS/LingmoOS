/*
 * SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>
 * SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "powerprofile.h"

#include "power_profiles_interface.h"
#include "powerprofileadaptor.h"
#include "properties_interface.h"

#include <PowerDevilProfileSettings.h>
#include <powerdevil_debug.h>

#include <KPluginFactory>

using namespace PowerDevil::BundledActions;

K_PLUGIN_CLASS_WITH_JSON(PowerProfile, "powerdevilpowerprofileaction.json")

static const QString activeProfileProperty = QStringLiteral("ActiveProfile");
static const QString profilesProperty = QStringLiteral("Profiles");
static const QString performanceInhibitedProperty = QStringLiteral("PerformanceInhibited");
static const QString performanceDegradedProperty = QStringLiteral("PerformanceDegraded");
static const QString profileHoldsProperty = QStringLiteral("ActiveProfileHolds");

static const QString ppdName = QStringLiteral("net.hadess.PowerProfiles");
static const QString ppdPath = QStringLiteral("/net/hadess/PowerProfiles");

PowerProfile::PowerProfile(QObject *parent)
    : Action(parent)
    , m_powerProfilesInterface(new NetHadessPowerProfilesInterface(ppdName, ppdPath, QDBusConnection::systemBus(), this))
    , m_powerProfilesPropertiesInterface(new OrgFreedesktopDBusPropertiesInterface(ppdName, ppdPath, QDBusConnection::systemBus(), this))
    , m_holdWatcher(new QDBusServiceWatcher(QString(), QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForUnregistration, this))
{
    new PowerProfileAdaptor(this);

    connect(m_holdWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &PowerProfile::serviceUnregistered);
    connect(m_powerProfilesPropertiesInterface, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged, this, &PowerProfile::propertiesChanged);
    connect(m_powerProfilesInterface, &NetHadessPowerProfilesInterface::ProfileReleased, this, [this](unsigned int cookie) {
        auto it = std::find(m_holdMap.begin(), m_holdMap.end(), cookie);
        if (it != m_holdMap.end()) {
            if (m_holdMap.count(it.key()) == 1) {
                m_holdWatcher->removeWatchedService(it.key());
            }
            m_holdMap.erase(it);
        }
    });

    auto watcher = new QDBusPendingCallWatcher(m_powerProfilesPropertiesInterface->GetAll(m_powerProfilesInterface->interface()));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
        watcher->deleteLater();
        QDBusPendingReply<QVariantMap> reply = *watcher;
        if (watcher->isError()) {
            return;
        }
        readProperties(reply.value());
    });
    qDBusRegisterMetaType<QList<QVariantMap>>();
}

PowerProfile::~PowerProfile() = default;

void PowerProfile::onProfileLoad(const QString & /*previousProfile*/, const QString & /*newProfile*/)
{
    if (!m_configuredProfile.isEmpty()) {
        setProfile(m_configuredProfile);
    }
}

void PowerProfile::triggerImpl(const QVariantMap &args)
{
    Q_UNUSED(args);
}

bool PowerProfile::loadAction(const PowerDevil::ProfileSettings &profileSettings)
{
    QString newConfiguredProfile = profileSettings.powerProfile();
    if (m_configuredProfile != newConfiguredProfile) {
        m_configuredProfile = newConfiguredProfile;
        Q_EMIT configuredProfileChanged(m_configuredProfile);
    }
    return !m_configuredProfile.isEmpty();
}

bool PowerProfile::isSupported()
{
    return QDBusConnection::systemBus().interface()->activatableServiceNames().value().contains(ppdName)
        || QDBusConnection::systemBus().interface()->isServiceRegistered(ppdName).value();
}

QStringList PowerProfile::profileChoices() const
{
    return m_profileChoices;
}

QString PowerProfile::configuredProfile() const
{
    return m_configuredProfile;
}

QString PowerProfile::currentProfile() const
{
    return m_currentProfile;
}

QString PowerProfile::performanceDegradedReason() const
{
    return m_degradationReason;
}

QString PowerProfile::performanceInhibitedReason() const
{
    return m_performanceInhibitedReason;
}

QList<QVariantMap> PowerProfile::profileHolds() const
{
    return m_profileHolds;
}

void PowerProfile::setProfile(const QString &profile)
{
    auto call = m_powerProfilesPropertiesInterface->Set(m_powerProfilesInterface->interface(), activeProfileProperty, QDBusVariant(profile));
    if (calledFromDBus()) {
        setDelayedReply(true);
        const auto msg = message();
        auto watcher = new QDBusPendingCallWatcher(call);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [msg, watcher] {
            watcher->deleteLater();
            if (watcher->isError()) {
                QDBusConnection::sessionBus().send(msg.createErrorReply(watcher->error()));
            } else {
                QDBusConnection::sessionBus().send(msg.createReply());
            }
        });
    }
}

unsigned int PowerProfile::holdProfile(const QString &profile, const QString &reason, const QString &applicationId)
{
    if (!m_profileChoices.contains(profile)) {
        sendErrorReply(QDBusError::InvalidArgs, QStringLiteral("%1 is not a valid profile").arg(profile));
        return 0; // ignored by QtDBus
    }
    setDelayedReply(true);
    const auto msg = message();
    auto call = m_powerProfilesInterface->HoldProfile(profile, reason, applicationId);
    auto watcher = new QDBusPendingCallWatcher(call);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [msg, watcher, this] {
        watcher->deleteLater();
        QDBusPendingReply<unsigned int> reply = *watcher;
        if (reply.isError()) {
            QDBusConnection::sessionBus().send(msg.createErrorReply(watcher->error()));
        } else {
            m_holdWatcher->addWatchedService(msg.service());
            m_holdMap.insert(msg.service(), reply.value());
            QDBusConnection::sessionBus().send(msg.createReply(reply.value()));
        }
    });
    return 0; // ignored by QtDBus
}

void PowerProfile::releaseProfile(unsigned int cookie)
{
    setDelayedReply(true);
    const auto msg = message();
    auto call = m_powerProfilesInterface->ReleaseProfile(cookie);
    auto watcher = new QDBusPendingCallWatcher(call);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [msg, watcher, this] {
        watcher->deleteLater();
        if (watcher->isError()) {
            QDBusConnection::sessionBus().send(msg.createErrorReply(watcher->error()));
        } else {
            m_holdMap.remove(msg.service(), msg.arguments()[0].toUInt());
            if (m_holdMap.count(msg.service())) {
                m_holdWatcher->removeWatchedService(msg.service());
            }
            QDBusConnection::sessionBus().send(msg.createReply());
        }
    });
}

void PowerProfile::serviceUnregistered(const QString &name)
{
    const auto cookies = m_holdMap.equal_range(name);
    for (auto it = cookies.first; it != cookies.second; ++it) {
        m_powerProfilesInterface->ReleaseProfile(*it);
        m_holdMap.erase(it);
    }
    m_holdWatcher->removeWatchedService(name);
}

void PowerProfile::readProperties(const QVariantMap &properties)
{
    if (properties.contains(activeProfileProperty)) {
        QString newCurrentProfile = properties[activeProfileProperty].toString();
        if (m_currentProfile != newCurrentProfile) {
            m_currentProfile = newCurrentProfile;
            Q_EMIT currentProfileChanged(m_currentProfile);
        }
    }

    if (properties.contains(profilesProperty)) {
        QList<QVariantMap> profiles;
        properties[profilesProperty].value<QDBusArgument>() >> profiles;
        QStringList newProfileChoices;
        if (!profiles.isEmpty() && profiles.first()[QStringLiteral("Driver")] != QLatin1String("placeholder")) {
            std::transform(profiles.cbegin(), profiles.cend(), std::back_inserter(newProfileChoices), [](const QVariantMap &dict) {
                return dict[QStringLiteral("Profile")].toString();
            });
        }
        if (m_profileChoices != newProfileChoices) {
            m_profileChoices = newProfileChoices;
            Q_EMIT profileChoicesChanged(m_profileChoices);
        }
    }

    if (properties.contains(performanceInhibitedProperty)) {
        QString newReason = properties[performanceInhibitedProperty].toString();
        if (m_performanceInhibitedReason != newReason) {
            m_performanceInhibitedReason = newReason;
            Q_EMIT performanceInhibitedReasonChanged(m_performanceInhibitedReason);
        }
    }

    if (properties.contains(performanceDegradedProperty)) {
        QString newDegradationReason = properties[performanceDegradedProperty].toString();
        if (m_degradationReason != newDegradationReason) {
            m_degradationReason = newDegradationReason;
            Q_EMIT performanceDegradedReasonChanged(m_degradationReason);
        }
    }

    if (properties.contains(profileHoldsProperty)) {
        QList<QVariantMap> newProfileHolds;
        properties[profileHoldsProperty].value<QDBusArgument>() >> newProfileHolds;
        if (m_profileHolds != newProfileHolds) {
            m_profileHolds = newProfileHolds;
            Q_EMIT profileHoldsChanged(m_profileHolds);
        }
    }
}

void PowerProfile::propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    Q_UNUSED(invalidated)
    if (interface != m_powerProfilesInterface->interface()) {
        return;
    }
    readProperties(changed);
}

#include "powerprofile.moc"

#include "moc_powerprofile.cpp"
