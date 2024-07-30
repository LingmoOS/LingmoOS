/*
 *  SPDX-FileCopyrightText: 2008-2010 Dario Freddi <drf@kde.org>
 *  SPDX-FileCopyrightText: 2023 Jakob Petsovits <jpetso@petsovits.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "PowerKCM.h"

#include "ExternalServiceSettings.h"

// powerdevil/kcmodule/common
#include <PowerButtonActionModel.h>
#include <PowerProfileModel.h>
#include <SleepModeModel.h>

// powerdevil/daemon
#include <PowerDevilGlobalSettings.h>
#include <PowerDevilProfileSettings.h>
#include <controllers/lidcontroller.h>
#include <powerdevilenums.h>
#include <powerdevilpowermanagement.h>

// debug category for qCDebug()
#include <powerdevil_debug.h>

// KDE
#include <KLocalizedString>
#include <KPluginFactory>
#include <Kirigami/Platform/TabletModeWatcher>
#include <Solid/Battery>
#include <Solid/Device>

// Qt
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusServiceWatcher>
#include <QQuickItem>
#include <QQuickRenderControl>

K_PLUGIN_FACTORY_WITH_JSON(PowerDevilProfilesConfigFactory, "kcm_powerdevilprofilesconfig.json", {
    registerPlugin<PowerDevil::PowerKCM>();
    registerPlugin<PowerDevil::PowerConfigData>();
})

using namespace Qt::StringLiterals;

namespace PowerDevil
{

PowerConfigData::PowerConfigData(QObject *parent, const KPluginMetaData &metaData)
    : PowerConfigData(parent,
                      Kirigami::Platform::TabletModeWatcher::self()->isTabletMode() /*isMobile*/,
                      PowerDevil::PowerManagement::instance()->isVirtualMachine() /*isVM*/,
                      PowerDevil::PowerManagement::instance()->canSuspend(),
                      PowerDevil::PowerManagement::instance()->canHibernate())
{
    Q_UNUSED(metaData);
}

PowerConfigData::PowerConfigData(QObject *parent, bool isMobile, bool isVM, bool canSuspend, bool canHibernate)
    : KCModuleData(parent)
    , m_globalSettings(new GlobalSettings(canSuspend, canHibernate, this))
    , m_settingsAC(new ProfileSettings(u"AC"_s, isMobile, isVM, canSuspend, this))
    , m_settingsBattery(new ProfileSettings(u"Battery"_s, isMobile, isVM, canSuspend, this))
    , m_settingsLowBattery(new ProfileSettings(u"LowBattery"_s, isMobile, isVM, canSuspend, this))
{
    autoRegisterSkeletons();
}

PowerConfigData::~PowerConfigData()
{
}

GlobalSettings *PowerConfigData::global() const
{
    return m_globalSettings;
}

ProfileSettings *PowerConfigData::profileAC() const
{
    return m_settingsAC;
}

ProfileSettings *PowerConfigData::profileBattery() const
{
    return m_settingsBattery;
}

ProfileSettings *PowerConfigData::profileLowBattery() const
{
    return m_settingsLowBattery;
}

