// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRANSFERWRAPPER_H
#define TRANSFERWRAPPER_H

#include "discover/deviceinfo.h"
#include <CuteIPCService.h>

#include <QObject>

namespace cooperation_core {

class TransferWrapper : public CuteIPCService
{
    Q_OBJECT
public:
    static TransferWrapper *instance();

public slots:
    void onRefreshDevice();
    void onSearchDevice(const QString &ip);
    void onSendFiles(const QString &ip, const QString &name, const QStringList &files);

private slots:
    void onDeviceOnline(const QList<DeviceInfoPointer> &infoList);
    void onDeviceOffline(const QString &ip);
    void onFinishedDiscovery(bool hasFound);

signals:
    void searched(const QString& info);
    void refreshed(const QStringList& infoList);
    void deviceChanged(bool found, const QString& info);

private:
    explicit TransferWrapper(QObject *parent = nullptr);
    ~TransferWrapper();

    QString variantMapToQString(const QVariantMap &variantMap);
};

}   // namespace cooperation_core

#endif // TRANSFERWRAPPER_H
