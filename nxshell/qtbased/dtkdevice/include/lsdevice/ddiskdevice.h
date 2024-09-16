// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DDISKDEVICE_H
#define DDISKDEVICE_H

#include "dtkdevice_global.h"

#include <QObject>

DDEVICE_BEGIN_NAMESPACE

class DDiskDevicePrivate;
class LIBDTKDEVICESHARED_EXPORT DDiskDevice : public QObject
{
public:
    struct DDiskIoStat {
        quint64 readsCompletedSuccessfully {0};     // # of reads completed successfully
        quint64 readsMerged {0};                    // # of reads merged
        quint64 readSectors {0};                    // # of sectors read
        quint64 spentReadingTime {0};               // # of time spent reading (ms)
        quint64 writesCompleted {0};                // # of writes completed
        quint64 writesMerged {0};                   // # of writes merged
        quint64 writtenSectors {0};                 // # of sectors written
        quint64 spentWritingTime {0};               // # of time spent writing (ms)
        quint64 currentProgressIOs {0};             // # of I/Os currently in progress
        quint64 spentDoingIOsTime {0};              // # of time spent doing I/Os (ms)
        quint64 spentDoingIOsWeightedTime {0};      // # of weighted time spent doing I/Os (ms)
        quint64 discardsCompletedSuccessfully {0};  // # of discards completed successfully
        quint64 discardsMerged {0};                 // # of discards merged
        quint64 discardedSectors {0};               // # of sectors discarded
        quint64 spentDiscardingTime {0};            // # of time spent discarding
    };

    explicit DDiskDevice(QObject *parent = nullptr);
    ~DDiskDevice();

    int count();
    QString vendor(int index);
    QString model(int index);
    QString mediaType(int index);
    QString size(int index);
    QString interface(int index);
    QString serialNumber(int index);
    QString deviceFile(int index);
    QString rotationRate(int index);
    QString firmwareVersion(int index);
    QString sectorSize(int index);

    QString temperature(int index);
    DDiskIoStat diskIoStat(int index);

private:
    QScopedPointer<DDiskDevicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DDiskDevice)
};

DDEVICE_END_NAMESPACE

#endif // DDISKDEVICE_H
