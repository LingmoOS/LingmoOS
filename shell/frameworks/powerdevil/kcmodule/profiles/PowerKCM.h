/*
 *  SPDX-FileCopyrightText: 2023 Jakob Petsovits <jpetso@petsovits.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <KCModuleData>
#include <KQuickManagedConfigModule>

#include <QList>
#include <QVariant>

class PowerButtonActionModel;
class PowerProfileModel;
class SleepModeModel;

namespace PowerDevil
{
class ProfileSettings;
class GlobalSettings;
class ExternalServiceSettings;

class PowerConfigData : public KCModuleData
{
    Q_OBJECT

    Q_PROPERTY(QObject *global READ global CONSTANT)
    Q_PROPERTY(QObject *profileAC READ profileAC CONSTANT)
    Q_PROPERTY(QObject *profileBattery READ profileBattery CONSTANT)
    Q_PROPERTY(QObject *profileLowBattery READ profileLowBattery CONSTANT)

public:
    explicit PowerConfigData(QObject *parent, const KPluginMetaData &metaData);
    explicit PowerConfigData(QObject *parent, bool isMobile, bool isVM, bool canSuspend, bool canHibernate);
    ~PowerConfigData() override;

    GlobalSettings *global() const;
    ProfileSettings *profileAC() const;
    ProfileSettings *profileBattery() const;
    ProfileSettings *profileLowBattery() const;

private:
    GlobalSettings *m_globalSettings;
    ProfileSettings *m_settingsAC;
    ProfileSettings *m_settingsBattery;
    ProfileSettings *m_settingsLowBattery;
};

class PowerKCM : public KQuickManagedConfigModule
{
    Q_OBJECT

    // e.g. {"BrightnessControl": true, "KeyboardBrightnessControl": false, "SuspendSession": true, ...}
    Q_PROPERTY(QVariantMap supportedActions READ supportedActions NOTIFY supportedActionsChanged)

    Q_PROPERTY(QObject *settings READ settings CONSTANT)
    Q_PROPERTY(QObject *externalServiceSettings READ externalServiceSettings CONSTANT)
    Q_PROPERTY(QString currentProfile READ currentProfile NOTIFY currentProfileChanged)
    Q_PROPERTY(bool supportsBatteryProfiles READ supportsBatteryProfiles NOTIFY supportsBatteryProfilesChanged)

    Q_PROPERTY(bool isPowerSupplyBatteryPresent READ isPowerSupplyBatteryPresent NOTIFY isPowerSupplyBatteryPresentChanged)
    Q_PROPERTY(bool isPeripheralBatteryPresent READ isPeripheralBatteryPresent NOTIFY isPeripheralBatteryPresentChanged)
    Q_PROPERTY(bool isLidPresent READ isLidPresent NOTIFY isLidPresentChanged)
    Q_PROPERTY(bool isPowerButtonPresent READ isPowerButtonPresent NOTIFY isPowerButtonPresentChanged)

    Q_PROPERTY(bool isBatteryConservationModeSupported READ isBatteryConservationModeSupported NOTIFY isBatteryConservationModeSupportedChanged)
    Q_PROPERTY(bool isChargeStartThresholdSupported READ isChargeStartThresholdSupported NOTIFY isChargeStartThresholdSupportedChanged)
    Q_PROPERTY(bool isChargeStopThresholdSupported READ isChargeStopThresholdSupported NOTIFY isChargeStopThresholdSupportedChanged)
    Q_PROPERTY(bool chargeStopThresholdMightNeedReconnect READ chargeStopThresholdMightNeedReconnect NOTIFY chargeStopThresholdMightNeedReconnectChanged)

    Q_PROPERTY(bool powerManagementServiceRegistered READ powerManagementServiceRegistered NOTIFY powerManagementServiceRegisteredChanged)
    Q_PROPERTY(QString powerManagementServiceErrorReason READ powerManagementServiceErrorReason NOTIFY powerManagementServiceErrorReasonChanged)

    Q_PROPERTY(QObject *autoSuspendActionModel READ autoSuspendActionModel CONSTANT)
    Q_PROPERTY(QObject *batteryCriticalActionModel READ batteryCriticalActionModel CONSTANT)
    Q_PROPERTY(QObject *powerButtonActionModel READ powerButtonActionModel CONSTANT)
    Q_PROPERTY(QObject *lidActionModel READ lidActionModel CONSTANT)
    Q_PROPERTY(QObject *sleepModeModel READ sleepModeModel CONSTANT)
    Q_PROPERTY(QObject *powerProfileModel READ powerProfileModel CONSTANT)

public:
    PowerKCM(QObject *parent, const KPluginMetaData &metaData);

    bool isSaveNeeded() const override;

    QVariantMap supportedActions() const;

    PowerConfigData *settings() const;
    ExternalServiceSettings *externalServiceSettings() const;
    QString currentProfile() const;
    bool supportsBatteryProfiles() const;

    bool isPowerSupplyBatteryPresent() const;
    bool isPeripheralBatteryPresent() const;
    bool isLidPresent() const;
    bool isPowerButtonPresent() const;

    bool isBatteryConservationModeSupported() const;
    bool isChargeStartThresholdSupported() const;
    bool isChargeStopThresholdSupported() const;
    bool chargeStopThresholdMightNeedReconnect() const;

    QObject *autoSuspendActionModel() const;
    QObject *batteryCriticalActionModel() const;
    QObject *powerButtonActionModel() const;
    QObject *lidActionModel() const;
    QObject *sleepModeModel() const;
    QObject *powerProfileModel() const;

    bool powerManagementServiceRegistered() const;
    QString powerManagementServiceErrorReason() const;

public Q_SLOTS:
    void load() override;
    void save() override;

Q_SIGNALS:
    void supportedActionsChanged();

    void currentProfileChanged();
    void supportsBatteryProfilesChanged();

    void isPowerSupplyBatteryPresentChanged();
    void isPeripheralBatteryPresentChanged();
    void isLidPresentChanged();
    void isPowerButtonPresentChanged();

    void isBatteryConservationModeSupportedChanged();
    void isChargeStartThresholdSupportedChanged();
    void isChargeStopThresholdSupportedChanged();
    void chargeStopThresholdMightNeedReconnectChanged();

    void powerManagementServiceRegisteredChanged();
    void powerManagementServiceErrorReasonChanged();

private:
    void setCurrentProfile(const QString &currentProfile);
    void setSupportsBatteryProfiles(bool);

    void setPowerSupplyBatteryPresent(bool);
    void setPeripheralBatteryPresent(bool);
    void setLidPresent(bool);
    void setPowerButtonPresent(bool);

    void setPowerManagementServiceRegistered(bool);
    void setPowerManagementServiceErrorReason(const QString &);
    void onServiceRegistered(const QString &service);
    void onServiceUnregistered(const QString &service);

    QVariantMap m_supportedActions;

    PowerConfigData *m_settings;
    ExternalServiceSettings *m_externalServiceSettings;
    QString m_currentProfile;
    bool m_supportsBatteryProfiles;

    bool m_isPowerSupplyBatteryPresent;
    bool m_isPeripheralBatteryPresent;
    bool m_isLidPresent;
    bool m_isPowerButtonPresent;

    bool m_powerManagementServiceRegistered;
    QString m_powerManagementServiceErrorReason;

    PowerButtonActionModel *m_autoSuspendActionModel;
    PowerButtonActionModel *m_batteryCriticalActionModel;
    PowerButtonActionModel *m_powerButtonActionModel;
    PowerButtonActionModel *m_lidActionModel;
    SleepModeModel *m_sleepModeModel;
    PowerProfileModel *m_powerProfileModel;
};

} // namespace PowerDevil
