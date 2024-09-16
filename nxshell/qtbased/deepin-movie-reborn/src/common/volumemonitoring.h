// Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class VolumeMonitoringPrivate;
class VolumeMonitoring : public QObject
{
    Q_OBJECT
public:
    explicit VolumeMonitoring(QObject *parent = Q_NULLPTR);
    ~VolumeMonitoring();

    void start();
    void stop();

signals:
    void volumeChanged(int volume);
    void muteChanged(bool mute);

public slots:
    void timeoutSlot();

private:
    bool _bOpened;
    QScopedPointer<VolumeMonitoringPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), VolumeMonitoring)
};
