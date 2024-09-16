// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DNETDEVICE_H
#define DNETDEVICE_H

#include "dtkdevice_global.h"

#include <QObject>

DDEVICE_BEGIN_NAMESPACE

class DNetDevicePrivate;
class LIBDTKDEVICESHARED_EXPORT DNetDevice : public QObject
{
public:
    struct DInetAddr4 {
        int        family {0};
        QByteArray addr;
        QByteArray mask;
        QByteArray bcast;
    };

    struct DInetAddr6 {
        int        family {0};
        QByteArray addr;
        int        prefixlen {0};
        int        scope {0};
    };

    struct DNetifInfo {
        quint64 rxPackets {0};   // total packets received
        quint64 rxBytes {0};     // total bytes received
        quint64 rxErrors {0};    // bad packets received
        quint64 rxDropped {0};   // no space in linux buffers
        quint64 rxFifo {0};      // FIFO overruns
        quint64 rxFrame {0};     // frame alignment error
        quint64 txPackets {0};   // total packets transmitted
        quint64 txBytes {0};     // total bytes transmitted
        quint64 txErrors {0};    // packet transmit problems
        quint64 txDropped {0};   // no space available in linux
        quint64 txFifo {0};      // FIFO overruns
        quint64 txCarrier {0};   // loss of link pulse
    };

    explicit DNetDevice(QObject *parent = nullptr);
    ~DNetDevice();

    int count();
    QString name(int index);
    QString vendor(int index);
    QString model(int index);
    QString type(int index);
    QString macAddress(int index);
    QString driver(int index);
    QString speed(int index);

    QString status(int index);
    DInetAddr4 inetAddr4(int index);
    DInetAddr6 inetAddr6(int index);
    DNetifInfo netifInfo(int index);
    QString portStatus(int index, int port);
    QList<int> applicationPorts(int pid);

private:
    QScopedPointer<DNetDevicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DNetDevice)
};

DDEVICE_END_NAMESPACE

#endif // DNETDEVICE_H
