// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DMOTHERBOARDDEVICE_H
#define DMOTHERBOARDDEVICE_H

#include "dtkdevice_global.h"

#include <QObject>

DDEVICE_BEGIN_NAMESPACE

class DMotherboardDevicePrivate;
class LIBDTKDEVICESHARED_EXPORT DMotherboardDevice : public QObject
{
public:
    explicit DMotherboardDevice(QObject *parent = nullptr);
    ~DMotherboardDevice();

    QString vendor();
    QString model();
    QString date();
    QString serialNumber();
    QString biosInformation();
    QString chassisInformation();
    QString PhysicalMemoryArrayInformation();

    QString temperature();

private:
    QScopedPointer<DMotherboardDevicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DMotherboardDevice)
};

DDEVICE_END_NAMESPACE

#endif // DMOTHERBOARDDEVICE_H
