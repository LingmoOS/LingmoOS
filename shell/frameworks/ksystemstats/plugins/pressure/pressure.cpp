/*
    SPDX-FileCopyrightText: 2023 Adrian Edwards <adrian@adriancedwards.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "pressure.h"


#include "pressure_logging.h"

#include <KPluginFactory>
#include <KLocalizedString>
#include <QFile>

#include <systemstats/SensorContainer.h>
#include <systemstats/SensorObject.h>
#include <systemstats/SensorProperty.h>


/**
 * A Structure holding the data for a single line of output from /proc/pressure
 */
struct PressureDatapoint {
    double avg10;
    double avg60;
    double avg300;
    long unsigned int total;
};


/**
 * A Structure holding the data for a single files worth of output from /proc/pressure
 */
struct PressureData {
    PressureDatapoint some;
    PressureDatapoint full;
};

/**
 * Read a file from /proc and process it into an instance of PressureData.
 * 
 * Filename is specified as the name of the virtual file to read that will be appended to /proc/pressure/, for example "memory", "cpu", or "io"
 *
 * File contents returned looks like this:
 * some avg10=0.00 avg60=0.00 avg300=0.00 total=16304493
 * full avg10=0.00 avg60=0.00 avg300=0.00 total=15066192
 *
 * Returns a PressureData structure containing the parsed data
 */
PressureData parseDataFromFile(const QString fileSelector) {

    qCDebug(KSYSTEMSTATS_PRESSURE) << "Parsing " << "/proc/pressure/" << fileSelector;
        
    QFile pressureInfoFile(QStringLiteral("/proc/pressure/") + fileSelector);
    bool open = pressureInfoFile.open(QIODevice::ReadOnly);

    static const QByteArray avg10DataLabel = QByteArrayLiteral("avg10");
    static const QByteArray avg60DataLabel = QByteArrayLiteral("avg60");
    static const QByteArray avg300DataLabel = QByteArrayLiteral("avg300");
    static const QByteArray totalDataLabel = QByteArrayLiteral("total");

    static const QByteArray someDataLabel = QByteArrayLiteral("some");
    static const QByteArray fullDataLabel = QByteArrayLiteral("full");

    if (!open) {
        // You can choose how to handle the error case, such as throwing an exception
        // or returning a default-initialized Data structure.
        return PressureData{};
    }

    qCDebug(KSYSTEMSTATS_PRESSURE) << "File open successfully";
        
    PressureData pressureData;

    for (QByteArray line = pressureInfoFile.readLine(); !line.isEmpty(); line = pressureInfoFile.readLine()) {

        qCDebug(KSYSTEMSTATS_PRESSURE) << "Reading line:" << line;

        QList<QByteArray> fields = line.simplified().split(' ');

        //This assumes fields is not empty, which is mostly gauranteed because
        //we already checked if line isEmpty() in the loop
        QByteArray type = fields.takeFirst();


        qCDebug(KSYSTEMSTATS_PRESSURE) << "type:" << type;
            
        PressureDatapoint data = {0,0,0,0};

        // For each statistic from the fields parsed, set data based on it
        for (QByteArray statistic : fields) {
            const int delim = statistic.indexOf("=");
            const QString name = statistic.left(delim).trimmed();
            const QString value = statistic.mid(delim + 1).trimmed();
        

            qCDebug(KSYSTEMSTATS_PRESSURE) << "statistic:" << statistic << "name:" << name << "value:" << value << "processed:" << value.toDouble();

            if (name == avg10DataLabel) {
                data.avg10 = value.toDouble();
            } else if (name == avg60DataLabel) {
                data.avg60 = value.toDouble();
            } else if (name == avg300DataLabel) {
                data.avg300 = value.toDouble();
            } else if (name == totalDataLabel) {
                data.total = value.toDouble();
            }
        }

        if (type == someDataLabel) {
            pressureData.some = data;
        } else if (type == fullDataLabel) {
            pressureData.full = data;
        } else {
            pressureData.some = data;
        }
    }

    pressureInfoFile.close();


    qCDebug(KSYSTEMSTATS_PRESSURE) << fileSelector << "some" << pressureData.some.avg10 << pressureData.some.avg60 << pressureData.some.avg300  << pressureData.some.total;
    qCDebug(KSYSTEMSTATS_PRESSURE) << fileSelector << "full" << pressureData.full.avg10 << pressureData.full.avg60 << pressureData.full.avg300  << pressureData.full.total;

    return pressureData;
}


