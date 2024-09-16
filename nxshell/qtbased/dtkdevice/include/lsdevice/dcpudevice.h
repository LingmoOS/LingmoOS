// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DCPUDEVICE_H
#define DCPUDEVICE_H

#include "dtkdevice_global.h"

#include <QObject>

DDEVICE_BEGIN_NAMESPACE

class DCpuDevicePrivate;
class LIBDTKDEVICESHARED_EXPORT DCpuDevice : public QObject
{
public:
    struct DCpuStat {
        quint64 user {0}; // user time
        quint64 nice {0}; // user time with low priority
        quint64 sys {0}; // system time
        quint64 idle {0}; // idle time
        quint64 iowait {0}; // io wait time
        quint64 hardirq {0}; // interrupt time
        quint64 softirq {0}; // soft interrupt time
        quint64 steal {0}; // stolen time
        quint64 guest {0}; // guest time
        quint64 guestNice {0}; // guest time (niced)
    };

    struct DCpuUsage {
        quint64 total {0};
        quint64 idle {0};
    };

    explicit DCpuDevice(QObject *parent = nullptr);
    ~DCpuDevice();

    int physicalCount();
    int coreCount(int physicalID);
    int threadCount(int physicalID, int coreID);
    int physicalID(int processorID);
    int coreID(int processorID);
    QString architecture(int physicalID);
    QString vendor(int physicalID);
    QString model(int physicalID);
    QString minFreq(int processorID);
    QString maxFreq(int processorID);
    QString cache(int processorID, QString type);
    QString flags(int processorID);
    QString stepping(int processorID);
    QString family(int processorID);
    QString bogoMIPS(int processorID);

    QString temperature(int physicalID);
    QString currentFreq(int processorID);
    DCpuStat stat();
    DCpuUsage usage();
    DCpuStat stat(int processorID);
    DCpuUsage usage(int processorID);

private:
    QScopedPointer<DCpuDevicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DCpuDevice)
};

DDEVICE_END_NAMESPACE

#endif // DCPUDEVICE_H