PowerKCM::PowerKCM(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
    , m_settings(new PowerConfigData(this, metaData))
    , m_externalServiceSettings(new ExternalServiceSettings(this))
    , m_supportsBatteryProfiles(false)
    , m_isPowerSupplyBatteryPresent(false)
    , m_isPeripheralBatteryPresent(false)
    , m_isLidPresent(false)
    , m_isPowerButtonPresent(false)
    , m_powerManagementServiceRegistered(false)
    , m_autoSuspendActionModel(new PowerButtonActionModel(this,
                                                          PowerManagement::instance(),
                                                          {
                                                              PowerButtonAction::NoAction,
                                                              PowerButtonAction::Sleep,
                                                              PowerButtonAction::Hibernate,
                                                              PowerButtonAction::Shutdown,
                                                          }))
    , m_batteryCriticalActionModel(new PowerButtonActionModel(this,
                                                              PowerManagement::instance(),
                                                              {
                                                                  PowerButtonAction::NoAction,
                                                                  PowerButtonAction::Sleep,
                                                                  PowerButtonAction::Hibernate,
                                                                  PowerButtonAction::Shutdown,
                                                              }))
    , m_powerButtonActionModel(new PowerButtonActionModel(this,
                                                          PowerManagement::instance(),
                                                          {
                                                              PowerButtonAction::NoAction,
                                                              PowerButtonAction::Sleep,
                                                              PowerButtonAction::Hibernate,
                                                              PowerButtonAction::Shutdown,
                                                              PowerButtonAction::LockScreen,
                                                              PowerButtonAction::PromptLogoutDialog,
                                                              PowerButtonAction::TurnOffScreen,
                                                          }))
    , m_lidActionModel(new PowerButtonActionModel(this,
                                                  PowerManagement::instance(),
                                                  {
                                                      PowerButtonAction::NoAction,
                                                      PowerButtonAction::Sleep,
                                                      PowerButtonAction::Hibernate,
                                                      PowerButtonAction::Shutdown,
                                                      PowerButtonAction::LockScreen,
                                                      PowerButtonAction::TurnOffScreen,
                                                  }))
    , m_sleepModeModel(new SleepModeModel(this, PowerManagement::instance()))
    , m_powerProfileModel(new PowerProfileModel(this))
{
    qmlRegisterUncreatableMetaObject(PowerDevil::staticMetaObject, "org.kde.powerdevil", 1, 0, "PowerDevil", QStringLiteral("For enums and flags only"));

    // External service settings
    connect(m_externalServiceSettings, &ExternalServiceSettings::settingsChanged, this, &PowerKCM::settingsChanged);
    connect(m_externalServiceSettings,
            &ExternalServiceSettings::isChargeStartThresholdSupportedChanged,
            this,
            &PowerKCM::isChargeStartThresholdSupportedChanged);
    connect(m_externalServiceSettings, &ExternalServiceSettings::isChargeStopThresholdSupportedChanged, this, &PowerKCM::isChargeStopThresholdSupportedChanged);
    connect(m_externalServiceSettings, &ExternalServiceSettings::isBatteryConservationModeSupportedChanged, this, &PowerKCM::isBatteryConservationModeSupportedChanged);
    connect(m_externalServiceSettings,
            &ExternalServiceSettings::chargeStopThresholdMightNeedReconnectChanged,
            this,
            &PowerKCM::chargeStopThresholdMightNeedReconnectChanged);

    // Solid device discovery
    const auto devices = Solid::Device::listFromType(Solid::DeviceInterface::Battery, QString());
    for (const Solid::Device &device : devices) {
        const Solid::Battery *b = qobject_cast<const Solid::Battery *>(device.asDeviceInterface(Solid::DeviceInterface::Battery));
        if (b->isPowerSupply()) {
            setPowerSupplyBatteryPresent(true);
            if (b->type() == Solid::Battery::PrimaryBattery || b->type() == Solid::Battery::UpsBattery) {
                setSupportsBatteryProfiles(true);
            }
        } else {
            setPeripheralBatteryPresent(true);
        }
    }

    // Look for PowerDevil's own service
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(u"org.kde.Solid.PowerManagement"_s,
                                                           QDBusConnection::sessionBus(),
                                                           QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
                                                           this);

    connect(watcher, &QDBusServiceWatcher::serviceRegistered, this, &PowerKCM::onServiceRegistered);
    connect(watcher, &QDBusServiceWatcher::serviceUnregistered, this, &PowerKCM::onServiceUnregistered);

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(u"org.kde.Solid.PowerManagement"_s)) {
        onServiceRegistered(u"org.kde.Solid.PowerManagement"_s);
    } else {
        onServiceUnregistered(u"org.kde.Solid.PowerManagement"_s);
    }

    // Load all the plugins, so we can tell the UI which ones are supported
    const QVector<KPluginMetaData> offers = KPluginMetaData::findPlugins(QStringLiteral("powerdevil/action"));

    for (const KPluginMetaData &offer : offers) {
        QString actionId = offer.value(QStringLiteral("X-KDE-PowerDevil-Action-ID"));
        bool supported = true;

        // Does it have a runtime requirement?
        if (offer.value(QStringLiteral("X-KDE-PowerDevil-Action-HasRuntimeRequirement"), false)) {
            qCDebug(POWERDEVIL) << offer.name() << " has a runtime requirement";

            QDBusMessage call = QDBusMessage::createMethodCall(u"org.kde.Solid.PowerManagement"_s,
                                                               u"/org/kde/Solid/PowerManagement"_s,
                                                               u"org.kde.Solid.PowerManagement"_s,
                                                               u"isActionSupported"_s);
            call.setArguments(QVariantList() << actionId);
            QDBusPendingReply<bool> reply = QDBusConnection::sessionBus().asyncCall(call);
            reply.waitForFinished();

            if (reply.isValid()) {
                if (!reply.value()) {
                    supported = false;
                    qCDebug(POWERDEVIL) << "The action " << actionId << " appears not to be supported by the core.";
                }
            } else {
                qCDebug(POWERDEVIL) << "There was a problem in contacting DBus!! Assuming the action is ok.";
            }
        }

        m_supportedActions.insert(actionId, supported);
    }
}

