/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     lingmoos <lingmoos@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BATTERY_H
#define BATTERY_H

#include <QObject>
#include <QDBusInterface>

class Battery : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY validChanged)
    Q_PROPERTY(int chargeState READ chargeState NOTIFY chargeStateChanged)
    Q_PROPERTY(int chargePercent READ chargePercent NOTIFY chargePercentChanged)
    Q_PROPERTY(int lastChargedPercent READ lastChargedPercent NOTIFY lastChargedPercentChanged)
    Q_PROPERTY(int capacity READ capacity NOTIFY capacityChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY remainingTimeChanged)
    Q_PROPERTY(bool onBattery READ onBattery NOTIFY onBatteryChanged)
    Q_PROPERTY(bool showPercentage READ showPercentage NOTIFY showPercentageChanged)
    Q_PROPERTY(QString iconSource READ iconSource NOTIFY iconSourceChanged)

public:
    static Battery *self();
    explicit Battery(QObject *parent = nullptr);

    bool available() const;
    bool onBattery() const;

    bool showPercentage() const;
    void setShowPercentage(bool enabled);

    int chargeState() const;
    int chargePercent() const;
    int lastChargedPercent() const;
    int capacity() const;
    QString statusString() const;

    QString iconSource() const;

signals:
    void validChanged();
    void chargeStateChanged(int);
    void chargePercentChanged(int);
    void capacityChanged(int);
    void remainingTimeChanged(qlonglong time);
    void onBatteryChanged();
    void lastChargedPercentChanged();
    void iconSourceChanged();
    void showPercentageChanged();

private slots:
    void onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps);

private:
    QDBusInterface m_upowerInterface;
    QDBusInterface m_interface;
    bool m_available;
    bool m_onBattery;
    bool m_showPercentage;
};

#endif // BATTERY_H
