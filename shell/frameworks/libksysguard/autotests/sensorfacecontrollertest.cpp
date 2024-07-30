/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>

#include <QJsonArray>
#include <QJsonDocument>

#include <KConfig>
#include <Solid/Block>
#include <Solid/Device>
#include <Solid/Predicate>
#include <Solid/StorageAccess>
#include <Solid/StorageVolume>

#include "SensorFaceController.h"

#include "SensorFaceController_p.h"

class SensorFaceControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSensorIdConversion_data()
    {
        QTest::addColumn<QJsonArray>("oldSensors");
        QTest::addColumn<QJsonArray>("expectedSensors");

        QTest::addRow("network")
        << QJsonArray {
            QStringLiteral("network/all/sentDataRate"),
            QStringLiteral("network/all/totalReceivedData"),
            QStringLiteral("network/interfaces/test/receiver/data"),
            QStringLiteral("network/interfaces/test/transmitter/dataTotal"),
        }
        << QJsonArray {
            QStringLiteral("network/all/upload"),
            QStringLiteral("network/all/totalDownload"),
            QStringLiteral("network/test/download"),
            QStringLiteral("network/test/totalUpload"),
        };

        const auto storageAccesses = Solid::Device::listFromQuery(Solid::Predicate(Solid::DeviceInterface::StorageAccess, QStringLiteral("accessible"), true));
        for (int i = 0; i < storageAccesses.size(); ++i) {
            const auto storageAccess = storageAccesses[i].as<Solid::StorageAccess>();
            const auto blockDevice = storageAccesses[i].as<Solid::Block>();
            const auto storageVolume = storageAccesses[i].as<Solid::StorageVolume>();
            if (!storageVolume) {
                continue;
            }

            const QString newPrefix = QStringLiteral("disk/") + (storageVolume->uuid().isEmpty() ? storageVolume->label() : storageVolume->uuid());
            // Old code uses "disk/sdc2_(8:34)/..."
            QString device = blockDevice->device().mid(strlen("/dev/"));
            const QString diskPrefix = QStringLiteral("disk/%1_(%2:%3)").arg(device).arg(blockDevice->deviceMajor()).arg(blockDevice->deviceMinor());
            QTest::addRow("disk%d",i)
            << QJsonArray {
                {diskPrefix + QStringLiteral("/Rate/rio")},
                {diskPrefix + QStringLiteral("/Rate/wio")},
            }
            << QJsonArray {
                {newPrefix + QStringLiteral("/read")},
                {newPrefix + QStringLiteral("/write")},
            };
            // Old code uses "partitions/mountPath/..."
            const QString mountPath = storageAccess->filePath() == QLatin1String("/") ? QStringLiteral("/__root__") : storageAccess->filePath();
            QString partitionPrefix = QStringLiteral("partitions") + mountPath;
            QTest::addRow("partition%d", i)
            << QJsonArray {
                {partitionPrefix + QStringLiteral("/total")},
                {partitionPrefix + QStringLiteral("/freespace")},
                {partitionPrefix + QStringLiteral("/filllevel")},
                {partitionPrefix + QStringLiteral("/usedspace")},
            } << QJsonArray {
                {newPrefix + QStringLiteral("/total")},
                {newPrefix + QStringLiteral("/free")},
                {newPrefix + QStringLiteral("/usedPercent")},
                {newPrefix + QStringLiteral("/used")},
            };
        }

        QTest::addRow("paritionsall")
        << QJsonArray {
            QStringLiteral("partitions/all/total"),
            QStringLiteral("partitions/all/freespace"),
            QStringLiteral("partitions/all/filllevel"),
            QStringLiteral("partitions/all/usedspace"),
        } <<  QJsonArray {
            QStringLiteral("disk/all/total"),
            QStringLiteral("disk/all/free"),
            QStringLiteral("disk/all/usedPercent"),
            QStringLiteral("disk/all/used"),
        };

        QTest::addRow("cpu")
        << QJsonArray {
            QStringLiteral("cpu/cpu1/clock"),
            QStringLiteral("cpu/cpu1/sys"),
            QStringLiteral("cpu/cpu1/TotalLoad"),
            QStringLiteral("cpu/system/sys"),
            QStringLiteral("cpu/system/TotalLoad"),
            QStringLiteral("cpu/system/user"),
            QStringLiteral("cpu/system/wait"),
        }
        << QJsonArray {
            QStringLiteral("cpu/cpu1/frequency"),
            QStringLiteral("cpu/cpu1/system"),
            QStringLiteral("cpu/cpu1/usage"),
            QStringLiteral("cpu/all/system"),
            QStringLiteral("cpu/all/usage"),
            QStringLiteral("cpu/all/user"),
            QStringLiteral("cpu/all/wait"),
        };

        QTest::addRow("memory")
        << QJsonArray{
            QStringLiteral("mem/physical/allocated"),
            QStringLiteral("mem/physical/allocatedlevel"),
            QStringLiteral("mem/physical/application"),
            QStringLiteral("mem/physical/applicationlevel"),
            QStringLiteral("mem/physical/available"),
            QStringLiteral("mem/physical/availablelevel"),
            QStringLiteral("mem/physical/buf"),
            QStringLiteral("mem/physical/buflevel"),
            QStringLiteral("mem/physical/cached"),
            QStringLiteral("mem/physical/cachedlevel"),
            QStringLiteral("mem/physical/free"),
            QStringLiteral("mem/physical/freelevel"),
            QStringLiteral("mem/physical/total"),
            QStringLiteral("mem/physical/used"),
            QStringLiteral("mem/physical/usedlevel"),
            QStringLiteral("mem/swap/free"),
            QStringLiteral("mem/swap/used"),
        } << QJsonArray {
            QStringLiteral("memory/physical/used"),
            QStringLiteral("memory/physical/usedPercent"),
            QStringLiteral("memory/physical/application"),
            QStringLiteral("memory/physical/applicationPercent"),
            QStringLiteral("memory/physical/free"),
            QStringLiteral("memory/physical/freePercent"),
            QStringLiteral("memory/physical/buffer"),
            QStringLiteral("memory/physical/bufferPercent"),
            QStringLiteral("memory/physical/cache"),
            QStringLiteral("memory/physical/cachePercent"),
            QStringLiteral("memory/physical/free"),
            QStringLiteral("memory/physical/freePercent"),
            QStringLiteral("memory/physical/total"),
            QStringLiteral("memory/physical/used"),
            QStringLiteral("memory/physical/usedPercent"),
            QStringLiteral("memory/swap/free"),
            QStringLiteral("memory/swap/used"),
        };

        QTest::addRow("gpu")
        << QJsonArray {
            QStringLiteral("nvidia/gpu1/temperature"),
            QStringLiteral("nvidia/gpu2/memory"),
            QStringLiteral("nvidia/gpu3/sharedMemory"),
            QStringLiteral("nvidia/gpu4/memoryClock"),
            QStringLiteral("nvidia/gpu5/processorClock"),
            QStringLiteral("nvidia/gpu6/encoderUsage"),
            QStringLiteral("nvidia/gpu7/decoderUsage"),
        }
        << QJsonArray {
            QStringLiteral("gpu/gpu1/temperature"),
            QStringLiteral("gpu/gpu2/usedVram"),
            QStringLiteral("gpu/gpu3/usedVram"),
            QStringLiteral("gpu/gpu4/memoryFrequency"),
            QStringLiteral("gpu/gpu5/coreFrequency"),
            QStringLiteral("gpu/gpu6/usage"),
            QStringLiteral("gpu/gpu7/usage"),
        };

        QTest::addRow("uptime")
        << QJsonArray {
            QStringLiteral("uptime"),
            QStringLiteral("system/uptime/uptime"),
        }
        << QJsonArray {
            QStringLiteral("os/system/uptime"),
            QStringLiteral("os/system/uptime"),
        };
    }

    void testSensorIdConversion()
    {
        QFETCH(QJsonArray, oldSensors);
        QFETCH(QJsonArray, expectedSensors);
        KConfig config;
        auto sensorsGroup = config.group("Sensors");
        sensorsGroup.writeEntry("sensors", QJsonDocument{oldSensors}.toJson(QJsonDocument::Compact));

        KSysGuard::SensorFaceControllerPrivate d;

        auto sensors = d.readAndUpdateSensors(sensorsGroup, QStringLiteral("sensors"));

        QCOMPARE(sensors.size(), expectedSensors.size());

        for (int i = 0; i < sensors.size(); ++i) {
            QCOMPARE(sensors.at(i).toString(), expectedSensors.at(i).toString());
        }

        auto newEntry = sensorsGroup.readEntry("sensors");
        QCOMPARE(newEntry.toUtf8(), QJsonDocument{expectedSensors}.toJson(QJsonDocument::Compact));
    }
};

QTEST_MAIN(SensorFaceControllerTest);

#include "sensorfacecontrollertest.moc"
