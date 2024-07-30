/*
 *  SPDX-FileCopyrightText: 2023 Jakob Petsovits <jpetso@petsovits.com>
 *  SPDX-FileCopyrightText: 2024 Fabian Arndt <fabian.arndt@root-core.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <KAuth/ExecuteJob>
#include <QObject>

class QWindow;

namespace PowerDevil
{

class ExternalServiceSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int chargeStartThreshold READ chargeStartThreshold WRITE setChargeStartThreshold NOTIFY chargeStartThresholdChanged)
    Q_PROPERTY(int chargeStopThreshold READ chargeStopThreshold WRITE setChargeStopThreshold NOTIFY chargeStopThresholdChanged)
    Q_PROPERTY(int batteryConservationMode READ batteryConservationMode WRITE setBatteryConservationMode NOTIFY batteryConservationModeChanged)

public:
    explicit ExternalServiceSettings(QObject *parent);

    bool isSaveNeeded() const;

    bool batteryConservationMode() const;
    int chargeStartThreshold() const;
    int chargeStopThreshold() const;

    bool isBatteryConservationModeSupported() const;
    bool isChargeStartThresholdSupported() const;
    bool isChargeStopThresholdSupported() const;

    bool chargeStopThresholdMightNeedReconnect() const;

public Q_SLOTS:
    void load(QWindow *parentWindowForKAuth = nullptr);
    void save(QWindow *parentWindowForKAuth = nullptr);

    void setBatteryConservationMode(bool);
    void setChargeStartThreshold(int);
    void setChargeStopThreshold(int);

Q_SIGNALS:
    void settingsChanged();

    // settings, which in addition to their own signal also trigger settingsChanged()
    void batteryConservationModeChanged();
    void chargeStartThresholdChanged();
    void chargeStopThresholdChanged();

    // not settings per se, so these don't trigger settingsChanged()
    void isBatteryConservationModeSupportedChanged();
    void isChargeStartThresholdSupportedChanged();
    void isChargeStopThresholdSupportedChanged();
    void chargeStopThresholdMightNeedReconnectChanged();

private:
    void setBatteryConservationModeSupported(bool);
    void setSavedBatteryConservationMode(bool);

    void setSavedChargeStartThreshold(int);
    void setSavedChargeStopThreshold(int);
    void setChargeStopThresholdMightNeedReconnect(bool);

    void executeChargeThresholdHelperAction(const QString &actionName,
                                            QWindow *parentWindowForKAuth,
                                            const QVariantMap &arguments,
                                            const std::function<void(KAuth::ExecuteJob *job)> callback);

    int m_chargeStartThreshold;
    int m_chargeStopThreshold;

    int m_savedChargeStartThreshold;
    int m_savedChargeStopThreshold;

    bool m_chargeStopThresholdMightNeedReconnect;

    bool m_isBatteryConservationModeSupported;
    bool m_batteryConservationMode;
    bool m_savedBatteryConservationMode;
};

} // namespace PowerDevil