PressurePlugin::~PressurePlugin() = default;

/// Top level class for this plugin that implements SensorPlugin from libksysguard
PressurePlugin::PressurePlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{

    qCDebug(KSYSTEMSTATS_PRESSURE) << "Initializing";
        
    container = new KSysGuard::SensorContainer(QStringLiteral("pressure"), i18nc("@title", "Pressure"), this);

    memoryObject = new KSysGuard::SensorObject(QStringLiteral("memory"), i18nc("@title", "Memory Pressure"), container);
    memorySome10SecProperty = new KSysGuard::SensorProperty(QStringLiteral("some10Sec"), i18nc("@title", "Some (10 second average)"), memoryObject);
    memorySome10SecProperty->setDescription(i18nc("@info", "Percentage of time that at least one task is stalled waiting for available memory resources, averaged over 10 seconds"));

    memorySome60SecProperty = new KSysGuard::SensorProperty(QStringLiteral("some60Sec"), i18nc("@title", "Some (1 minute average)"), memoryObject);
    memorySome60SecProperty->setDescription(i18nc("@info", "Percentage of time that at least one task is stalled waiting for available memory resources, averaged over 1 minutes"));

    memorySome300SecProperty = new KSysGuard::SensorProperty(QStringLiteral("some300Sec"), i18nc("@title", "Some (5 minute average)"), memoryObject);
    memorySome300SecProperty->setDescription(i18nc("@info", "Percentage of time that at least one task is stalled waiting for available memory resources, averaged over 5 minutes"));

    memorySomeTotalProperty = new KSysGuard::SensorProperty(QStringLiteral("someTotal"), i18nc("@title", "Total Some Stall Time"), memoryObject);
    memorySomeTotalProperty->setDescription(i18nc("@info", "Amount of time that at least one task is stalled waiting for available memory resources, measured in microseconds"));


    memoryFull10SecProperty = new KSysGuard::SensorProperty(QStringLiteral("full10Sec"), i18nc("@title", "Full (10 second average)"), memoryObject);
    memoryFull10SecProperty->setDescription(i18nc("@info", "Percentage of time that at least one task is stalled waiting for available CPU resources, averaged over 10 seconds"));

    memoryFull60SecProperty = new KSysGuard::SensorProperty(QStringLiteral("full60Sec"), i18nc("@title", "Full (1 minute average)"), memoryObject);
    memoryFull60SecProperty->setDescription(i18nc("@info", "Percentage of time that at least one task is stalled waiting for available CPU resources, averaged over 1 minutes"));

    memoryFull300SecProperty = new KSysGuard::SensorProperty(QStringLiteral("full300Sec"), i18nc("@title", "Full (5 minute average)"), memoryObject);
    memoryFull300SecProperty->setDescription(i18nc("@info", "Percentage of time that at least one task is stalled waiting for available CPU resources, averaged over 5 minutes"));

    memoryFullTotalProperty = new KSysGuard::SensorProperty(QStringLiteral("fullTotal"), i18nc("@title", "Total Full Stall Time"), memoryObject);
    memoryFullTotalProperty->setDescription(i18nc("@info", "Amount of time that at least one task is stalled waiting for available CPU resources, measured in microseconds"));


    cpuObject = new KSysGuard::SensorObject(QStringLiteral("cpu"), i18nc("@title", "CPU Pressure"), container);
    
    cpuSome10SecProperty = new KSysGuard::SensorProperty(QStringLiteral("some10Sec"), i18nc("@title", "Some (10 second average)"), cpuObject);
    cpuSome10SecProperty->setDescription(i18nc("@info", "Percentage of time that at least one task is stalled waiting for available IO resources, averaged over 10 seconds"));

    cpuSome60SecProperty = new KSysGuard::SensorProperty(QStringLiteral("some60Sec"), i18nc("@title", "Some (1 minute average)"), cpuObject);
    cpuSome60SecProperty->setDescription(i18nc("@info", "Percentage of time that at least one task is stalled waiting for available IO resources, averaged over 1 minutes"));

    cpuSome300SecProperty = new KSysGuard::SensorProperty(QStringLiteral("some300Sec"), i18nc("@title", "Some (5 minute average)"), cpuObject);
    cpuSome300SecProperty->setDescription(i18nc("@info", "Percentage of time that at least one task is stalled waiting for available IO resources, averaged over 5 minutes"));

    cpuSomeTotalProperty = new KSysGuard::SensorProperty(QStringLiteral("someTotal"), i18nc("@title", "Total Some Stall Time"), cpuObject);
    cpuSomeTotalProperty->setDescription(i18nc("@info", "Amount of time that at least one task is stalled waiting for available IO resources, measured in microseconds"));


    
    cpuFull10SecProperty = new KSysGuard::SensorProperty(QStringLiteral("full10Sec"), i18nc("@title", "Full (10 second average)"), cpuObject);
    cpuFull10SecProperty->setDescription(i18nc("@info", "Percentage of time that all tasks are stalled waiting for available memory resources, averaged over 10 seconds"));

    cpuFull60SecProperty = new KSysGuard::SensorProperty(QStringLiteral("full60Sec"), i18nc("@title", "Full (1 minute average)"), cpuObject);
    cpuFull60SecProperty->setDescription(i18nc("@info", "Percentage of time that all tasks are stalled waiting for available memory resources, averaged over 1 minutes"));

    cpuFull300SecProperty = new KSysGuard::SensorProperty(QStringLiteral("full300Sec"), i18nc("@title", "Full (5 minute average)"), cpuObject);
    cpuFull300SecProperty->setDescription(i18nc("@info", "Percentage of time that all tasks are stalled waiting for available memory resources, averaged over 5 minutes"));

    cpuFullTotalProperty = new KSysGuard::SensorProperty(QStringLiteral("fullTotal"), i18nc("@title", "Total Full Stall Time"), cpuObject);
    cpuFullTotalProperty->setDescription(i18nc("@info", "Amount of time that all tasks are stalled waiting for available memory resources, measured in microseconds"));



    ioObject = new KSysGuard::SensorObject(QStringLiteral("io"), i18nc("@title", "IO Pressure"), container);
    
    ioSome10SecProperty = new KSysGuard::SensorProperty(QStringLiteral("some10Sec"), i18nc("@title", "Some (10 second average)"), ioObject);
    ioSome10SecProperty->setDescription(i18nc("@info", "Percentage of time that all tasks are stalled waiting for available CPU resources, averaged over 10 seconds"));

    ioSome60SecProperty = new KSysGuard::SensorProperty(QStringLiteral("some60Sec"), i18nc("@title", "Some (1 minute average)"), ioObject);
    ioSome60SecProperty->setDescription(i18nc("@info", "Percentage of time that all tasks are stalled waiting for available CPU resources, averaged over 1 minutes"));

    ioSome300SecProperty = new KSysGuard::SensorProperty(QStringLiteral("some300Sec"), i18nc("@title", "Some (5 minute average)"), ioObject);
    ioSome300SecProperty->setDescription(i18nc("@info", "Percentage of time that all tasks are stalled waiting for available CPU resources, averaged over 5 minutes"));

    ioSomeTotalProperty = new KSysGuard::SensorProperty(QStringLiteral("someTotal"), i18nc("@title", "Total Some Stall Time"), ioObject);
    ioSomeTotalProperty->setDescription(i18nc("@info", "Amount of time that all tasks are stalled waiting for available CPU resources, measured in microseconds"));


   
    ioFull10SecProperty = new KSysGuard::SensorProperty(QStringLiteral("full10Sec"), i18nc("@title", "Full (10 second average)"), ioObject);
    ioFull10SecProperty->setDescription(i18nc("@info", "Percentage of time that all tasks are stalled waiting for available IO resources, averaged over 10 seconds"));

    ioFull60SecProperty = new KSysGuard::SensorProperty(QStringLiteral("full60Sec"), i18nc("@title", "Full (1 minute average)"), ioObject);
    ioFull60SecProperty->setDescription(i18nc("@info", "Percentage of time that all tasks are stalled waiting for available IO resources, averaged over 1 minutes"));

    ioFull300SecProperty = new KSysGuard::SensorProperty(QStringLiteral("full300Sec"), i18nc("@title", "Full (5 minute average)"), ioObject);
    ioFull300SecProperty->setDescription(i18nc("@info", "Percentage of time that all tasks are stalled waiting for available IO resources, averaged over 5 minutes"));

    ioFullTotalProperty = new KSysGuard::SensorProperty(QStringLiteral("fullTotal"), i18nc("@title", "Total Full Stall Time"), ioObject);
    ioFullTotalProperty->setDescription(i18nc("@info", "Amount of time that all tasks are stalled waiting for available IO resources, measured in microseconds"));




    qCDebug(KSYSTEMSTATS_PRESSURE) << "Setting up";

    auto setup_datafield = [](KSysGuard::SensorProperty *sensorProperty)
    {
        sensorProperty->setUnit(KSysGuard::UnitPercent);
        sensorProperty->setVariantType(QVariant::Double);
        sensorProperty->setMax(100);
    };

    setup_datafield(memorySome10SecProperty);
    setup_datafield(memorySome60SecProperty);
    setup_datafield(memorySome300SecProperty);
    setup_datafield(memoryFull10SecProperty);
    setup_datafield(memoryFull60SecProperty);
    setup_datafield(memoryFull300SecProperty);
    setup_datafield(cpuSome10SecProperty);
    setup_datafield(cpuSome60SecProperty);
    setup_datafield(cpuSome300SecProperty);
    setup_datafield(cpuFull10SecProperty);
    setup_datafield(cpuFull60SecProperty);
    setup_datafield(cpuFull300SecProperty);
    setup_datafield(ioSome10SecProperty);
    setup_datafield(ioSome60SecProperty);
    setup_datafield(ioSome300SecProperty);
    setup_datafield(ioFull10SecProperty);
    setup_datafield(ioFull60SecProperty);
    setup_datafield(ioFull300SecProperty);

    auto setup_totalfield = [](KSysGuard::SensorProperty *sensorProperty)
    {
        // TODO: specify unit sensorProperty->setUnit(KSysGuard::UnitPercent);
        sensorProperty->setVariantType(QVariant::Double);//long unsigned int
    };

    setup_totalfield(memorySomeTotalProperty);
    setup_totalfield(memoryFullTotalProperty);
    setup_totalfield(cpuSomeTotalProperty);
    setup_totalfield(cpuFullTotalProperty);
    setup_totalfield(ioSomeTotalProperty);
    setup_totalfield(ioFullTotalProperty);

}

