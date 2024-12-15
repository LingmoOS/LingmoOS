// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dpowerdevice.h"

#include "dbus/upowerdeviceinterface.h"
#include "dpowerdevice_p.h"
#include "dpowertypes.h"

#include <qdbuspendingreply.h>
#include <qdebug.h>

DPOWER_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DError;
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DUnexpected;

DPowerDevice::DPowerDevice(const QString &name, QObject *parent)
    : QObject(parent)
    , d_ptr(new DPowerDevicePrivate(this))
{
    Q_D(DPowerDevice);
    d->devicename = name;
    d->m_device_inter = new UPowerDeviceInterface(name, this);

    connect(d->m_device_inter,
            &UPowerDeviceInterface::UpdateTimeChanged,
            this,
            [this](const quint64 value) {
                emit this->updateTimeChanged(QDateTime::fromSecsSinceEpoch(value));
            });
    connect(d->m_device_inter,
            &UPowerDeviceInterface::PercentageChanged,
            this,
            [this](const double value) {
                emit this->percentageChanged(value);
            });
    connect(d->m_device_inter,
            &UPowerDeviceInterface::TimeToEmptyChanged,
            this,
            [this](const qint64 value) {
                emit this->timeToEmptyChanged(value);
            });
    connect(d->m_device_inter,
            &UPowerDeviceInterface::TimeToEmptyChanged,
            this,
            [this](const qint64 value) {
                emit this->timeToEmptyChanged(value);
            });
    connect(d->m_device_inter,
            &UPowerDeviceInterface::TimeToEmptyChanged,
            this,
            [this](const qint64 value) {
                emit this->timeToEmptyChanged(value);
            });
    connect(d->m_device_inter,
            &UPowerDeviceInterface::TimeToFullChanged,
            this,
            [this](const qint64 value) {
                emit this->timeToFullChanged(value);
            });
    connect(d->m_device_inter,
            &UPowerDeviceInterface::EnergyRateChanged,
            this,
            [this](const double value) {
                emit this->energyRateChanged(value);
            });
    connect(d->m_device_inter,
            &UPowerDeviceInterface::EnergyChanged,
            this,
            [this](const double value) {
                emit this->energyChanged(value);
            });
    connect(d->m_device_inter,
            &UPowerDeviceInterface::IconNameChanged,
            this,
            [this](const QString &value) {
                emit this->iconNameChanged(value);
            });
}

DPowerDevice::~DPowerDevice() { }

// properties

bool DPowerDevice::hasHistory() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->hasHistory();
}

bool DPowerDevice::hasStatistics() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->hasStatistics();
}

bool DPowerDevice::isPresent() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->isPresent();
}

bool DPowerDevice::isRechargeable() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->isRechargeable();
}

bool DPowerDevice::online() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->online();
}

bool DPowerDevice::powerSupply() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->powerSupply();
}

double DPowerDevice::capacity() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->capacity();
}

double DPowerDevice::energy() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->energy();
}

double DPowerDevice::energyEmpty() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->energyEmpty();
}

double DPowerDevice::energyFull() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->energyFull();
}

double DPowerDevice::energyFullDesign() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->energyFullDesign();
}

double DPowerDevice::energyRate() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->energyRate();
}

double DPowerDevice::luminosity() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->luminosity();
}

double DPowerDevice::percentage() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->percentage();
}

double DPowerDevice::temperature() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->temperature();
}

double DPowerDevice::voltage() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->voltage();
}

qint32 DPowerDevice::chargeCycles() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->chargeCycles();
}

qint64 DPowerDevice::timeToEmpty() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->timeToEmpty();
}

qint64 DPowerDevice::timeToFull() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->timeToFull();
}

QString DPowerDevice::iconName() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->iconName();
}

QString DPowerDevice::model() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->model();
}

QString DPowerDevice::nativePath() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->nativePath();
}

QString DPowerDevice::serial() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->serial();
}

QString DPowerDevice::vendor() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->vendor();
}

quint32 DPowerDevice::batteryLevel() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->batteryLevel();
}

quint32 DPowerDevice::state() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->state();
}

quint32 DPowerDevice::technology() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->technology();
}

quint32 DPowerDevice::type() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->type();
}

quint32 DPowerDevice::warningLevel() const
{
    Q_D(const DPowerDevice);
    return d->m_device_inter->warningLevel();
}

QDateTime DPowerDevice::updateTime() const
{
    Q_D(const DPowerDevice);
    return QDateTime::fromSecsSinceEpoch(d->m_device_inter->updateTime());
}

QString DPowerDevice::deviceName() const
{
    Q_D(const DPowerDevice);
    return d->devicename;
}

// pubilc slots
DExpected<QList<History>> DPowerDevice::history(const QString &type,
                                                const uint timespan,
                                                const uint resolution) const
{
    Q_D(const DPowerDevice);
    QDBusPendingReply<QList<History_p>> reply =
            d->m_device_inter->getHistory(type, timespan, resolution);
    reply.waitForFinished();
    QList<History> historys;
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }

    for (auto &&historyDBus : reply.value()) {
        History history;
        history.value = historyDBus.value;
        history.state = historyDBus.state;
        history.time = historyDBus.time;
        historys.append(history);
    }

    return historys;
}

DExpected<QList<Statistic>> DPowerDevice::statistics(const QString &type) const
{
    Q_D(const DPowerDevice);
    QDBusPendingReply<QList<Statistic_p>> reply = d->m_device_inter->getStatistics(type);
    reply.waitForFinished();
    QList<Statistic> statistics;
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }

    for (auto &&statisticDBus : reply.value()) {
        Statistic statistic;
        statistic.value = statisticDBus.value;
        statistic.accuracy = statisticDBus.accuracy;
        statistics.append(statistic);
    }
    return statistics;
}

DExpected<void> DPowerDevice::refresh()
{
    Q_D(DPowerDevice);
    QDBusPendingReply<> reply = d->m_device_inter->refresh();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DPOWER_END_NAMESPACE
