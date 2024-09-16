// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DINPUTDEVICE_H
#define DINPUTDEVICE_H

#include "dtkdevice_global.h"

#include <QObject>

DDEVICE_BEGIN_NAMESPACE

class DInputDevicePrivate;
class LIBDTKDEVICESHARED_EXPORT DInputDevice : public QObject
{
public:
    explicit DInputDevice(QObject *parent = nullptr);
    ~DInputDevice();

    int count();
    QString name(int index);
    QString vendor(int index);
    QString model(int index);
    QString interface(int index);
    QString driver(int index);

private:
    QScopedPointer<DInputDevicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DInputDevice)
};

DDEVICE_END_NAMESPACE

#endif // DDISKDEVICE_H
