/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2022 Alessio Bonfiglio <alessio.bonfiglio@mail.polimi.it>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "cpu.h"

#include <functional>
#include <iterator>
#include <numeric>

#include <KLocalizedString>
#include <systemstats/AggregateSensor.h>

BaseCpuObject::BaseCpuObject(const QString &id, const QString &name, KSysGuard::SensorContainer *parent)
    : SensorObject(id, name, parent)
{
}

void BaseCpuObject::makeSensors()
{
    m_usage = new KSysGuard::SensorProperty(QStringLiteral("usage"), QStringLiteral("usage"), 0, this);
    m_system = new KSysGuard::SensorProperty(QStringLiteral("system"), QStringLiteral("system"), 0, this);
    m_user = new KSysGuard::SensorProperty(QStringLiteral("user"), QStringLiteral("user"), 0, this);
    m_wait = new KSysGuard::SensorProperty(QStringLiteral("wait"), QStringLiteral("wait"), 0, this);
    auto n = new KSysGuard::SensorProperty(QStringLiteral("name"), i18nc("@title", "Name"), name(), this);
    n->setVariantType(QVariant::String);
}


void BaseCpuObject::initialize()
{
    makeSensors();

    m_usage->setPrefix(name());
    m_usage->setName(i18nc("@title", "Total Usage"));
    m_usage->setShortName(id() == u"all" ? i18nc("@title, Short for 'Total Usage'", "Usage") : name().toUtf8().constData());
    m_usage->setUnit(KSysGuard::UnitPercent);
    m_usage->setVariantType(QVariant::Double);
    m_usage->setMax(100);

    m_system->setPrefix(name());
    m_system->setName(i18nc("@title", "System Usage"));
    m_system->setShortName(id() == u"all" ? i18nc("@title, Short for 'System Usage'", "System") : i18nc("@title, Short for '%1 System Usage'", "%1 System", name()));
    m_system->setUnit(KSysGuard::UnitPercent);
    m_system->setVariantType(QVariant::Double);
    m_system->setMax(100);

    m_user->setPrefix(name());
    m_user->setName(i18nc("@title", "User Usage"));
    m_user->setShortName(id() == u"all" ? i18nc("@title, Short for 'User Usage'", "User") : i18nc("@title, Short for '%1 User Usage'", "%1 User", name()));
    m_user->setUnit(KSysGuard::UnitPercent);
    m_user->setVariantType(QVariant::Double);
    m_user->setMax(100);

    m_wait->setPrefix(name());
    m_wait->setName(i18nc("@title", "Wait Usage"));
    m_wait->setShortName(id() == u"all" ? i18nc("@title, Short for 'Wait Load'", "Wait") : i18nc("@title, Short for '%1 Wait Load'", "%1 Wait", name()));
    m_wait->setUnit(KSysGuard::UnitPercent);
    m_wait->setVariantType(QVariant::Double);
    m_wait->setMax(100);
}


CpuObject::CpuObject(const QString &id, const QString &name, KSysGuard::SensorContainer *parent)
    : BaseCpuObject(id, name, parent)
{
}

void CpuObject::makeSensors()
{
    BaseCpuObject::makeSensors();

    m_frequency = new KSysGuard::SensorProperty(QStringLiteral("frequency"), QStringLiteral("frequency"), 0, this);
    m_temperature = new KSysGuard::SensorProperty(QStringLiteral("temperature"), QStringLiteral("temperature"), 0, this);
}

void CpuObject::initialize()
{
    BaseCpuObject::initialize();

    m_frequency->setPrefix(name());
    m_frequency->setName(i18nc("@title", "Current Frequency"));
    m_frequency->setShortName(i18nc("@title, Short for 'Current Frequency'", name().toUtf8().constData()));
    m_frequency->setDescription(i18nc("@info", "Current frequency of the CPU"));
    m_frequency->setVariantType(QVariant::Double);
    m_frequency->setUnit(KSysGuard::Unit::UnitMegaHertz);

    m_temperature->setPrefix(name());
    m_temperature->setName(i18nc("@title", "Current Temperature"));
    m_temperature->setShortName(i18nc("@title, Short for Current Temperatur", name().toUtf8().constData()));
    m_temperature->setVariantType(QVariant::Double);
    m_temperature->setUnit(KSysGuard::Unit::UnitCelsius);
}


AllCpusObject::AllCpusObject(KSysGuard::SensorContainer *parent)
    : BaseCpuObject(QStringLiteral("all"), i18nc("@title", "All"), parent)
{
}

