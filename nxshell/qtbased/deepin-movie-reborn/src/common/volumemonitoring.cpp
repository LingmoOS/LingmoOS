// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "volumemonitoring.h"

#include <QTimer>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>

#include "dbus_adpator.h"
#include "dmr_settings.h"
#include "dbusutils.h"

class VolumeMonitoringPrivate
{
public:
    explicit VolumeMonitoringPrivate(VolumeMonitoring *parent) : q_ptr(parent) {}

    QTimer            timer;

    VolumeMonitoring *q_ptr;
    Q_DECLARE_PUBLIC(VolumeMonitoring)
};

VolumeMonitoring::VolumeMonitoring(QObject *parent)
    : QObject(parent), d_ptr(new VolumeMonitoringPrivate(this))
{
    Q_D(VolumeMonitoring);
    _bOpened = false;
    connect(&d->timer, SIGNAL(timeout()), this, SLOT(timeoutSlot()));
}

VolumeMonitoring::~VolumeMonitoring()
{
    stop();
}

void VolumeMonitoring::start()
{
    Q_D(VolumeMonitoring);
    d->timer.start(1000);
}

void VolumeMonitoring::stop()
{
    Q_D(VolumeMonitoring);
    d->timer.stop();
}

void VolumeMonitoring::timeoutSlot()
{
    QVariant v = DBusUtils::redDBusProperty("org.deepin.daemon.Audio1", "/org/deepin/daemon/Audio1",
                                            "org.deepin.daemon.Audio1", "SinkInputs");

    if (!v.isValid())
        return;

    QList<QDBusObjectPath> allSinkInputsList = v.value<QList<QDBusObjectPath> >();

    QString sinkInputPath;
    for (auto curPath : allSinkInputsList) {
        QVariant nameV = DBusUtils::redDBusProperty("org.deepin.daemon.Audio1", curPath.path(),
                                                    "org.deepin.daemon.Audio1.SinkInput", "Name");

        QString movieStr = QObject::tr("Movie");
        if (!nameV.isValid() || (!nameV.toString().contains( movieStr, Qt::CaseInsensitive) && !nameV.toString().contains("deepin-movie", Qt::CaseInsensitive)))
            continue;

        sinkInputPath = curPath.path();
        break;
    }
    if (sinkInputPath.isEmpty())
        return;

    QDBusInterface ainterface("org.deepin.daemon.Audio1", sinkInputPath,
                              "org.deepin.daemon.Audio1.SinkInput",
                              QDBusConnection::sessionBus());
    if (!ainterface.isValid()) {
        return ;
    }

    //获取音量
    QVariant volumeV = DBusUtils::redDBusProperty("org.deepin.daemon.Audio1", sinkInputPath,
                                                  "org.deepin.daemon.Audio1.SinkInput", "Volume");

    //获取音量
    QVariant muteV = DBusUtils::redDBusProperty("org.deepin.daemon.Audio1", sinkInputPath,
                                                "org.deepin.daemon.Audio1.SinkInput", "Mute");

    // int temp = volumeV.toDouble();
    int volume = static_cast<int>(volumeV.toDouble() * 100);
//   int volume = (volumeV.toDouble() +  0.001) * 100;

    auto oldMute = Settings::get().internalOption("mute");
    auto oldVolume = Settings::get().internalOption("global_volume");

    //第一次从dbus里获取的音量可能和实际不匹配，若是第一进入就用实际音量 by zhuyuliang
    if (!_bOpened) {
        Q_EMIT volumeChanged(oldVolume.toInt());
        Q_EMIT muteChanged(oldMute.toBool());
        _bOpened = true;
    } else {
        if (volume != oldVolume)
            Q_EMIT volumeChanged(volume);
        Q_EMIT muteChanged(muteV.toBool());
    }
}
