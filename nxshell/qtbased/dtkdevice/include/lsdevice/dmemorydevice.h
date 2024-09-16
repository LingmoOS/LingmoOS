// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DMEMORYDEVICE_H
#define DMEMORYDEVICE_H

#include "dtkdevice_global.h"

#include <QObject>

DDEVICE_BEGIN_NAMESPACE

class DMemoryDevicePrivate;
class LIBDTKDEVICESHARED_EXPORT DMemoryDevice : public QObject
{
public:
    explicit DMemoryDevice(QObject *parent = nullptr);
    ~DMemoryDevice();

    int count();
    QString vendor(int index);
    QString model(int index);
    QString totalWidth(int index);
    QString dataWidth(int index);
    QString type(int index);
    QString speed(int index);
    QString serialNumber(int index);
    QString size(int index);
    QString swapSize();

    QString available();
    QString buffers();
    QString cached();
    QString active();
    QString inactive();
    QString sharedSize();
    QString swapFree();
    QString swapCached();
    QString slab();
    QString dirty();
    QString mapped();

private:
    QScopedPointer<DMemoryDevicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DMemoryDevice)
};

DDEVICE_END_NAMESPACE

#endif // DMEMORYDEVICE_H