void AllCpusObject::makeSensors()
{
    BaseCpuObject::makeSensors();

    m_cpuCount = new KSysGuard::SensorProperty(QStringLiteral("cpuCount"), this);
    m_coreCount = new KSysGuard::SensorProperty(QStringLiteral("coreCount"), this);

    auto maxAggregator = [](QVariant a, QVariant b) {return std::max(a.toDouble(), b.toDouble());};
    auto minAggregator = [](QVariant a, QVariant b) {return std::min(a.toDouble(), b.toDouble());};
    auto avgAggregator = [](KSysGuard::AggregateSensor::SensorIterator begin, const KSysGuard::AggregateSensor::SensorIterator end) {
        int count = std::distance(begin, end);
    double sum = std::transform_reduce(begin, end, 0.0, std::plus{}, [](auto val) { return qvariant_cast<double>(val); });
        return sum / count;
    };

    auto maxFrequency = new KSysGuard::AggregateSensor(this, "maximumFrequency", i18nc("@title", "Maximum CPU Frequency"));
    maxFrequency->setShortName(i18nc("@title, Short for 'Maximum CPU Frequency'", "Max Frequency"));
    maxFrequency->setDescription(i18nc("@info", "Current maximum frequency between all CPUs"));
    maxFrequency->setUnit(KSysGuard::Unit::UnitMegaHertz);
    maxFrequency->setMatchSensors(QRegularExpression("^(?!all).*$"), "frequency");
    maxFrequency->setAggregateFunction(maxAggregator);
    
    auto minFrequency = new KSysGuard::AggregateSensor(this, "minimumFrequency", i18nc("@title", "Minimum CPU Frequency"));
    minFrequency->setShortName(i18nc("@title, Short for 'Minimum CPU Frequency'", "Min Frequency"));
    minFrequency->setDescription(i18nc("@info", "Current minimum frequency between all CPUs"));
    minFrequency->setUnit(KSysGuard::Unit::UnitMegaHertz);
    minFrequency->setMatchSensors(QRegularExpression("^(?!all).*$"), "frequency");
    minFrequency->setAggregateFunction(minAggregator);

    auto avgFrequency = new KSysGuard::AggregateSensor(this, "averageFrequency", i18nc("@title", "Average CPU Frequency"));
    avgFrequency ->setShortName(i18nc("@title, Short for 'Average CPU Frequency'", "Average Frequency"));
    avgFrequency ->setDescription(i18nc("@info", "Current average frequency between all CPUs"));
    avgFrequency ->setUnit(KSysGuard::Unit::UnitMegaHertz);
    avgFrequency ->setMatchSensors(QRegularExpression("^(?!all).*$"), "frequency");
    avgFrequency ->setAggregateFunction(avgAggregator);
    
    auto maxTemp = new KSysGuard::AggregateSensor(this, "maximumTemperature", i18nc("@title", "Maximum CPU Temperature"));
    maxTemp->setShortName(i18nc("@title, Short for 'Maximum CPU Temperature'", "Max Temperature"));
    maxTemp->setVariantType(QVariant::Double);
    maxTemp->setUnit(KSysGuard::Unit::UnitCelsius);
    maxTemp->setMatchSensors(QRegularExpression("^(?!all).*$"), "temperature");
    maxTemp->setAggregateFunction(maxAggregator);
    
    auto minTemp = new KSysGuard::AggregateSensor(this, "minimumTemperature", i18nc("@title", "Minimum CPU Temperature"));
    minTemp->setShortName(i18nc("@title, Short for 'Minimum CPU Temperature'", "Min Temperature"));
    minTemp->setVariantType(QVariant::Double);
    minTemp->setUnit(KSysGuard::Unit::UnitCelsius);
    minTemp->setMatchSensors(QRegularExpression("^(?!all).*$"), "temperature");
    minTemp->setAggregateFunction(minAggregator);

    auto avgTemp = new KSysGuard::AggregateSensor(this, "averageTemperature", i18nc("@title", "Average CPU Temperature"));
    avgTemp->setShortName(i18nc("@title, Short for 'Average CPU Temperature'", "Average Temperature"));
    avgTemp->setVariantType(QVariant::Double);
    avgTemp->setUnit(KSysGuard::Unit::UnitCelsius);
    avgTemp->setMatchSensors(QRegularExpression("^(?!all).*$"), "temperature");
    avgTemp->setAggregateFunction(avgAggregator);
}

void AllCpusObject::initialize()
{
    BaseCpuObject::initialize();

    m_usage->setPrefix(QString());
    m_system->setPrefix(QString());
    m_user->setPrefix(QString());
    m_wait->setPrefix(QString());

    m_cpuCount->setName(i18nc("@title", "Number of CPUs"));
    m_cpuCount->setShortName(i18nc("@title, Short fort 'Number of CPUs'", "CPUs"));
    m_cpuCount->setDescription(i18nc("@info", "Number of physical CPUs installed in the system"));

    m_coreCount->setName(i18nc("@title", "Number of Cores"));
    m_coreCount->setShortName(i18nc("@title, Short fort 'Number of Cores'", "Cores"));
    m_coreCount->setDescription(i18nc("@info", "Number of CPU cores across all physical CPUS"));
}

void AllCpusObject::setCounts(unsigned int cpuCount, unsigned int coreCount)
{
    m_cpuCount->setValue(cpuCount);
    m_coreCount->setValue(coreCount);
}
