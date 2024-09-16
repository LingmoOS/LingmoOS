// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dpowerdevice.h"
#include "dpowermanager.h"

#include <qdebug.h>

#include <QCoreApplication>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Dtk::Power::DPowerManager manager;
    auto device = manager.displayDevice();
    qDebug() << device->energyFullDesign();

    QObject::connect(device.data(),
                     &Dtk::Power::DPowerDevice::updateTimeChanged,
                     &app,
                     [=](const QDateTime &value) {
                         qDebug() << "updateTime:" << value;
                     });
    QObject::connect(device.data(),
                     &Dtk::Power::DPowerDevice::timeToEmptyChanged,
                     &app,
                     [=](const qint64 value) {
                         qDebug() << "Time:" << value;
                     });
    QObject::connect(device.data(),
                     &Dtk::Power::DPowerDevice::percentageChanged,
                     &app,
                     [=](const double value) {
                         qDebug() << "percentage:" << value;
                     });
    QObject::connect(&manager,
                     &Dtk::Power::DPowerManager::lidIsClosedChanged,
                     &app,
                     [=](const bool value) {
                         qDebug() << "Lid:" << value;
                     });
    QObject::connect(&manager,
                     &Dtk::Power::DPowerManager::deviceAdded,
                     &app,
                     [=](const QString &name) {
                         qDebug() << "name:" << name;
                     });
    QObject::connect(&manager,
                     &Dtk::Power::DPowerManager::deviceRemoved,
                     &app,
                     [=](const QString &name) {
                         qDebug() << "nameRM:" << name;
                     });

    return app.exec();
}