void PressurePlugin::update()
{

    qCDebug(KSYSTEMSTATS_PRESSURE) << "Updating";
        
    PressureData data = parseDataFromFile(QStringLiteral("memory"));

    memorySome10SecProperty->setValue(data.some.avg10);
    memorySome60SecProperty->setValue(data.some.avg60);
    memorySome300SecProperty->setValue(data.some.avg300);
    memorySomeTotalProperty->setValue(QVariant::fromValue(data.some.total));
    memoryFull10SecProperty->setValue(data.full.avg10);
    memoryFull60SecProperty->setValue(data.full.avg60);
    memoryFull300SecProperty->setValue(data.full.avg300);
    memoryFullTotalProperty->setValue(QVariant::fromValue(data.full.total));

    data = parseDataFromFile(QStringLiteral("cpu"));
    
    cpuSome10SecProperty->setValue(data.some.avg10);
    cpuSome60SecProperty->setValue(data.some.avg60);
    cpuSome300SecProperty->setValue(data.some.avg300);
    cpuSomeTotalProperty->setValue(QVariant::fromValue(data.some.total));
    cpuFull10SecProperty->setValue(data.full.avg10);
    cpuFull60SecProperty->setValue(data.full.avg60);
    cpuFull300SecProperty->setValue(data.full.avg300);
    cpuFullTotalProperty->setValue(QVariant::fromValue(data.full.total));

    data = parseDataFromFile(QStringLiteral("io"));

    ioSome10SecProperty->setValue(data.some.avg10);
    ioSome60SecProperty->setValue(data.some.avg60);
    ioSome300SecProperty->setValue(data.some.avg300);
    ioSomeTotalProperty->setValue(QVariant::fromValue(data.some.total));
    ioFull10SecProperty->setValue(data.full.avg10);
    ioFull60SecProperty->setValue(data.full.avg60);
    ioFull300SecProperty->setValue(data.full.avg300);
    ioFullTotalProperty->setValue(QVariant::fromValue(data.full.total));
}

K_PLUGIN_CLASS_WITH_JSON(PressurePlugin, "metadata.json")

#include "pressure.moc"