void PowerKCM::load()
{
    QWindow *renderWindowAsKAuthParent = QQuickRenderControl::renderWindowFor(mainUi()->window());
    m_externalServiceSettings->load(renderWindowAsKAuthParent);

    setLidPresent(LidController().isLidPresent());
    setPowerButtonPresent(true /* HACK This needs proper API to determine! */);

    KQuickManagedConfigModule::load();
}

void PowerKCM::save()
{
    KQuickManagedConfigModule::save();

    QWindow *renderWindowAsKAuthParent = QQuickRenderControl::renderWindowFor(mainUi()->window());
    m_externalServiceSettings->save(renderWindowAsKAuthParent);

    // Notify daemon
    QDBusMessage call = QDBusMessage::createMethodCall(u"org.kde.Solid.PowerManagement"_s,
                                                       u"/org/kde/Solid/PowerManagement"_s,
                                                       u"org.kde.Solid.PowerManagement"_s,
                                                       u"refreshStatus"_s);
    QDBusConnection::sessionBus().asyncCall(call);
}

bool PowerKCM::isSaveNeeded() const
{
    return m_externalServiceSettings->isSaveNeeded();
}

QVariantMap PowerKCM::supportedActions() const
{
    return m_supportedActions;
}

PowerConfigData *PowerKCM::settings() const
{
    return m_settings;
}

ExternalServiceSettings *PowerKCM::externalServiceSettings() const
{
    return m_externalServiceSettings;
}

QString PowerKCM::currentProfile() const
{
    return m_currentProfile;
}

void PowerKCM::setCurrentProfile(const QString &currentProfile)
{
    if (currentProfile == m_currentProfile) {
        return;
    }
    m_currentProfile = currentProfile;
    Q_EMIT currentProfileChanged();
}

bool PowerKCM::supportsBatteryProfiles() const
{
    return m_supportsBatteryProfiles;
}

void PowerKCM::setSupportsBatteryProfiles(bool supportsBatteryProfiles)
{
    if (supportsBatteryProfiles == m_supportsBatteryProfiles) {
        return;
    }
    m_supportsBatteryProfiles = supportsBatteryProfiles;
    Q_EMIT supportsBatteryProfilesChanged();
}

bool PowerKCM::isBatteryConservationModeSupported() const
{
    return m_externalServiceSettings->isBatteryConservationModeSupported();
}

bool PowerKCM::isChargeStartThresholdSupported() const
{
    return m_externalServiceSettings->isChargeStartThresholdSupported();
}

bool PowerKCM::isChargeStopThresholdSupported() const
{
    return m_externalServiceSettings->isChargeStopThresholdSupported();
}

bool PowerKCM::chargeStopThresholdMightNeedReconnect() const
{
    return m_externalServiceSettings->chargeStopThresholdMightNeedReconnect();
}

