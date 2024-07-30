/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QCoreApplication>
#include <QDebug>

#include <QDBusConnection>
#include <QDBusMetaType>

#include <QCommandLineParser>

#include <iostream>

#include <formatter/Formatter.h>
#include <systemstats/DBusInterface.h>

class SensorWatcher : public QCoreApplication
{
    Q_OBJECT

public:
    SensorWatcher(int &argc, char **argv);
    ~SensorWatcher() = default;

    void subscribe(const QStringList &sensorNames);
    void list();
    void showDetails(const QStringList &sensorNames);

    void setShowDetails(bool details);
    void setRemain(bool remain);

private:
    void onNewSensorData(const KSysGuard::SensorDataList &changes);
    void onSensorMetaDataChanged(const KSysGuard::SensorInfoMap &sensors);
    void showSensorDetails(const KSysGuard::SensorInfoMap &sensors);
    KSysGuard::SystemStats::DBusInterface *m_iface;
    /// Contains the names of all sensors that have **not** reported any data yet
    QSet<QString> m_unreportedSensors;
    bool m_showDetails = false;
    /// When @c true, remain connected; otherwise, exit as soon as all subscribed sensors have reported data
    bool m_remain = false;
};

int main(int argc, char **argv)
{
    qDBusRegisterMetaType<KSysGuard::SensorData>();
    qDBusRegisterMetaType<KSysGuard::SensorInfo>();
    qDBusRegisterMetaType<KSysGuard::SensorDataList>();
    qDBusRegisterMetaType<QHash<QString, KSysGuard::SensorInfo>>();
    qDBusRegisterMetaType<QStringList>();

    SensorWatcher app(argc, argv);

    QCommandLineParser parser;
    parser.addOption({ QStringLiteral("list"), QStringLiteral("List Available Sensors") });
    parser.addOption({ QStringLiteral("details"), QStringLiteral("Show detailed information about selected sensors") });
    parser.addOption({ QStringLiteral("remain"), QStringLiteral("Do not exit, but keep reporting sensor values") });

    parser.addPositionalArgument(QStringLiteral("sensorNames"), QStringLiteral("List of sensors to monitor"), QStringLiteral("sensorId1 sensorId2  ..."));
    parser.addHelpOption();
    parser.process(app);

    if (parser.isSet(QStringLiteral("list"))) {
        app.list();
    } else if (parser.positionalArguments().isEmpty()) {
        qDebug() << "No sensors specified.";
        parser.showHelp(-1);
    } else {
        app.setShowDetails(parser.isSet(QStringLiteral("details")));
        app.setRemain(parser.isSet(QStringLiteral("remain")));
        app.subscribe(parser.positionalArguments());
        app.exec();
    }
}

SensorWatcher::SensorWatcher(int &argc, char **argv)
    : QCoreApplication(argc, argv)
    , m_iface(new KSysGuard::SystemStats::DBusInterface(KSysGuard::SystemStats::ServiceName,
          KSysGuard::SystemStats::ObjectPath,
          QDBusConnection::sessionBus(),
          this))
{
    connect(m_iface, &KSysGuard::SystemStats::DBusInterface::newSensorData, this, &SensorWatcher::onNewSensorData);
    connect(m_iface, &KSysGuard::SystemStats::DBusInterface::sensorMetaDataChanged, this, &SensorWatcher::onSensorMetaDataChanged);
}

void SensorWatcher::subscribe(const QStringList &sensorNames)
{
    m_iface->subscribe(sensorNames);

    m_unreportedSensors = QSet<QString>(sensorNames.begin(), sensorNames.end());
    auto pendingInitialValues = m_iface->sensorData(sensorNames);
    pendingInitialValues.waitForFinished();
    onNewSensorData(pendingInitialValues.value());

    if (!m_remain && !m_unreportedSensors.isEmpty()) {
        // We want to do one pass only, and there are sensors that
        // have no initial data. That **might** be because they
        // are mis-spelled, so go through all the sensors and
        // check that the names are ok; sensors that do not exist
        // will be removed (and get a complaining message, too).
        QSet<QString> existingSensors;
        // Fill the set of existing sensor names
        {
            auto pendingSensors = m_iface->allSensors();
            pendingSensors.waitForFinished();
            const auto allSensors = pendingSensors.value();
            for (auto it = allSensors.constBegin(); it != allSensors.constEnd(); it++) {
                existingSensors.insert(it.key());
            }
        }

        QSet<QString> nonexistentSensors;
        for (const auto& sensorName : std::as_const(m_unreportedSensors)) {
            if (!existingSensors.contains(sensorName)) {
                // This sensor (named on the command-line, and with no data so far) does not exist
                std::cout << "No sensor named '" << qPrintable(sensorName) << "'\n";
                nonexistentSensors.insert(sensorName);
            }
        }
        m_unreportedSensors.subtract(nonexistentSensors);
    }

    if (m_showDetails) {
        auto pendingSensors = m_iface->sensors(sensorNames);
        pendingSensors.waitForFinished();

        auto sensors = pendingSensors.value();
        showSensorDetails(sensors);
    }

    if (!m_remain && m_unreportedSensors.isEmpty())
    {
        QTimer::singleShot(0, this, &QCoreApplication::quit);
    }
}

void SensorWatcher::onNewSensorData(const KSysGuard::SensorDataList &changes)
{
    for (const auto &entry : changes) {
        std::cout << qPrintable(entry.sensorProperty) << ' ' << qPrintable(entry.payload.toString()) << std::endl;
        m_unreportedSensors.remove(entry.sensorProperty);
    }
    // Use a timer so that we get out of this slot first; since this is **also** called
    // from subscribe, we want to let that function complete too, just in case the user
    // has asked for details.
    if (!m_remain && m_unreportedSensors.isEmpty())
    {
        QTimer::singleShot(0, this, &QCoreApplication::quit);
    }
}

void SensorWatcher::onSensorMetaDataChanged(const KSysGuard::SensorInfoMap &sensors)
{
    if (!m_showDetails) {
        return;
    }

    std::cout << "Sensor metadata changed\n";
    showSensorDetails(sensors);
}

void SensorWatcher::list()
{
    auto pendingSensors = m_iface->allSensors();
    pendingSensors.waitForFinished();
    auto sensors = pendingSensors.value();
    if (sensors.count() < 1) {
        std::cout << "No sensors available.";
    }
    for (auto it = sensors.constBegin(); it != sensors.constEnd(); it++) {
        std::cout << qPrintable(it.key()) << ' ' << qPrintable(it.value().name) << std::endl;
    }
}

void SensorWatcher::setShowDetails(bool details)
{
    m_showDetails = details;
}

void SensorWatcher::setRemain(bool remain)
{
    m_remain = remain;
}

void SensorWatcher::showSensorDetails(const KSysGuard::SensorInfoMap &sensors)
{
    for (auto it = sensors.constBegin(); it != sensors.constEnd(); ++it) {
        auto info = it.value();
        std::cout << qPrintable(it.key()) << "\n";
        std::cout << "    Name:        " << qPrintable(info.name) << "\n";
        std::cout << "    Short Name:  " << qPrintable(info.shortName) << "\n";
        std::cout << "    Description: " << qPrintable(info.description) << "\n";
        std::cout << "    Unit:        " << qPrintable(KSysGuard::Formatter::symbol(info.unit)) << "\n";
        std::cout << "    Minimum:     " << info.min << "\n";
        std::cout << "    Maximum:     " << info.max << "\n";
    }
}

#include "main.moc"
