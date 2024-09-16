// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dpowersettings.h"
#include "dpowertypes.h"

#include <qdebug.h>

#include <QCoreApplication>

#include <unistd.h>

DPOWER_USE_NAMESPACE

class AccountsDemo : public QObject
{
public:
    AccountsDemo()
    {
        connect(&settings, &DPowerSettings::cpuGovernorChanged, this, [](const QString &value) {
            qDebug() << "cpuGovernorChanged:" << value;
        });
        connect(&settings, &DPowerSettings::powerModeChanged, this, [](const PowerMode &mode) {
            qDebug() << "powerModeChanged:" << static_cast<int>(mode);
        });
        connect(&settings,
                &DPowerSettings::autoPowerSavingOnBatteryChanged,
                this,
                [](const bool enabled) {
                    qDebug() << "autoPowerSavingOnBatteryChanged:" << enabled;
                });
        connect(&settings,
                &DPowerSettings::autoPowerSavingWhenBatteryLowChanged,
                this,
                [](const bool enabled) {
                    qDebug() << "autoPowerSavingWhenBatteryLowChanged:" << enabled;
                });
        connect(&settings,
                &DPowerSettings::powerSavingBrightnessDropPercentChanged,
                this,
                [](const quint32 value) {
                    qDebug() << "powerSavingBrightnessDropPercentChanged:" << value;
                });
        connect(&settings,
                &DPowerSettings::batteryLidClosedActionChanged,
                this,
                [](const LidClosedAction &value) {
                    qDebug() << "batteryLidClosedActionChanged:" << static_cast<int>(value);
                });
        connect(&settings, &DPowerSettings::batteryLockDelayChanged, this, [](const qint32 value) {
            qDebug() << "batteryLockDelayChanged:" << value;
        });
        connect(&settings,
                &DPowerSettings::batteryPressPowerBtnActionChanged,
                this,
                [](const PowerBtnAction &value) {
                    qDebug() << "batteryPressPowerBtnActionChanged:" << static_cast<int>(value);
                });
        connect(&settings,
                &DPowerSettings::batteryScreenBlackDelayChanged,
                this,
                [](const qint32 value) {
                    qDebug() << "batteryScreenBlackDelayChanged:" << value;
                });
        connect(&settings,
                &DPowerSettings::batteryScreensaverDelayChanged,
                this,
                [](const qint32 value) {
                    qDebug() << "batteryScreensaverDelayChanged:" << value;
                });
        connect(&settings, &DPowerSettings::batterySleepDelayChanged, this, [](const qint32 value) {
            qDebug() << "batterySleepDelayChanged:" << value;
        });
        connect(&settings,
                &DPowerSettings::linePowerLidClosedActionChanged,
                this,
                [](const LidClosedAction &value) {
                    qDebug() << "linePowerLidClosedActionChanged:" << static_cast<int>(value);
                });
        connect(&settings,
                &DPowerSettings::linePowerLockDelayChanged,
                this,
                [](const qint32 value) {
                    qDebug() << "linePowerLockDelayChanged:" << value;
                });
        connect(&settings,
                &DPowerSettings::linePowerPressPowerBtnActionChanged,
                this,
                [](const PowerBtnAction &value) {
                    qDebug() << "linePowerPressPowerBtnActionChanged:" << static_cast<int>(value);
                });
        connect(&settings,
                &DPowerSettings::linePowerScreenBlackDelayChanged,
                this,
                [](const qint32 value) {
                    qDebug() << "linePowerScreenBlackDelayChanged:" << value;
                });
        connect(&settings,
                &DPowerSettings::linePowerScreensaverDelayChanged,
                this,
                [](const qint32 value) {
                    qDebug() << "linePowerScreensaverDelayChanged:" << value;
                });
        connect(&settings,
                &DPowerSettings::linePowerSleepDelayChanged,
                this,
                [](const qint32 value) {
                    qDebug() << "linePowerSleepDelayChanged:" << value;
                });
        connect(&settings,
                &DPowerSettings::lowPowerAutoSleepThresholdChanged,
                this,
                [](const qint32 value) {
                    qDebug() << "lowPowerAutoSleepThresholdChanged:" << value;
                });
        connect(&settings,
                &DPowerSettings::lowPowerNotifyEnableChanged,
                this,
                [](const bool value) {
                    qDebug() << "lowPowerNotifyEnableChanged:" << value;
                });
        connect(&settings,
                &DPowerSettings::lowPowerNotifyThresholdChanged,
                this,
                [](const qint32 value) {
                    qDebug() << "lowPowerNotifyThresholdChanged:" << value;
                });
        connect(&settings, &DPowerSettings::screenBlackLockChanged, this, [](const bool value) {
            qDebug() << "screenBlackLockChanged:" << value;
        });
        connect(&settings, &DPowerSettings::sleepLockChanged, this, [](const bool value) {
            qDebug() << "sleepLockChanged:" << value;
        });
    }

    void run()
    {
        auto value = settings.linePowerLidClosedAction();
        qDebug() << static_cast<int>(value);
        settings.setLinePowerLidClosedAction(LidClosedAction::DoNothing);
        sleep(1);
        settings.setLinePowerLidClosedAction(value);
    }

private:
    DPowerSettings settings;
};

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);
    AccountsDemo d;
    d.run();
    return a.exec();
}