bool PowerKCM::isPowerSupplyBatteryPresent() const
{
    return m_isPowerSupplyBatteryPresent;
}

bool PowerKCM::isPeripheralBatteryPresent() const
{
    return m_isPeripheralBatteryPresent;
}

void PowerKCM::setPowerSupplyBatteryPresent(bool isBatteryPresent)
{
    if (isBatteryPresent == m_isPowerSupplyBatteryPresent) {
        return;
    }
    m_isPowerSupplyBatteryPresent = isBatteryPresent;
    Q_EMIT isPowerSupplyBatteryPresentChanged();
}

void PowerKCM::setPeripheralBatteryPresent(bool isBatteryPresent)
{
    if (isBatteryPresent == m_isPeripheralBatteryPresent) {
        return;
    }
    m_isPeripheralBatteryPresent = isBatteryPresent;
    Q_EMIT isPeripheralBatteryPresentChanged();
}

bool PowerKCM::isLidPresent() const
{
    return m_isLidPresent;
}

bool PowerKCM::isPowerButtonPresent() const
{
    return m_isPowerButtonPresent;
}

void PowerKCM::setLidPresent(bool isLidPresent)
{
    if (isLidPresent == m_isLidPresent) {
        return;
    }
    m_isLidPresent = isLidPresent;
    Q_EMIT isLidPresentChanged();
}

void PowerKCM::setPowerButtonPresent(bool isPowerButtonPresent)
{
    if (isPowerButtonPresent == m_isPowerButtonPresent) {
        return;
    }
    m_isPowerButtonPresent = isPowerButtonPresent;
    Q_EMIT isPowerButtonPresentChanged();
}

QObject *PowerKCM::autoSuspendActionModel() const
{
    return m_autoSuspendActionModel;
}

QObject *PowerKCM::batteryCriticalActionModel() const
{
    return m_batteryCriticalActionModel;
}

QObject *PowerKCM::powerButtonActionModel() const
{
    return m_powerButtonActionModel;
}

QObject *PowerKCM::lidActionModel() const
{
    return m_lidActionModel;
}

QObject *PowerKCM::sleepModeModel() const
{
    return m_sleepModeModel;
}

QObject *PowerKCM::powerProfileModel() const
{
    return m_powerProfileModel;
}

bool PowerKCM::powerManagementServiceRegistered() const
{
    return m_powerManagementServiceRegistered;
}

QString PowerKCM::powerManagementServiceErrorReason() const
{
    return m_powerManagementServiceErrorReason;
}

void PowerKCM::setPowerManagementServiceRegistered(bool registered)
{
    if (registered == m_powerManagementServiceRegistered) {
        return;
    }
    m_powerManagementServiceRegistered = registered;
    Q_EMIT powerManagementServiceRegisteredChanged();
}

void PowerKCM::setPowerManagementServiceErrorReason(const QString &reason)
{
    if (reason == m_powerManagementServiceErrorReason) {
        return;
    }
    m_powerManagementServiceErrorReason = reason;
    Q_EMIT powerManagementServiceErrorReasonChanged();
}

void PowerKCM::onServiceRegistered(const QString & /*service*/)
{
    QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral("org.kde.Solid.PowerManagement"),
                                                       QStringLiteral("/org/kde/Solid/PowerManagement"),
                                                       QStringLiteral("org.kde.Solid.PowerManagement"),
                                                       QStringLiteral("currentProfile"));
    QDBusPendingCallWatcher *currentProfileWatcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(call), this);

    QObject::connect(currentProfileWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QString> reply = *watcher;

        if (!reply.isError()) {
            setCurrentProfile(reply.value());
        }

        watcher->deleteLater();
    });

    setPowerManagementServiceRegistered(true);
}

void PowerKCM::onServiceUnregistered(const QString & /*service*/)
{
    setPowerManagementServiceErrorReason(i18n("The Power Management Service appears not to be running."));
    setPowerManagementServiceRegistered(false);
}

} // namespace PowerDevil

#include "PowerKCM.moc"

#include "moc_PowerKCM.cpp"
